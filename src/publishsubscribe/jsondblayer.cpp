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

#if !defined(QT_NO_JSONDBLAYER)

#include "jsondblayer_p.h"

#include <QStringList>
#include <QThread>
#include <QVariantMap>
#include <QSet>

QT_BEGIN_NAMESPACE

//#define DEBUG_ON

#ifdef DEBUG_ON
#include <qdebug.h>
#define DEBUG_MSG(msg) qDebug()<<msg;
#define DEBUG_SIGNATURE qDebug()<<Q_FUNC_INFO;
#else
#define DEBUG_MSG(msg)
#define DEBUG_SIGNATURE
#endif

#define handleToJsonDbHandle(handle) reinterpret_cast<JsonDbHandle*>(handle)

#define WAIT_FOR_THREAD_TIMEOUT 500

static const QString applicationPartition() { return QStringLiteral("Private"); }
static const QString systemPartition() { return QStringLiteral("com.nokia.mt.Settings"); }

static QString getPartition(const QString &identifier)
{
    if (identifier.startsWith(QStringLiteral("com.nokia.mt.settings")))
        return systemPartition();
    else
        return applicationPartition();
}

JsonDbPath::JsonDbPath() : path(QStringLiteral(""))
{
}

JsonDbPath::JsonDbPath(const QString &path)
{
    this->path = JsonDbPath::normalizePath(path);
}

JsonDbPath::JsonDbPath(const JsonDbPath &other)
{
    path = other.path;
}

JsonDbPath &JsonDbPath::operator=(const JsonDbPath &other)
{
    path = other.path;

    return *this;
}

bool JsonDbPath::operator==(const JsonDbPath &other) const
{
    return this->path == other.path;
}

JsonDbPath JsonDbPath::operator+(const QString &str) const
{
    return *this + JsonDbPath(str);
}

JsonDbPath JsonDbPath::operator+(const JsonDbPath &other) const
{
    JsonDbPath p(*this);

    if (other.path.length() == 0)
        return p;

    if (p.path.length() > 0)
        p.path += QLatin1String(".") + other.path;
    else
        p.path = other.path;

    return p;
}

QString JsonDbPath::normalizePath(const QString &path)
{
    QString result = path.trimmed().replace(QLatin1Char('/'), QLatin1Char('.'));

    if (result.startsWith(QLatin1Char('.')))
        result = result.mid(1);

    if (result.endsWith(QLatin1Char('.')))
        result.chop(1);

    return result;
}

QStringList JsonDbPath::getIdentifier(const QString &path)
{
    QRegExp reg(QStringLiteral("(.*)[.]([^.]+)"));
    if (reg.indexIn(path) == -1) {
        return QStringList(path)<<path;
    }

    return reg.capturedTexts().mid(1);
}



JsonDbSyncCall::JsonDbSyncCall(const QString &partition, const QString &query, QList<QJsonObject> *result):
    mQuery(query),
    mObject(NULL),
    mPartition(partition),
    mResult(result),
    mSuccess(false),
    mConnection(NULL),
    mReadRequest(NULL),
    mUpdateRequest(NULL)
{
    startTimer(WAIT_FOR_THREAD_TIMEOUT);
}

JsonDbSyncCall::JsonDbSyncCall(const QString &partition, const QJsonObject *object):
    mQuery(QStringLiteral("")),
    mObject(object),
    mPartition(partition),
    mResult(NULL),
    mSuccess(false),
    mConnection(NULL),
    mReadRequest(NULL),
    mUpdateRequest(NULL)
{
    startTimer(WAIT_FOR_THREAD_TIMEOUT);
}

JsonDbSyncCall::~JsonDbSyncCall()
{
    DEBUG_SIGNATURE

    if (mReadRequest)
        delete mReadRequest;

    if (mUpdateRequest)
        delete mUpdateRequest;

    if (mConnection)
        delete mConnection;
}

void JsonDbSyncCall::timerEvent(QTimerEvent*)
{
    DEBUG_SIGNATURE

    QThread::currentThread()->quit();
}

