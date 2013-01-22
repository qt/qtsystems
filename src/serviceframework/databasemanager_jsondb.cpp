/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QDebug>
#include <QHash>
#include <QCryptographicHash>
#include <QFile>
#include <QTime>
#include <QThreadStorage>
#include <QMutex>
#include <QMutexLocker>

#include <QCoreApplication>
#include <QTimer>
#include <QThread>
#include <QJsonArray>

#include <QtJsonDb/qjsondbconnection.h>
#include <QtJsonDb/qjsondbreadrequest.h>
#include <QtJsonDb/qjsondbwriterequest.h>
#include <QtJsonDb/qjsondbwatcher.h>

#include "databasemanager_jsondb_p.h"
#include "qservicedebuglog_p.h"

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE_JSONDB

#define JSON_EXPIRATION_TIMER 15000

class JsondbThread;

class JsondbWorker : public QObject
{
    Q_OBJECT
public:
    JsondbWorker();

    ~JsondbWorker();
    const QList<QJsonObject> getCache();
    void waitForCache();

Q_SIGNALS:
    void serviceAdded(const QString &service, DatabaseManager::DbScope);
    void serviceRemoved(const QString &service,  DatabaseManager::DbScope);

public Q_SLOTS:
    bool sendRequest(QJsonDbRequest *request);
    void onNotification();

private Q_SLOTS:
    void reapThread();
    void startNotifier(qint32 stateNumber);
    void watcherStatusChanged(QtJsonDb::QJsonDbWatcher::Status status);

private:

    void setCache(const QList<QJsonObject> &newResults);
    void setResults(const QList<QJsonObject> &newResults, QJsonDbRequest::Status status);
    QMutex cache_mutex;
    QMutex thread_control_mutex;
    QMutex request_serialize_mutex;
    QMutex request_mutex;

    void startThread();

    QJsonDbConnection *db;
    QJsonDbWatcher *dbwatcher;

    // init must cache_loading_mutex
    QList<QJsonObject> cache;
    QList<QJsonObject> cache_deleted_items;

    // seralize with request_serialize_mutex
    // block on request_mutex
    QList<QJsonObject> results;
    QJsonDbRequest::Status results_status;
    QJsonDbRequest *request;
    QThread *request_thread;
    QJsonDbRequest::ErrorCode results_error;
    QString results_errormsg;

    QTime ticker;
    QTimer thread_timeout;

    // lock with thread_control_mutex
    QThread *workerThread;
    JsondbThread *jsonthread;

    bool cacheLoaded;

    friend class JsondbThread;

};

class JsondbThread : public QObject
{
    Q_OBJECT
public:
    JsondbThread(JsondbWorker *worker) :
        QObject(),
        db(new QJsonDbConnection(this)),
        worker(worker)

    {
        db->connectToServer();

        qRegisterMetaType<QJsonDbRequest*>("QJsonDbRequest*");

        cache_timeout = new QTimer(this);
        cache_timeout->setSingleShot(true);
        cache_timeout->setInterval(JSON_EXPIRATION_TIMER);
        req_timeout = new QTimer(this);
        req_timeout->setSingleShot(true);
        req_timeout->setInterval(JSON_EXPIRATION_TIMER);

        connect(cache_timeout, SIGNAL(timeout()), this, SLOT(cacheRequestTimeout()));
        connect(req_timeout, SIGNAL(timeout()), this, SLOT(requestTimeout()));
    }

    // called from any thread
    void newCacheRequest(QJsonDbRequest *req)
    {
        /* don't deadlock if run before QCoreApplication */
        if (!QCoreApplication::instance()) {
            qWarning() << "SFW failing to load cache, no QApplication exists";
            return;
        }

        worker->cache_mutex.lock();
        qServiceLog() << "class" << "dbm_jsondb"
                      << "event" << "fetch cache data";
        QMetaObject::invokeMethod(this, "doCacheRequest", Q_ARG(QJsonDbRequest *, req));
    }

    // called from any thread
    void newRequest(QJsonDbRequest *req)
    {
        /* don't deadlock if run before QCoreApplication */
        if (!QCoreApplication::instance()) {
            qWarning() << "SFW failing to query jsondb, no QApplication exists";
            return;
        }

        worker->request_mutex.lock();
        QMetaObject::invokeMethod(this, "doNewRequest", Q_ARG(QJsonDbRequest *, req));
    }

private Q_SLOTS:

