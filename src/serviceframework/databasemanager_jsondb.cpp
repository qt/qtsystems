/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
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

#include <QCoreApplication>
#include <QTimer>
#include <QThread>
#include <QJsonArray>
#include <QJsonObject>

#include <QtJsonDb/qjsondbconnection.h>
#include <QtJsonDb/qjsondbreadrequest.h>
#include <QtJsonDb/qjsondbwriterequest.h>
#include <QtJsonDb/qjsondbwatcher.h>

#include "databasemanager_jsondb_p.h"

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE_JSONDB

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


static QString makeHash(const QString& interface, const QString& service, const QString& version) {
    return QString::fromLatin1(QCryptographicHash::hash(QString(interface + service + version).toUtf8(), QCryptographicHash::Md4).toHex());
}

/*
   Constructor
*/
DatabaseManager::DatabaseManager()
    : db(0), dbwatcher(new QJsonDbWatcher(this)), m_notenabled(false)
{
    db = QJsonDbConnection::defaultConnection();
    if (db->thread() != QThread::currentThread())
        db = new QJsonDbConnection(this);
    db->connectToServer();

    connect(dbwatcher, SIGNAL(notificationsAvailable(int)), this, SLOT(onNotificationsAvailable()));
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
    QServiceInterfaceDescriptor interface;

    foreach (interface, service.interfaces) {

        QString version = QString(QLatin1String("%1.%2")).arg(interface.majorVersion()).arg(interface.minorVersion());

        QString hash = makeHash(interface.interfaceName(),
                                service.name,
                                version);

        QJsonDbReadRequest request;
        request.setQuery(QString(QStringLiteral("[?_type=\"com.nokia.mt.serviceframework.interface\"][?identifier=\"%1\"]")).arg(hash));
        if (!sendRequest(&request)) {
            qDebug() << "Db error" << m_lastError.text() << request.query();
            m_lastError.setError(DBError::InvalidDatabaseFile, QLatin1String("Cannot register service. Unable to contact database."));
            return false;
        }
        QList<QJsonObject> res = request.takeResults();
        if (!res.isEmpty()) {
            QString alreadyRegisteredService = res.first().value(QLatin1String("service")).toString();
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
    foreach (const QServiceInterfaceDescriptor &interface, service.interfaces) {
        QJsonObject interfaceData;
        interfaceData.insert(QLatin1String("_type"), QLatin1String("com.nokia.mt.serviceframework.interface"));
        QString version = QString(QLatin1String("%1.%2")).arg(interface.majorVersion()).arg(interface.minorVersion());
        interfaceData.insert(QLatin1String("identifier"), makeHash(interface.interfaceName(),
                                                             service.name,
                                                             version));
        interfaceData.insert(QLatin1String("location"), service.location);
        interfaceData.insert(QLatin1String("service"), service.name);
        interfaceData.insert(QLatin1String("interface"), interface.interfaceName());
        interfaceData.insert(QLatin1String("version"), QString(QLatin1String("%1.%2")).arg(interface.majorVersion()).arg(interface.minorVersion()));
        if (interface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).isValid())
            interfaceData.insert(QLatin1String("description"), interface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString());
        if (interface.attribute(QServiceInterfaceDescriptor::ServiceDescription).isValid())
            interfaceData.insert(QLatin1String("servicedescription"), interface.attribute(QServiceInterfaceDescriptor::ServiceDescription).toString());
        if (interface.attribute(QServiceInterfaceDescriptor::ServiceType).isValid())
            interfaceData.insert(QLatin1String("servicetype"), interface.attribute(QServiceInterfaceDescriptor::ServiceType).toInt());
        QString caps = interface.attribute(QServiceInterfaceDescriptor::Capabilities).toString();
        if (!caps.isEmpty())
            interfaceData.insert(QLatin1String("capabilities"), QJsonArray::fromStringList(caps.split(QLatin1Char(','))));
        QJsonObject attData;
        foreach (const QString &attribute, interface.customAttributes()) {
            attData.insert(attribute, interface.customAttribute(attribute));
        }
        if (!attData.isEmpty())
            interfaceData.insert(QLatin1String("attributes"), attData);

        objects.append(interfaceData);
    }

    QJsonDbCreateRequest request(objects);
    if (!sendRequest(&request)) {
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
    //qDebug() << Q_FUNC_INFO << serviceName;

    QJsonDbReadRequest request;
    request.setQuery(QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"][?service=\"%1\"]")
                     .arg(serviceName));
    if (!sendRequest(&request)) {
        m_lastError.setError(DBError::DatabaseNotOpen, QString::fromLatin1("Unable to connect to database"));
        return false;
    }
    QList<QJsonObject> list = request.takeResults();
    if (list.isEmpty()) {
        m_lastError.setError(DBError::NotFound, QString::fromLatin1("Service not found: \"%1\"").arg(serviceName));
        return false;
    }

    QJsonDbRemoveRequest removeRequest(list);
    sendRequest(&removeRequest);

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
    if (!sendRequest(&request)) {
        qDebug() << "Did query" << query;
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
        if (!sendRequest(&request)) {
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
//    qDebug() << ":!:" << __FUNCTION__;
    QList<QServiceInterfaceDescriptor> descriptors;

    QString query;
    if (!filter.serviceName().isEmpty()) {
        query = QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"][?service=\"%1\"]")
                .arg(filter.serviceName());
    } else if (!filter.interfaceName().isEmpty()) {
        query = QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"][?interface=\"%1\"]")
                .arg(filter.interfaceName());
    } else {
        query = QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"]");
    }

    QTime ticker;
    ticker.start();
    QJsonDbReadRequest request;
    request.setQuery(query);
    if (!sendRequest(&request))
        return descriptors;
    qDebug() << "SFW jsondb call took" << ticker.elapsed() << "ms";


    QList<QJsonObject> list = request.takeResults();
    foreach (const QJsonObject &v, list) {
        bool match = false;
        if (filter.interfaceName().isEmpty()) {
            match = true;
        } else if (filter.interfaceName() == v.value(QLatin1String("interface")).toString()) {
            match = true;
        }

        if (filter.majorVersion() > 0 || filter.minorVersion() > 0) {
            bool ok;
            float versiondb = v.value(QLatin1String("version")).toString().toFloat(&ok);
            if (!ok) {
                match = false;
            } else {
                float versionfl = QString(QLatin1String("%1.%2")).arg(filter.majorVersion()).arg(filter.minorVersion()).toFloat(&ok);
                if ((filter.versionMatchRule() == QServiceFilter::MinimumVersionMatch && versionfl <= versiondb) ||
                    (filter.versionMatchRule() == QServiceFilter::ExactVersionMatch && versionfl == versiondb))
                    match = true;
                else
                    match = false;
            }
        }

        if (!filter.capabilities().isEmpty()) {
            qWarning() << "JsonDB does not support capability matching currently";
        }

        if (!filter.customAttributes().isEmpty()){
            qWarning() << "JsonDB does not support attribute matching currently";
        }

        if (match) {
                QServiceInterfaceDescriptor interface;
                interface.d = new QServiceInterfaceDescriptorPrivate;
                interface.d->interfaceName = v.value(QLatin1String("interface")).toString();
                interface.d->serviceName = v.value(QLatin1String("service")).toString();
                interface.d->major = v.value(QLatin1String("version")).toString().split(QLatin1String(".")).at(0).toInt();
                interface.d->minor = v.value(QLatin1String("version")).toString().split(QLatin1String(".")).at(1).toInt();
                if (v.contains(QLatin1String("servicetype")))
                    interface.d->attributes[QServiceInterfaceDescriptor::ServiceType] = static_cast<int>(v.value(QLatin1String("servicetype")).toDouble());
                if (v.contains(QLatin1String("location")))
                    interface.d->attributes[QServiceInterfaceDescriptor::Location] = v.value(QLatin1String("location")).toString();
                if (v.contains(QLatin1String("description")))
                    interface.d->attributes[QServiceInterfaceDescriptor::InterfaceDescription] = v.value(QLatin1String("description")).toString();
                if (v.contains(QLatin1String("servicedescription")))
                    interface.d->attributes[QServiceInterfaceDescriptor::ServiceDescription] = v.value(QLatin1String("servicedescription")).toString();

                descriptors.append(interface);
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

    // Check if a service is already registered with the given location
    QString query;
    if (interfaceName.isEmpty())
        query = QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"]");
    else
        query = QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"][?interface=\"%1\"]").arg(interfaceName);
    QJsonDbReadRequest request;
    request.setQuery(query);
    if (!sendRequest(&request)) {
//        qDebug() << "Found nothing";
        return serviceNames;
    }

    QList<QJsonObject> res = request.takeResults();
    while (!res.empty()) {
        QJsonObject entry = res.takeFirst();
        serviceNames.append(entry.value(QLatin1String("service")).toString());
    }
    serviceNames.removeDuplicates();
//    qDebug() << "Returning" << serviceNames;
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

    // Mark all interface defaults as not the default
    QString query = QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"][?interface=\"%1\"][?default=true]")
                 .arg(interfaceName);
    QJsonDbReadRequest request;
    request.setQuery(query);
    if (!sendRequest(&request)) {
        qDebug() << "Did query" << request.query();
        m_lastError.setError(DBError::NotFound, QLatin1String("Unable to get response from jsondb."));
        return QServiceInterfaceDescriptor();
    }

    QList<QJsonObject> res = request.takeResults();
    if (res.isEmpty()) {
        request.setQuery(QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"][?interface=\"%1\"][/service]")
                         .arg(interfaceName));
        if (!sendRequest(&request)) {
            m_lastError.setError(DBError::NotFound, QLatin1String("Error fetching interface data from jsondb, failed to get result."));
            return QServiceInterfaceDescriptor();
        }
        res = request.takeResults();
        if (res.isEmpty()) {
            m_lastError.setError(DBError::NotFound, QLatin1String("Interface not found."));
            return QServiceInterfaceDescriptor();
        }

    }

    QJsonObject entry = res.takeFirst();

    QServiceInterfaceDescriptor interface;
    interface.d = new QServiceInterfaceDescriptorPrivate;
    interface.d->interfaceName = entry.value(QLatin1String("interface")).toString();
    interface.d->serviceName = entry.value(QLatin1String("service")).toString();
    interface.d->major = entry.value(QLatin1String("version")).toString().split(QLatin1String(".")).at(0).toInt();
    interface.d->minor = entry.value(QLatin1String("version")).toString().split(QLatin1String(".")).at(1).toInt();
    if (entry.contains(QLatin1String("servicetype")))
        interface.d->attributes[QServiceInterfaceDescriptor::ServiceType] = static_cast<int>(entry.value(QLatin1String("servicetype")).toDouble());
    if (entry.contains(QLatin1String("location")))
        interface.d->attributes[QServiceInterfaceDescriptor::Location] = entry.value(QLatin1String("location")).toString();
    if (entry.contains(QLatin1String("description")))
        interface.d->attributes[QServiceInterfaceDescriptor::InterfaceDescription] = entry.value(QLatin1String("description")).toString();
    if (entry.contains(QLatin1String("servicedescription")))
        interface.d->attributes[QServiceInterfaceDescriptor::ServiceDescription] = entry.value(QLatin1String("servicedescription")).toString();
    if (entry.contains(QLatin1String("capabilities")))
        interface.d->attributes[QServiceInterfaceDescriptor::Capabilities] = entry.value(QLatin1String("capabilities")).toString();

    return interface;
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
        return false;
    }

    //find the descriptor with the latest version
    int latestIndex = 0;
        for (int i = 1; i < descriptors.count(); ++i) {
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
bool DatabaseManager::setInterfaceDefault(const QServiceInterfaceDescriptor &descriptor, DbScope scope)
{
    Q_UNUSED(scope)
    m_lastError.setError(DBError::NoError);

    // Mark all interface defaults as not the default
    QJsonDbReadRequest request;
    request.setQuery(QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"][?interface=\"%1\"]")
                     .arg(descriptor.interfaceName()));
    if (!sendRequest(&request)) {
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
        if (!sendRequest(&request))
            qDebug() << "Failed to update" << objects;
    }

    // Fetch the entry
    QString version = QString(QLatin1String("%1.%2")).arg(descriptor.majorVersion()).arg(descriptor.minorVersion());
    QString hash = makeHash(descriptor.interfaceName(), descriptor.serviceName(), version);
    request.setQuery(QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"][?identifier=\"%1\"]")
                     .arg(hash));
    if (!sendRequest(&request)) {
        qDebug() << "Found nothing";
        return false;
    }

    res = request.takeResults();
    if (res.isEmpty()) {
        qDebug() << "Can't find interface" << hash << descriptor.interfaceName() << " and service " << descriptor.serviceName() <<
                    "version" << descriptor.majorVersion() << descriptor.minorVersion();
        return false;
    } else if (res.count() > 1) {
        qWarning() << "Found more than one interface with exactly the same signature, something is wrong" << res.count();
        return false;
    }

    QJsonObject entry = res.takeFirst();
    entry.insert(QLatin1String("default"), true);
    QJsonDbUpdateRequest updateRequest(entry);
    if (!sendRequest(&updateRequest)) {
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

    m_notenabled = enabled;

    if (enabled) {
        m_lastError.setError(DBError::NoError);

        QJsonDbReadRequest request;
        request.setQuery(QString::fromLatin1("[?_type=\"com.nokia.mt.serviceframework.interface\"]"));
        sendRequest(&request);
        m_services.clear();

        QList<QJsonObject> res = request.takeResults();
        while (!res.empty()) {
            QJsonObject entry = res.takeFirst();
            QString service = entry.value(QLatin1String("service")).toString();
            m_services.insert(service, m_services.value(service)+1);
        }

        dbwatcher->setWatchedActions(QJsonDbWatcher::Created | QJsonDbWatcher::Removed);
        dbwatcher->setQuery(QLatin1String("[?_type=\"com.nokia.mt.serviceframework.interface\"]"));
        db->addWatcher(dbwatcher);
    } else {
        db->removeWatcher(dbwatcher);
        m_services.clear();
    }
}