void JsonDbSyncCall::createSyncReadRequest()
{
    DEBUG_SIGNATURE

    mConnection = new QtJsonDb::QJsonDbConnection;
    mConnection->connectToServer();

    mReadRequest = new QtJsonDb::QJsonDbReadRequest;

    mReadRequest->setQuery(mQuery);
    mReadRequest->setPartition(mPartition);

    connect(mReadRequest,
            SIGNAL(resultsAvailable(int)),
            this,
            SLOT(handleResponse(int)));

    connect(mReadRequest,
            SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            this,
            SLOT(handleError(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));

    mConnection->send(mReadRequest);
}

void JsonDbSyncCall::createSyncUpdateRequest()
{
    DEBUG_SIGNATURE

    mConnection = new QtJsonDb::QJsonDbConnection;
    mConnection->connectToServer();

    mUpdateRequest = new QtJsonDb::QJsonDbUpdateRequest(*mObject);
    mUpdateRequest->setPartition(mPartition);

    connect(mUpdateRequest,
            SIGNAL(finished()),
            this,
            SLOT(handleFinished()));

    connect(mUpdateRequest,
            SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            this,
            SLOT(handleError(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));

    mConnection->send(mUpdateRequest);
}

void JsonDbSyncCall::handleResponse(int)
{
    DEBUG_SIGNATURE

    *mResult = mReadRequest->takeResults();

    QThread::currentThread()->quit();
}

void JsonDbSyncCall::handleError(QtJsonDb::QJsonDbRequest::ErrorCode code, QString)
{
    DEBUG_SIGNATURE

    Q_ASSERT(code != QtJsonDb::QJsonDbRequest::InvalidPartition);

    mSuccess = false;

    QThread::currentThread()->quit();
}

void JsonDbSyncCall::handleFinished()
{
    DEBUG_SIGNATURE

    mSuccess = true;

    QThread::currentThread()->quit();
}

bool JsonDbSyncCall::wasSuccessful() const
{
    return mSuccess;
}


JsonDbHandle::JsonDbHandle( JsonDbHandle *parent,
                            const QString &root,
                            QValueSpace::LayerOptions) : QObject(parent), client(NULL)
{
    DEBUG_SIGNATURE

    if (parent != NULL)
        this->path = parent->path + root;
    else
        this->path = JsonDbPath(root);

    connected = false;
}

JsonDbHandle::~JsonDbHandle()
{
    DEBUG_SIGNATURE

    unsubscribe();

    if (client)
        delete client;
}

bool JsonDbHandle::value(const QString &path, QVariant *data)
{
    DEBUG_SIGNATURE

    QString wholePath = getWholePath(path);
    QString partition = getPartition(wholePath);

    // Assume caller wants to get the value of a setting
    QList<QJsonObject> result = getResponse(partition, getSettingQuery(wholePath));

    if (!result.count() || result[0].isEmpty()) {
        // Assume the caller wants to get the whole settings object
        result = getResponse(partition, getObjectQuery(wholePath));

        if (!result.count()) {
            return false;
        }

        *data = result.at(0).toVariantMap();
    } else {
        *data = result.at(0).value(QStringLiteral("value")).toVariant();
    }

    return !data->isNull();
}

bool JsonDbHandle::setValue(const QString &path, const QVariant &data)
{
    DEBUG_SIGNATURE

    QString wholePath = getWholePath(path);
    QStringList parts = JsonDbPath::getIdentifier(wholePath);

    QJsonObject updateObject = getObject(parts[0], IDENTIFIER);

    if (!updateObject.contains(QStringLiteral("settings"))) {
        // Object does not contain settings dictionary
        return false;
    }

    QJsonObject settings = updateObject.value(QStringLiteral("settings")).toObject();
    if (!settings.contains(parts[1])) {
        // Settings object does not contain the setting
        return false;
    }

    settings[parts[1]] = QJsonValue::fromVariant(data);
    updateObject[QStringLiteral("settings")] = settings;

    return doUpdateRequest(getPartition(wholePath), updateObject);
}

bool JsonDbHandle::doUpdateRequest(const QString &partition, const QJsonObject &updateObject)
{
    QThread syncThread;
    JsonDbSyncCall *call = new JsonDbSyncCall(partition, &updateObject);

    connect(&syncThread,
            SIGNAL(started()),
            call,
            SLOT(createSyncUpdateRequest()));

    call->moveToThread(&syncThread);

    syncThread.start();
    syncThread.wait();

    bool success = call->wasSuccessful();

    call->deleteLater();

    return success;
}

QJsonObject JsonDbHandle::getObject(const QString &identifier, const QString &property)
{
    DEBUG_SIGNATURE

            QList<QJsonObject> result = getResponse(getPartition(identifier),
                                                    QString(QStringLiteral("[?%1=\"%2\"]%3")).
                                                    arg(property).
                                                    arg(identifier).
                                                    arg(SETTINGS_FILTER));

    if (!result.count())
        return QJsonObject();

    return result.at(0);
}

QList<QJsonObject> JsonDbHandle::getResponse(const QString &partition, const QString& query)
{
    DEBUG_SIGNATURE
    DEBUG_MSG(QString("Query: " + query).toStdString().c_str())

    QList<QJsonObject> result;
    QThread syncThread;
    JsonDbSyncCall *call = new JsonDbSyncCall(partition, query, &result);

    connect(&syncThread,
            SIGNAL(started()),
            call,
            SLOT(createSyncReadRequest()));

    connect(&syncThread,
            SIGNAL(finished()),
            call,
            SLOT(deleteLater()));

    call->moveToThread(&syncThread);

    syncThread.start();
    syncThread.wait();

    return result;
}

QString JsonDbHandle::getWholePath(const QString &path) const
{
    DEBUG_SIGNATURE

    QString identifier = this->path.getPath();

    if (identifier.length() == 0)
        identifier = path;
    else if (path.length() > 0)
        identifier += QLatin1String(".") + path;

    return JsonDbPath::normalizePath(identifier);
}

bool JsonDbHandle::unsetValue(const QString&)
{
    DEBUG_SIGNATURE

    // Not supported
    return false;
}

void JsonDbHandle::subscribe()
{
    DEBUG_SIGNATURE

    if (connected) {
        return;
    }

    QString wholePath = getWholePath(QStringLiteral(""));
    QString query;
    QtJsonDb::QJsonDbWatcher::Actions actions;

    getNotificationQueryAndActions(wholePath, query, actions);

    if (!client) {
        client = new QtJsonDb::QJsonDbConnection();
        client->connectToServer();
    }

    watcher = new QtJsonDb::QJsonDbWatcher;
    watcher->setQuery(query);
    watcher->setPartition(getPartition(wholePath));
    watcher->setWatchedActions(actions);

    connect(watcher,
            SIGNAL(notificationsAvailable(int)),
            this,
            SLOT(onNotificationsAvailable(int)));

    client->addWatcher(watcher);

    connected = true;
}

void JsonDbHandle::onNotificationsAvailable(int count)
{
    DEBUG_MSG("!--- JsonDbHandle::onNotificationsAvailable(int) ---!")

    watcher->takeNotifications(count);

    emit valueChanged();
}

void JsonDbHandle::onError(int, int, const QString&)
{
    DEBUG_SIGNATURE
}

void JsonDbHandle::getNotificationQueryAndActions(QString path, QString &query, QtJsonDb::QJsonDbWatcher::Actions& actions)
{
    DEBUG_SIGNATURE

    QVariant object;

    actions = QtJsonDb::QJsonDbWatcher::Updated;

    if (!value(QStringLiteral(""), &object)) {
        // Neither a settings object nor a setting in a settings object was found
        // Probably the caller wants to subscribe somewhere on the top in the namespace
        query = QString(QStringLiteral("[?identifier startsWith \"%1\"]%2")).
                    arg(path).
                    arg(SETTINGS_FILTER);

        actions |= QtJsonDb::QJsonDbWatcher::Created | QtJsonDb::QJsonDbWatcher::Removed;
    } else {
        if (object.toMap().contains(QStringLiteral("_type"))) {
            // Path matches a settings object subscribe for that object
            query = getObjectQuery(path);

            actions |= QtJsonDb::QJsonDbWatcher::Created | QtJsonDb::QJsonDbWatcher::Removed;
        } else {
            // Path matches a setting in a settings object subscribe for that setting
            query = getSettingQuery(path);
        }
    }
}

QString JsonDbHandle::getObjectQuery(const QString &identifier)
{
    return QString(QStringLiteral("[?identifier=\"%1\"]%2")).
            arg(identifier).
            arg(SETTINGS_FILTER);
}

QString JsonDbHandle::getSettingQuery(const QString &identifier)
{
    QStringList parts = JsonDbPath::getIdentifier(identifier);
    return QString(QStringLiteral("[?identifier=\"%1\"]%2[={value:settings.%3}]")).
                arg(parts[0]).
                arg(SETTINGS_FILTER).
                arg(parts[1]);
}

void JsonDbHandle::unsubscribe()
{
    DEBUG_SIGNATURE

    if (connected) {
        DEBUG_MSG("DISCONNECT")

        if (watcher) {
            if (watcher->isActive())
                client->removeWatcher(watcher);

            delete watcher;
        }

        connected = false;
    }
}

QSet<QString> JsonDbHandle::children()
{
    DEBUG_SIGNATURE

    QString identifier = getWholePath(QStringLiteral(""));

    QList<QJsonObject> children = getResponse(
                getPartition(identifier),
                QString(QStringLiteral("[?identifier startsWith \"%1\"]%2[={id:identifier}]")).
                    arg(identifier).
                    arg(SETTINGS_FILTER));

    QSet<QString> result;

    QRegExp reg = QRegExp(QStringLiteral("^") + identifier.trimmed() + QStringLiteral("\\."));

    foreach (QJsonObject child, children) {
        if (child.contains(QStringLiteral("id")) && child[QStringLiteral("id")].isString()) {
            QStringList split = child.value(QStringLiteral("id")).toString().remove(reg).split(QLatin1Char('.'));

            if (split.count() > 0) {
                QString value = split[0];

                if (!result.contains(value)) {
                    result << value;
                }
            }
        }
    }

    return result;
}

bool JsonDbHandle::removeSubTree()
{
    DEBUG_SIGNATURE

    return false;
}






JsonDbLayer::JsonDbLayer()
{
    DEBUG_SIGNATURE
}

QSet<QString> JsonDbLayer::children(Handle handle)
{
    DEBUG_SIGNATURE

    return handleToJsonDbHandle(handle)->children();
}

QUuid JsonDbLayer::id()
{
    DEBUG_SIGNATURE

    return QVALUESPACE_JSONDB_LAYER;
}

QAbstractValueSpaceLayer::Handle JsonDbLayer::item(QAbstractValueSpaceLayer::Handle parent, const QString & subPath)
{
    DEBUG_SIGNATURE

    JsonDbHandle *parentHandle;
    if (parent != InvalidHandle) {
        parentHandle = handleToJsonDbHandle(parent);
    } else {
        parentHandle = NULL;
    }

    return reinterpret_cast<Handle>(new JsonDbHandle(parentHandle, subPath, this->layerOptions()));
}

QValueSpace::LayerOptions JsonDbLayer::layerOptions() const
{
    DEBUG_SIGNATURE

    return QValueSpace::WritableLayer | QValueSpace::PermanentLayer;
}

bool JsonDbLayer::notifyInterest(Handle handle, bool interested)
{
    DEBUG_SIGNATURE

    JsonDbHandle *h = handleToJsonDbHandle(handle);

    if (interested) {
        connect(h, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
        h->subscribe();
    }
    else {
        disconnect(h, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
        h->unsubscribe();
    }

    return true;
}

void JsonDbLayer::removeHandle(Handle handle)
{
    DEBUG_SIGNATURE

    delete handleToJsonDbHandle(handle);
}

bool JsonDbLayer::removeSubTree(QValueSpacePublisher*, Handle handle)
{
    DEBUG_SIGNATURE

    return handleToJsonDbHandle(handle)->removeSubTree();
}

// Virtual methods inherited from QAbstractValueSpaceLayer
void JsonDbLayer::addWatch(QValueSpacePublisher*, Handle)
{
    DEBUG_SIGNATURE
}

void JsonDbLayer::removeWatches(QValueSpacePublisher*, Handle)
{
    DEBUG_SIGNATURE
}

void JsonDbLayer::setProperty(Handle handle, Properties properties)
{
    DEBUG_SIGNATURE

    JsonDbHandle *h = handleToJsonDbHandle(handle);

    if (properties & Publish) {
        connect(h, SIGNAL(valueChanged()), this, SLOT(jsonDbHandleChanged()));
        h->subscribe();
    }
    else {
        disconnect(h, SIGNAL(valueChanged()), this, SLOT(jsonDbHandleChanged()));
        h->unsubscribe();
    }
}

bool JsonDbLayer::supportsInterestNotification() const
{
    DEBUG_SIGNATURE

    return true;
}

void JsonDbLayer::sync()
{
    DEBUG_SIGNATURE
}

bool JsonDbLayer::value(Handle handle, QVariant *data)
{
    DEBUG_SIGNATURE

    return handleToJsonDbHandle(handle)->value(QStringLiteral(""), data);
}

bool JsonDbLayer::value(Handle handle, const QString &subPath, QVariant *data)
{
    DEBUG_SIGNATURE

    JsonDbHandle *h = handleToJsonDbHandle(handle);

    if (subPath.startsWith(QStringLiteral("/"))) {
        return h->value(subPath.mid(1), data);
    } else {
        return h->value(subPath, data);
    }
}

bool JsonDbLayer::setValue(QValueSpacePublisher*, Handle handle, const QString &subPath, const QVariant &value)
{
    DEBUG_SIGNATURE

    JsonDbHandle *h = handleToJsonDbHandle(handle);

    if (subPath.startsWith(QStringLiteral("/"))) {
        return h->setValue(subPath.mid(1), value);
    } else {
        return h->setValue(subPath, value);
    }
}

bool JsonDbLayer::removeValue(QValueSpacePublisher*, Handle handle, const QString & subPath)
{
    DEBUG_SIGNATURE

    JsonDbHandle *h = handleToJsonDbHandle(handle);

    if (subPath.startsWith(QStringLiteral("/"))) {
        return h->unsetValue(subPath.mid(1));
    } else {
        return h->unsetValue(subPath);
    }
}

void JsonDbLayer::jsonDbHandleChanged()
{
    DEBUG_MSG("--- JsonDbLayer::jsonDbHandleChanged() ---")

    emit handleChanged(quintptr(sender()));
}



Q_GLOBAL_STATIC(JsonDbLayer, jsonDbLayer)



JsonDbLayer *JsonDbLayer::instance()
{
    DEBUG_SIGNATURE

    return jsonDbLayer();
}

QT_END_NAMESPACE

#endif // QT_NO_JSONDBLAYER