    void doCacheRequest(QJsonDbRequest *req)
    {
        /* ALL SFW partion are in setting */
        if (req->partition().isEmpty()) {
            req->setPartition(QStringLiteral("com.nokia.mt.Settings"));
        }
        db->send(req);
        connect(req, SIGNAL(finished()), this, SLOT(requestCacheFinishedSlot()));
        connect(req, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)), this, SLOT(requestCacheFinishedSlot()));
        cache_timeout->start();
    }

    void doNewRequest(QJsonDbRequest *req)
    {
        /* ALL SFW partion are in setting */
        if (req->partition().isEmpty()) {
            req->setPartition(QStringLiteral("com.nokia.mt.Settings"));
        }
        db->send(req);
        connect(req, SIGNAL(finished()), this, SLOT(requestFinishedSlot()));
        connect(req, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
                this, SLOT(requestErrorSlot(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));
        req_timeout->start();
    }

    void requestCacheFinishedSlot()
    {
        QJsonDbReadRequest *request = qobject_cast<QJsonDbReadRequest *>(QObject::sender());
        worker->setCache(request->takeResults());
        QString s;
        foreach (const QJsonObject &e, worker->cache) {
            s.append(e.value(QStringLiteral("service")).toString());
            s.append(QStringLiteral(", "));
        }
        qServiceLog() << "class" << "dbm_jsondb"
                      << "event" << "fetch done, unlock"
                      << "statenum" << (qint32)request->stateNumber()
                      << "services" << s;
        if (cache_timeout->isActive())
            worker->cache_mutex.unlock();
        cache_timeout->stop();
        emit cacheLoaded(request->stateNumber());
        request->deleteLater();
    }
    void cacheRequestTimeout()
    {
        qWarning() << "SFW timeout fetching services from jsondb";
        QList<QJsonObject> empty;
        worker->setCache(empty);
        worker->cache_mutex.unlock();
    }

    void requestFinishedSlot()
    {
        disconnect(QObject::sender(), SIGNAL(finished()), this, SLOT(requestFinishedSlot()));
        disconnect(QObject::sender(), SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
                this, SLOT(requestErrorSlot(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));

        if (req_timeout->isActive())
            worker->request_mutex.unlock();
        req_timeout->stop();
    }
    void requestErrorSlot(QtJsonDb::QJsonDbRequest::ErrorCode err, QString msg)
    {
        worker->results_error = err;
        worker->results_errormsg = msg;
        if (req_timeout->isActive())
            worker->request_mutex.unlock();
        req_timeout->stop();
    }
    void requestTimeout()
    {
        qWarning() << "SFW timeout fetching jsondb request";
        worker->results_error = QJsonDbRequest::DatabaseError;
        worker->results_errormsg = QStringLiteral("Database timeout");
        worker->request_mutex.unlock();
    }

Q_SIGNALS:
    void cacheLoaded(qint32 stateNumber);

private:
    QJsonDbConnection *db;
    JsondbWorker *worker;

    QTimer *cache_timeout;
    QTimer *req_timeout;
};

JsondbWorker::JsondbWorker() :
    QObject(),
    dbwatcher(0),
    workerThread(0),
    jsonthread(0),
    cacheLoaded(false)

{
    db = 0x0; // don't connect to the db until we're in the right thread

    thread_timeout.setInterval(JSON_EXPIRATION_TIMER*2);
    thread_timeout.setSingleShot(true);
    connect(&thread_timeout, SIGNAL(timeout()), this, SLOT(reapThread()));

    startThread();

    connect(jsonthread, SIGNAL(cacheLoaded(qint32)), this, SLOT(startNotifier(qint32)));

    QJsonDbReadRequest *request = new QJsonDbReadRequest();
    request->setQuery(QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"]"));

    request->moveToThread(workerThread);

    jsonthread->newCacheRequest(request);

    workerThread->start();

    if (!QCoreApplication::instance()) {
        qWarning() << "SFW failing to start jsondb backend no QCoreApplication exists, expect massive failure";
        return;
    }
    this->moveToThread(QCoreApplication::instance()->thread());
}

JsondbWorker::~JsondbWorker()
{
    QMutexLocker m(&thread_control_mutex);
    if (workerThread) {
        workerThread->quit();
        workerThread->wait();
    }
}

void JsondbWorker::startNotifier(qint32 stateNumber)
{
    db = QJsonDbConnection::defaultConnection();
    qServiceLog() << "class" << "dbm_jsondb"
                  << "event" << "notifier start"
                  << "state" << stateNumber;
    dbwatcher = new QJsonDbWatcher(this);
    connect(dbwatcher, SIGNAL(notificationsAvailable(int)), this, SLOT(onNotification()));
    dbwatcher->setWatchedActions(QJsonDbWatcher::Created | QJsonDbWatcher::Updated | QJsonDbWatcher::Removed);
    dbwatcher->setQuery(QLatin1String("[?_type=\"com.nokia.mt.serviceframework.interface\"]"));
    dbwatcher->setPartition(QStringLiteral("com.nokia.mt.Settings"));
    dbwatcher->setInitialStateNumber(stateNumber);
    db->addWatcher(dbwatcher);
    connect(dbwatcher, SIGNAL(statusChanged(QtJsonDb::QJsonDbWatcher::Status)), this, SLOT(watcherStatusChanged(QtJsonDb::QJsonDbWatcher::Status)));
}

void JsondbWorker::watcherStatusChanged(QtJsonDb::QJsonDbWatcher::Status status)
{
    QString statusString = QStringLiteral("unknown");

    switch (status) {
    case QtJsonDb::QJsonDbWatcher::Inactive:
        statusString = QStringLiteral("Inactive");
        break;
    case QtJsonDb::QJsonDbWatcher::Activating:
        statusString = QStringLiteral("Activating");
        break;
    case QtJsonDb::QJsonDbWatcher::Active:
        statusString = QStringLiteral("Active");
        break;
    }

    qServiceLog() << "class" << "dbm_jsondb"
                  << "event" << "watcher status change"
                  << "status" << statusString;
}