bool DatabaseManager::sendRequest(QJsonDbRequest *request)
{
    if (m_lastError.code() != DBError::NoError)
        return false;

    connect(request, SIGNAL(finished()), &m_eventLoop, SLOT(quit()));
    connect(request, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            &m_eventLoop, SLOT(quit()));

    static const int JSON_EXPIRATION_TIMER = 2000;

    QTimer timer;
    timer.setSingleShot(true);
    timer.start(JSON_EXPIRATION_TIMER);
    connect(&timer, SIGNAL(timeout()), &m_eventLoop, SLOT(quit()));

    if (!db->send(request)) {
        Q_ASSERT(false);
        return false;
    }
    m_eventLoop.exec(QEventLoop::AllEvents);

    if (!timer.isActive())
        return false;

    if (request->status() == QJsonDbRequest::Error)
        m_lastError.setSQLError(QStringLiteral("jsondb request failed"));
    return m_lastError.code() == DBError::NoError;
}

void DatabaseManager::onNotificationsAvailable()
{
    if (!m_notenabled)
        return;
    Q_ASSERT(sender() == dbwatcher);

    QList<QJsonDbNotification> notifications = dbwatcher->takeNotifications();

    foreach (const QJsonDbNotification &n, notifications) {
        QJsonObject map = n.object();
        QString service = map.value(QLatin1String("service")).toString();

        if (n.action() == QJsonDbWatcher::Created) {
            if (!m_services.contains(service))
                emit serviceAdded(service, DatabaseManager::SystemScope);
            m_services.insert(service, m_services.value(service)+1);
        } else if (n.action() == QJsonDbWatcher::Removed) {
            if (m_services.value(service) > 1) {
                m_services.insert(service, m_services.value(service)-1);
            } else if (m_services.value(service) == 1) {
                emit serviceRemoved(service, DatabaseManager::SystemScope);
                m_services.remove(service);
            }
        }
    }
}

#include "moc_databasemanager_jsondb_p.cpp"

QT_END_NAMESPACE