/*
 * \internal
 *
 * Must grab the thread lock before calling this function
 */
void JsondbWorker::startThread()
{
    workerThread = new QThread(this);
    jsonthread = new JsondbThread(this);
    connect(workerThread, SIGNAL(destroyed()), jsonthread, SLOT(deleteLater()));

    jsonthread->moveToThread(workerThread);
    workerThread->start();

}


const QList<QJsonObject> JsondbWorker::getCache()
{
    QMutexLocker locker(&thread_control_mutex);
    waitForCache();

    return cache;
}

void JsondbWorker::waitForCache()
{
    if (cache_mutex.tryLock(JSON_EXPIRATION_TIMER)) {
        cache_mutex.unlock();
    } else {
        qWarning() << "SFW timeout waiting for jsondb caching thread";
        qServiceLog() << "class" << "dbm_jsondb"
                      << "event" << "failed taking cachelock";
    }
}

static bool serviceGreaterThan(const QJsonObject &s1, const QJsonObject &s2)
{
    // no order is defined in sfw, but a unit tests depends on this
    // order, so we greater than
    return s1.value(QStringLiteral("service")).toString() < s2.value(QStringLiteral("service")).toString();
}

void JsondbWorker::setCache(const QList<QJsonObject> &newResults)
{
    cache = newResults;
    qSort(cache.begin(), cache.end(), serviceGreaterThan);
    qServiceLog() << "class" << "jsondbworker"
                  << "event" << "cacheload"
                  << "time" << ticker.elapsed()
                  << "count" << cache.count();
    // kill the thread in 15s is no else uses sfw
    thread_timeout.start();
}

bool JsondbWorker::sendRequest(QJsonDbRequest *request)
{
    QMutexLocker t(&thread_control_mutex);
    if (!workerThread) {
        startThread();
    }
    // reset thread death
    thread_timeout.start();
    t.unlock();

    QMutexLocker s(&request_serialize_mutex);
    results_error = QJsonDbRequest::NoError;
    results_errormsg.clear();

    this->request = request;
    this->request_thread = request->thread();
    jsonthread->newRequest(request);

    QMutexLocker r(&request_mutex);

//    qDebug() << "SFW request status" << request->status() << results_error << results_errormsg;
    /*
     * setResults called from worker thread while
     * blocked on the mutex
     */

    if (request->status() != QJsonDbRequest::Error) {
        QMutexLocker c(&cache_mutex);
        qServiceLog() << "class" << "dbm_jsondb"
                      << "event" << "lock cache"
                      << "from" << "sendRequest";
        QJsonDbCreateRequest *create = qobject_cast<QJsonDbCreateRequest *>(request);
        if (create) {
            foreach (const QJsonObject &o, create->objects()) {
                bool contains = false;
                qServiceLog() << "class" << "dbm_jsondb"
                              << "event" << "create req"
                              << "service" << o.value(QStringLiteral("service")).toString();

                foreach (const QJsonObject &e, cache) {
                    if (e.value(QStringLiteral("service")).toString() == o.value(QStringLiteral("service")).toString()) {
                        contains = true;
                        break;
                    }
                }
                if (!contains) {
                    qServiceLog() << "class" << "dbm_jsondb"
                                  << "event" << "add notify"
                                  << "service" << o.value(QStringLiteral("service")).toString();
                    emit serviceAdded(o.value(QStringLiteral("service")).toString(), DatabaseManager::SystemScope);
                }
                cache.append(o);
            }
            qSort(cache.begin(), cache.end(), serviceGreaterThan);
        }
        QJsonDbRemoveRequest *remove = qobject_cast<QJsonDbRemoveRequest *>(request);
        if (remove && !remove->objects().isEmpty()) {
            QStringList services;
            for (int i = 0; i < cache.count(); i++) {
                const QJsonObject cached_value = cache.at(i);
                foreach (const QJsonObject &o, remove->objects()) {
                    if (o.value(QStringLiteral("identifier")) ==
                            cached_value.value(QStringLiteral("identifier"))) {
                        QString service = o.value(QStringLiteral("service")).toString();

                        qServiceLog() << "class" << "dbm_jsondb"
                                      << "event" << "request remove"
                                      << "service" << service
                                      << "ident" << cached_value.value(QStringLiteral("identifier")).toString()
                                      << "dup" << services.contains(service);

                        if (!services.contains(service))
                            services.append(service);
                        cache_deleted_items.append(cached_value);
                        cache.removeAt(i);
                        i--;
                        break;
                    }
                }
            }

            foreach (const QString service, services) {
                bool contains = false;
                foreach (const QJsonObject &e, cache) {
                    if (e.value(QStringLiteral("service")).toString() == service) {
                        contains = true;
                        break;
                    }
                }
                if (!contains) {
                    qServiceLog() << "class" << "dbm_jsondb"
                                  << "event" << "notify remove"
                                  << "service" << service;
                    emit serviceRemoved(service, DatabaseManager::SystemScope);
                }
            }
            qServiceLog() << "class" << "dbm_jsondb"
                          << "event" << "unlock cache"
                          << "from" << "sendRequest";
        }
        QJsonDbUpdateRequest *update = qobject_cast<QJsonDbUpdateRequest *>(request);
        if (update) {
            for (int i = 0; i < cache.count(); i++) {
                const QJsonObject cached_value = cache.at(i);
                foreach (const QJsonObject &o, update->objects()) {
                    if (o.value(QStringLiteral("identifier")) ==
                            cached_value.value(QStringLiteral("identifier"))) {
                        qServiceLog() << "class" << "dbm_jsondb"
                                      << "event" << "request update"
                                      << "service" << o.value(QStringLiteral("service")).toString();
                        cache.replace(i, o);
                        i = cache.count() + 1; // exit both loops
                        break;
                    }
                }
            }
        }
    }

    return request->status() != QJsonDbRequest::Error;
}

void JsondbWorker::onNotification()
{
    QMutexLocker c(&cache_mutex);
    qServiceLog() << "class" << "dbm_jsondb"
                  << "event" << "lock cache"
                  << "from" << "onNotification";
    Q_ASSERT(sender() == dbwatcher);

    QStringList servicesAdded;
    QStringList servicesRemoved;

    QList<QJsonDbNotification> notifications = dbwatcher->takeNotifications();

    foreach (const QJsonDbNotification &n, notifications) {
        QJsonObject map = n.object();
        QString service = map.value(QStringLiteral("service")).toString();
        QString id = map.value(QStringLiteral("identifier")).toString();

        qServiceLog() << "class" << "dbm_jsondb"
                      << "event" << "notification"
                      << "service" << service;

        if (n.action() == QJsonDbWatcher::Created) {
            bool updated = false;
            bool contains = false;
            for (int i = 0; i < cache.count(); i++) {
                const QJsonObject &e = cache.at(i);
                // we add items to the cache on our own additons, so update
                // or ignore them as required */
                if (e.value(QLatin1String("identifier")).toString() == id) {
                    cache.replace(i, e);
                    updated = true;
                }

                if (e.value(QStringLiteral("service")) == service) {
                    contains = true;
                }
            }
            if (!updated) {
                cache.append(map);
                qSort(cache.begin(), cache.end(), serviceGreaterThan);
            }
            if (!contains) {
                servicesAdded += service;
            }
        } else if (n.action() == QJsonDbWatcher::Removed) {
            // if we removed the service ourselves we're already cleared out the
            // cache, and emitted the signal.  This will pass straight through
            bool containsService = false;
            foreach (const QJsonObject &e, cache) {
                QString e_id =  e.value(QStringLiteral("identifier")).toString();
                if (id == e_id) {
                    cache.removeOne(e);
                }
            }
            foreach (const QJsonObject &e, cache) {
                if (service == e.value(QLatin1String("service")).toString()) {
                    containsService = true;
                    break;
                }
            }
            foreach (const QJsonObject &e, cache_deleted_items) {
                if (service == e.value(QLatin1String("service")).toString()) {
                    containsService = true;
                    cache_deleted_items.removeOne(e);
                    break;
                }
            }
            if (!containsService) {
                servicesRemoved += service;
            }
        } else if (n.action() == QJsonDbWatcher::Updated) {
            for (int i = 0; i < cache.count(); i++) {
                if (id == cache.at(i).value(QStringLiteral("identifier")).toString()) {
                    cache.replace(i, map);
                }
            }
        }
    }

    // Do not emit these signals while the cache mutex is locked.
    qServiceLog() << "class" << "dbm_jsondb"
                  << "event" << "unlock cache"
                  << "from" << "onNotification";
    c.unlock();

    foreach (const QString &service, servicesAdded) {
        qServiceLog() << "class" << "dbm_jsondb"
                      << "event" << "emit serviceAdded"
                      << "service" << service;
        emit serviceAdded(service, DatabaseManager::SystemScope);
    }

    foreach (const QString &service, servicesRemoved) {
        qServiceLog() << "class" << "dbm_jsondb"
                      << "event" << "emit serviceRemoved"
                      << "service" << service;
        emit serviceRemoved(service, DatabaseManager::SystemScope);
    }

}

void JsondbWorker::reapThread()
{
    qServiceLog() << "class" << "dbm_jsondb"
                  << "event" << "idle shutdown";

    QMutexLocker locker(&thread_control_mutex);
    workerThread->quit();
    workerThread->wait();
    delete workerThread;
    workerThread = 0;
}

//Q_GLOBAL_STATIC(JsondbWorker, _q_service_jsondbworker);
JsondbWorker *_q_service_jsondbworker()
{
    static JsondbWorker *w = 0;
    static QMutex m;

    QMutexLocker l(&m);
    if (!w)
        w = new JsondbWorker();
    return w;
}

/*
    \class DatabaseManager
    The database manager is responsible for receiving queries about
    services and managing user and system scope databases in order to
    respond to those queries.

    It provides operations for
    - registering and unregistering services
    - querying for services and interfaces
    - setting and getting default interface implementations

    and provides notifications by emitting signals for added
    or removed services.

    Implementation note:
    When one of the above operations is first invoked a connection with the
    appropriate database(s) is opened.  This connection remains
    open until the DatabaseManager is destroyed.

    If the system scope database cannot be opened when performing
    user scope operations.  The operations are carried out as per normal
    but only acting on the user scope database.  Each operation invokation
    will try to open a connection with the system scope database.

    Terminology note:
    When referring to user scope regarding operations, it generally
    means access to both the user and system databases with the
    data from both combined into a single dataset.
    When referring to a user scope database it means the
    user database only.
*/


static QString makeHash(const QString& serviceInterface, const QString& service, const QString& version) {
    return QString::fromLatin1(QCryptographicHash::hash(QString(serviceInterface + service + version).toUtf8(), QCryptographicHash::Md4).toHex());
}


/*
   Constructor
*/
DatabaseManager::DatabaseManager()
    : m_notenabled(false)
{
}


/*
   Destructor
*/
DatabaseManager::~DatabaseManager()
{
    if (m_notenabled) {
        setChangeNotificationsEnabled(DatabaseManager::SystemScope, false);
    }
}

/*
   Fetch the cached data, and update the expiration timer
 */
const QList<QJsonObject> DatabaseManager::getCache()
{
    return _q_service_jsondbworker()->getCache();
}

/*
    Adds the details \a  service into the service database corresponding to
    \a scope.

    Returns true if the operation succeeded and false otherwise.
    The last error is set when this function is called.
*/
bool DatabaseManager::registerService(ServiceMetaDataResults &service, DbScope scope)
{
    Q_UNUSED(scope)
    m_lastError.setError(DBError::NoError);

    // Check if a service is already registered

    const QList<QJsonObject> &cache = getCache();

    foreach (const QServiceInterfaceDescriptor &d, service.interfaces) {
        QString version = QString(QLatin1String("%1.%2")).arg(d.majorVersion()).arg(d.minorVersion());

        QString hash = makeHash(d.interfaceName(),
                                d.serviceName(),
                                version);

        foreach (const QJsonObject &v, cache) {
            if (v.value(QLatin1String("indentifier")).toString() == hash) {
                QString alreadyRegisteredService = v.value(QLatin1String("service")).toString();
                const QString errorText = QLatin1String("Cannot register service \"%1\". Service \"%2\" is already "
                                                        "registered to service \"%3\".  It must first be deregistered "
                                                        "before it can be reregistered");

                m_lastError.setError(DBError::LocationAlreadyRegistered,
                                     errorText.arg(service.name)
                                     .arg(service.location)
                                     .arg(alreadyRegisteredService));

                qDebug() << "Failed" << m_lastError.text();
                return false;
            }
        }
    }

//    QVariantMap query;
//    query.insert(kQuery, QString(QLatin1String("[?_type=\"com.nokia.mt.core.Package\"][?identifier=\"%1\"]")).arg(service.location));
//    int id = db->find(query);
//    if (!waitForResponse(id)) {
//        qWarning() << "Can not find query the service registered as an Application with identifier" << service.location;
//        qWarning() << "Please check the info.json file is setup properly";
//    }

//    if (m_data.toMap()[QLatin1String("length")].toInt() == 0){
//        qWarning() << "Can not find the service registered as an Application with identifier" << service.location;
//        qWarning() << "Please check the info.json file is setup properly";
//    }

    QList<QJsonObject> objects;
    foreach (const QServiceInterfaceDescriptor &serviceInterface, service.interfaces) {
        QJsonObject interfaceData;
        interfaceData.insert(QLatin1String("_type"), QLatin1String("com.nokia.mt.serviceframework.interface"));
        QString version = QString(QLatin1String("%1.%2")).arg(serviceInterface.majorVersion()).arg(serviceInterface.minorVersion());
        interfaceData.insert(QLatin1String("identifier"), makeHash(serviceInterface.interfaceName(),
                                                             service.name,
                                                             version));
        interfaceData.insert(QLatin1String("location"), service.location);
        interfaceData.insert(QLatin1String("service"), service.name);
        interfaceData.insert(QLatin1String("interface"), serviceInterface.interfaceName());
        interfaceData.insert(QLatin1String("version"), QString(QLatin1String("%1.%2")).arg(serviceInterface.majorVersion()).arg(serviceInterface.minorVersion()));
        if (serviceInterface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).isValid())
            interfaceData.insert(QLatin1String("description"), serviceInterface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString());
        if (serviceInterface.attribute(QServiceInterfaceDescriptor::ServiceDescription).isValid())
            interfaceData.insert(QLatin1String("servicedescription"), serviceInterface.attribute(QServiceInterfaceDescriptor::ServiceDescription).toString());
        if (serviceInterface.attribute(QServiceInterfaceDescriptor::ServiceType).isValid())
            interfaceData.insert(QLatin1String("servicetype"), serviceInterface.attribute(QServiceInterfaceDescriptor::ServiceType).toString());
        QString caps = serviceInterface.attribute(QServiceInterfaceDescriptor::Capabilities).toString();
        if (!caps.isEmpty())
            interfaceData.insert(QLatin1String("capabilities"), QJsonArray::fromStringList(caps.split(QLatin1Char(','))));
        QJsonObject attData;
        foreach (const QString &attribute, serviceInterface.customAttributes()) {
            attData.insert(attribute, serviceInterface.customAttribute(attribute));
        }
        if (!attData.isEmpty())
            interfaceData.insert(QLatin1String("attributes"), attData);

        objects.append(interfaceData);
    }

    QJsonDbCreateRequest request(objects);
    if (!_q_service_jsondbworker()->sendRequest(&request)) {
        qDebug() << "Failed to create interface entry";
        return false;
    }

    return true;
}

/*
    Removes the details of \serviceName from the database corresponding to \a
    scope.

    Returns true if the operation succeeded, false otherwise.
    The last error is set when this function is called.
*/
bool DatabaseManager::unregisterService(const QString &serviceName, DbScope scope)
{
    Q_UNUSED(scope)
    m_lastError.setError(DBError::NoError);

    QJsonDbReadRequest request;
    request.setQuery(QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"][?service=\"%1\"]")
                     .arg(serviceName));
    if (!_q_service_jsondbworker()->sendRequest(&request)) {
        m_lastError.setError(DBError::DatabaseNotOpen, QString::fromLatin1("Unable to connect to database"));
        return false;
    }
    QList<QJsonObject> list = request.takeResults();
    if (list.isEmpty()) {
        m_lastError.setError(DBError::NotFound, QString::fromLatin1("Service not found: \"%1\"").arg(serviceName));
        return false;
    }

    QJsonDbRemoveRequest removeRequest(list);
    _q_service_jsondbworker()->sendRequest(&removeRequest);

    return true;
}

/*
    Removes the initialization specific information of \serviceName from the database
    corresponding to a \scope.

    Returns true if the operation succeeded, false otherwise.
    The last error is set when this function is called.
  */
bool DatabaseManager::serviceInitialized(const QString &serviceName, DbScope scope)
{
    Q_UNUSED(scope)
    m_lastError.setError(DBError::NoError);

    // Mark all interface defaults as not the default
    QString query = QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"][?service=\"%1\"]")
            .arg(serviceName);

    QJsonDbReadRequest request;
    request.setQuery(query);
    if (!_q_service_jsondbworker()->sendRequest(&request)) {
        m_lastError.setError(DBError::NotFound, QLatin1String("Unable to get response from jsondb."));
        return false;
    }

    QList<QJsonObject> res = request.takeResults();
    if (res.isEmpty()) {
        m_lastError.setError(DBError::NotFound, QLatin1String("Unable to find service"));
        return false;
    }

    QList<QJsonObject> objects;
    while (!res.isEmpty()) {
        QJsonObject entry = res.takeFirst();
        if (entry.contains(QLatin1String(SERVICE_INITIALIZED_ATTR))) {
            entry.remove(QLatin1String(SERVICE_INITIALIZED_ATTR));
            objects.append(entry);
        }
    }
    if (!objects.isEmpty()) {
        QJsonDbUpdateRequest request(objects);
        if (!_q_service_jsondbworker()->sendRequest(&request)) {
            qDebug() << "Failed to update interface entries" << objects;
            return false;
        }
    }
    return true;
}

/*
    Retrieves a list of interface descriptors that fulfill the constraints specified
    by \a filter at a given \a scope.

    The last error is set when this function is called.
*/
QList<QServiceInterfaceDescriptor>  DatabaseManager::getInterfaces(const QServiceFilter &filter, DbScope scope)
{
    Q_UNUSED(scope)

    m_lastError.setError(DBError::NoError);
    QList<QServiceInterfaceDescriptor> descriptors;

    const QList<QJsonObject> &cache = getCache();

    foreach (const QJsonObject &v, cache) {
        bool match_service = false;
        bool match_interface = false;
        bool match_version = false;
        bool match_caps = false;
        bool match_attrs = false;

        if (filter.interfaceName().isEmpty()) {
            match_interface = true;
        } else if (filter.interfaceName() == v.value(QLatin1String("interface")).toString()) {
            match_interface = true;
        }

        if (filter.serviceName().isEmpty()) {
            match_service = true;
        } else if (filter.serviceName() == v.value(QLatin1String("service")).toString()) {
            match_service = true;
        }

        if (filter.majorVersion() > 0 || filter.minorVersion() > 0) {
            bool ok;
            float versiondb = v.value(QLatin1String("version")).toString().toFloat(&ok);
            if (!ok) {
                match_version = false;
            } else {
                float versionfl = QString(QLatin1String("%1.%2")).arg(filter.majorVersion()).arg(filter.minorVersion()).toFloat(&ok);
                if ((filter.versionMatchRule() == QServiceFilter::MinimumVersionMatch && versionfl <= versiondb) ||
                    (filter.versionMatchRule() == QServiceFilter::ExactVersionMatch && versionfl == versiondb)) {
                    match_version = true;
                }
                else
                    match_version = false;
            }
        } else {
            match_version = true; // default to true if none listed
        }


        if (!filter.capabilities().isEmpty()) {
            qWarning() << "JsonDB does not support capability matching currently";
        }

        if (!filter.customAttributes().isEmpty()){
            qWarning() << "JsonDB does not support attribute matching currently";
        }

        // not supported at the moment
        match_caps = match_attrs = true;

        if (match_interface && match_service && match_version && match_caps && match_attrs) {
                QServiceInterfaceDescriptor serviceInterface;
                serviceInterface.d = new QServiceInterfaceDescriptorPrivate;
                serviceInterface.d->interfaceName = v.value(QLatin1String("interface")).toString();
                serviceInterface.d->serviceName = v.value(QLatin1String("service")).toString();
                serviceInterface.d->major = v.value(QLatin1String("version")).toString().split(QLatin1String(".")).at(0).toInt();
                serviceInterface.d->minor = v.value(QLatin1String("version")).toString().split(QLatin1String(".")).at(1).toInt();
                if (v.contains(QLatin1String("servicetype")))
                    serviceInterface.d->attributes[QServiceInterfaceDescriptor::ServiceType] = v.value(QLatin1String("servicetype")).toString().toInt();
                if (v.contains(QLatin1String("location")))
                    serviceInterface.d->attributes[QServiceInterfaceDescriptor::Location] = v.value(QLatin1String("location")).toString();
                if (v.contains(QLatin1String("description")))
                    serviceInterface.d->attributes[QServiceInterfaceDescriptor::InterfaceDescription] = v.value(QLatin1String("description")).toString();
                if (v.contains(QLatin1String("servicedescription")))
                    serviceInterface.d->attributes[QServiceInterfaceDescriptor::ServiceDescription] = v.value(QLatin1String("servicedescription")).toString();

                descriptors.append(serviceInterface);
            }
    }
    return descriptors;
}


/*
    Retrieves a list of the names of services that provide the interface
    specified by \a interfaceName.

    The last error is set when this function is called.
*/
QStringList DatabaseManager::getServiceNames(const QString &interfaceName, DatabaseManager::DbScope scope)
{
    Q_UNUSED(scope)
    m_lastError.setError(DBError::NoError);
    QStringList serviceNames;

    const QList<QJsonObject> &cache = getCache();

    foreach (const QJsonObject &v, cache) {
        if (interfaceName.isEmpty() || interfaceName == v.value(QLatin1String("interface")).toString()) {
            serviceNames.append(v.value(QLatin1String("service")).toString());
        }
    }

    serviceNames.removeDuplicates();
    return serviceNames;

}

/*
    Returns the default interface implementation descriptor for a given
    \a interfaceName and \a scope.

    The last error is set when this function is called.
*/
QServiceInterfaceDescriptor DatabaseManager::interfaceDefault(const QString &interfaceName, DbScope scope)
{
    Q_UNUSED(scope)
    m_lastError.setError(DBError::NoError);

    const QList<QJsonObject> &cache = getCache();

    QJsonObject entry;

    foreach (const QJsonObject &e, cache) {
        if (e.value(QLatin1String("interface")).toString() == interfaceName) {
            if (e.contains(QLatin1String("default"))) {
                entry = e;
                break;
            } else if (entry.isEmpty()) {
                entry = e;
            }
        }
    }

    if (entry.isEmpty()) {
        qWarning() << "SFW unable to find interface in jsondb for" << interfaceName;
        return QServiceInterfaceDescriptor();
    }

    QServiceInterfaceDescriptor serviceInterface;
    serviceInterface.d = new QServiceInterfaceDescriptorPrivate;
    serviceInterface.d->interfaceName = entry.value(QLatin1String("interface")).toString();
    serviceInterface.d->serviceName = entry.value(QLatin1String("service")).toString();
    serviceInterface.d->major = entry.value(QLatin1String("version")).toString().split(QLatin1String(".")).at(0).toInt();
    serviceInterface.d->minor = entry.value(QLatin1String("version")).toString().split(QLatin1String(".")).at(1).toInt();
    if (entry.contains(QLatin1String("servicetype")))
        serviceInterface.d->attributes[QServiceInterfaceDescriptor::ServiceType] = entry.value(QLatin1String("servicetype")).toString().toInt();
    if (entry.contains(QLatin1String("location")))
        serviceInterface.d->attributes[QServiceInterfaceDescriptor::Location] = entry.value(QLatin1String("location")).toString();
    if (entry.contains(QLatin1String("description")))
        serviceInterface.d->attributes[QServiceInterfaceDescriptor::InterfaceDescription] = entry.value(QLatin1String("description")).toString();
    if (entry.contains(QLatin1String("servicedescription")))
        serviceInterface.d->attributes[QServiceInterfaceDescriptor::ServiceDescription] = entry.value(QLatin1String("servicedescription")).toString();
    if (entry.contains(QLatin1String("capabilities")))
        serviceInterface.d->attributes[QServiceInterfaceDescriptor::Capabilities] = entry.value(QLatin1String("capabilities")).toString();

    return serviceInterface;
}

bool lessThan(const QServiceInterfaceDescriptor &d1,
                                        const QServiceInterfaceDescriptor &d2)
{
        return (d1.majorVersion() < d2.majorVersion())
                || ( d1.majorVersion() == d2.majorVersion()
                && d1.minorVersion() < d2.minorVersion());
}

/*
    Sets the default interface implemenation for \a interfaceName to the matching
    interface implementation provided by \a service.

    If \a service provides more than one interface implementation, the newest
    version of the interface is set as the default.

    Returns true if the operation was succeeded, false otherwise
    The last error is set when this function is called.
*/
bool DatabaseManager::setInterfaceDefault(const QString &serviceName,
                                          const QString &interfaceName, DbScope scope)
{
    m_lastError.setError(DBError::NoError);
    QList<QServiceInterfaceDescriptor> descriptors;
    QServiceFilter filter;
    filter.setServiceName(serviceName);
    filter.setInterface(interfaceName);

    descriptors = getInterfaces(filter, scope);
    if (m_lastError.code() != DBError::NoError)
        return false;

    if (descriptors.count() == 0) {
        QString errorText = QString::fromLatin1(
                    "No implementation for interface \"%1\" "
                    "found for service \"%2\"");
        m_lastError.setError(DBError::NotFound,
                errorText.arg(interfaceName)
                .arg(serviceName));
        qDebug() << "found nothing";
        return false;
    }

    //find the descriptor with the latest version
    int latestIndex = 0;
    for (int i = 0; i < descriptors.count(); ++i) {
        if (lessThan(descriptors[latestIndex], descriptors[i]))
            latestIndex = i;
    }

    return setInterfaceDefault(descriptors[latestIndex], scope);
}

/*
    Sets the interface implementation specified by \a descriptor to be the default
    implementation for the particular interface specified in the descriptor.

    Returns true if the operation succeeded, false otherwise.
    The last error is set when this function is called.
*/
bool DatabaseManager::setInterfaceDefault(const QServiceInterfaceDescriptor &serviceInterface, DbScope scope)
{
    Q_UNUSED(scope)
    m_lastError.setError(DBError::NoError);

    qDebug() << "Set default to" << serviceInterface.interfaceName() << serviceInterface.serviceName();

    // Mark all interface defaults as not the default
    QJsonDbReadRequest request;
    request.setQuery(QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"][?interface=\"%1\"]")
                     .arg(serviceInterface.interfaceName()));
    if (!_q_service_jsondbworker()->sendRequest(&request)) {
        qDebug() << "Found nothing";
        return false;
    }

    QList<QJsonObject> objects;
    QList<QJsonObject> res = request.takeResults();
    while (!res.empty()) {
        QJsonObject entry = res.takeFirst();
        if (entry.contains(QLatin1String("default"))) {
            entry.remove(QLatin1String("default"));
            objects.append(entry);
        }
    }
    if (!objects.isEmpty()) {
        QJsonDbUpdateRequest request(objects);
        if (!_q_service_jsondbworker()->sendRequest(&request))
            qDebug() << "Failed to update" << objects;
    }

    // Fetch the entry
    QString version = QString(QLatin1String("%1.%2")).arg(serviceInterface.majorVersion()).arg(serviceInterface.minorVersion());
    QString hash = makeHash(serviceInterface.interfaceName(), serviceInterface.serviceName(), version);
    request.setQuery(QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"][?identifier=\"%1\"]")
                     .arg(hash));
    if (!_q_service_jsondbworker()->sendRequest(&request)) {
        qDebug() << "Found nothing";
        return false;
    }

    res = request.takeResults();
    if (res.isEmpty()) {
        qDebug() << "Can't find interface" << hash << serviceInterface.interfaceName() << " and service " << serviceInterface.serviceName() <<
                    "version" << serviceInterface.majorVersion() << serviceInterface.minorVersion();
        return false;
    } else if (res.count() > 1) {
        qWarning() << "Found more than one interface with exactly the same signature, something is wrong" << res.count();
        return false;
    }

    QJsonObject entry = res.takeFirst();
    entry.insert(QLatin1String("default"), true);
    QJsonDbUpdateRequest updateRequest(entry);
    if (!_q_service_jsondbworker()->sendRequest(&updateRequest)) {
        return false;
    }
    return true;
}

/*
    Sets whether change notifications for added and removed services are
    \a enabled or not at a given \a scope.
*/
void DatabaseManager::setChangeNotificationsEnabled(DbScope scope, bool enabled)
{
    Q_UNUSED(scope)

    if (m_notenabled && enabled)
        return;

    m_lastError.setError(DBError::NoError);

    if (enabled) {
        /* wait for the cache so that we're in a consistent state */
        _q_service_jsondbworker()->waitForCache();

        connect(_q_service_jsondbworker(), SIGNAL(serviceAdded(QString,DatabaseManager::DbScope)),
                this, SIGNAL(serviceAdded(QString,DatabaseManager::DbScope)));
        connect(_q_service_jsondbworker(), SIGNAL(serviceRemoved(QString,DatabaseManager::DbScope)),
                this, SIGNAL(serviceRemoved(QString,DatabaseManager::DbScope)));
    } else {
        disconnect(this, SIGNAL(serviceAdded(QString,DatabaseManager::DbScope)));
        disconnect(this, SIGNAL(serviceRemoved(QString,DatabaseManager::DbScope)));
    }

    m_notenabled = enabled;
}

#include "moc_databasemanager_jsondb_p.cpp"
#include "databasemanager_jsondb.moc"

QT_END_NAMESPACE
