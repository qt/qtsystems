/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include "jsondblayer_p.h"
#include <private/jsondb-connection_p.h>

#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

//#define DEBUG_ON

#ifdef DEBUG_ON
#include <qdebug.h>
#define DEBUG_MSG(msg) (qDebug()<<msg)
#else
#define DEBUG_MSG(msg)
#endif

JsonDbPath::JsonDbPath() : path(QStringLiteral(""))
{
}

JsonDbPath::JsonDbPath(QString path)
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
    if (path.length() == 0)
        return path;

    if (path == QStringLiteral("/"))
        return QStringLiteral("");

    QString result = path.trimmed().replace(QLatin1Char('/'), QLatin1Char('.'));

    if (result.startsWith(QLatin1Char('.')))
        result.remove(0, 1);

    if (result.endsWith(QLatin1Char('.')))
        result.remove(result.length() - 1, 1);

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




JsonDbHandle::JsonDbHandle( JsonDbHandle *parent,
                            const QString &root,
                            QValueSpace::LayerOptions) : QObject(parent), client(NULL)
{
    DEBUG_MSG("JsonDbHandle::JsonDbHandle(JsonDbHandle *parent, const QString &root, QValueSpace::LayerOptions opts)");

    if (parent != NULL)
        this->path = parent->path + root;
    else
        this->path = JsonDbPath(root);
}

JsonDbHandle::~JsonDbHandle()
{
    DEBUG_MSG("JsonDbHandle::~JsonDbHandle()");

    unsubscribe();

    delete client;
}

bool JsonDbHandle::value(const QString &path, QVariant *data)
{
    DEBUG_MSG("JsonDbHandle::value(const QString &path, QVariant *data)");

    QString wholePath = getWholePath(path);
    QStringList parts = JsonDbPath::getIdentifier(wholePath);

    QVariantMap objectMap;

    // Assume caller wants to get the value of a setting
    QString query = QString(QStringLiteral("[?identifier=\"%1\"]%2[={value:settings.%3}]")).
            arg(parts[0]).
            arg(SETTINGS_FILTER).
            arg(parts[1]);

    DEBUG_MSG("value() wholePath is: " + wholePath);
    DEBUG_MSG("value() part[0] is: " + parts[0]);
    DEBUG_MSG("value() part[1] is: " + parts[1]);
    DEBUG_MSG("value() path is: " + path);
    DEBUG_MSG("value() query is: " + query);

    objectMap = getResponse(query).toMap();

    if (!checkIfObjectValid(objectMap)) {
        // Assume the caller wants to get the whole settings object
        query = QString(QStringLiteral("[?identifier=\"%1\"]%2")).
                arg(wholePath).
                arg(SETTINGS_FILTER);

        objectMap = getResponse(query).toMap();

        if (!checkIfObjectValid(objectMap)) {
            *data = QVariant();

            return false;
        }

        *data = objectMap[QStringLiteral("data")].toList()[0];
    }
    else {
        *data = objectMap[QStringLiteral("data")].toList()[0].toMap()[QStringLiteral("value")];

        if (data->isNull())
            return false;
    }

    return true;
}

bool JsonDbHandle::setValue(const QString &path, const QVariant &data)
{
    // TODO: setting schema is not checked!

    DEBUG_MSG("JsonDbHandle::setValue(const QString &path, const QVariant &data)");

    QStringList parts = JsonDbPath::getIdentifier(getWholePath(path));

    QVariantMap updateObject = getObject(parts[0], IDENTIFIER);
    if (updateObject.empty()) {
        // Settings object does not exist
        return false;
    }

    if (!updateObject.contains(QStringLiteral("settings"))) {
        // Object does not contain settings dictionary
        return false;
    }

    QVariantMap settings = updateObject[QStringLiteral("settings")].toMap();
    if (!settings.contains(parts[1])) {
        // Settings object does not contain the setting
        return false;
    }

    settings[parts[1]] = data;
    updateObject[QStringLiteral("settings")] = settings;

    return !JsonDbConnection::instance()->sync(
            JsonDbConnection::makeUpdateRequest(updateObject)).toMap().isEmpty();
}

QVariantMap JsonDbHandle::getObject(const QString &identifier, const QString &property)
{
    DEBUG_MSG("JsonDbHandle::getObject(const QString &identifier)");

    QVariantMap map = getResponse(QString(QStringLiteral("[?%1=\"%2\"]%3")).
                                  arg(property).
                                  arg(identifier).
                                  arg(SETTINGS_FILTER)).toMap();

    if (!checkIfObjectValid(map))
        return QVariantMap();

    return map[QStringLiteral("data")].toList()[0].toMap();
}

QVariant JsonDbHandle::getResponse(const QString& query)
{
    DEBUG_MSG("JsonDbHandle::getResponse(const QString& query)");
    DEBUG_MSG("Query: " + query);

    return JsonDbConnection::instance()->sync(JsonDbConnection::makeQueryRequest(query));
}

QString JsonDbHandle::getWholePath(const QString &path) const
{
    DEBUG_MSG("getWholePath() this->path is: " + this->path.getPath());
    DEBUG_MSG("getWholePath() path is: " + path);

    if (path == QStringLiteral("/"))
        return this->path.getPath();

    QString identifier = this->path.getPath();

    if (identifier.length() == 0)
        identifier = path;
    else if (path.length() > 0)
        identifier += QLatin1String(".") + path;

    return JsonDbPath::normalizePath(identifier);
}

bool JsonDbHandle::unsetValue(const QString&)
{
    DEBUG_MSG("JsonDbHandle::unsetValue(const QString &path)");

    // Not supported
    return false;
}

void JsonDbHandle::subscribe()
{
    DEBUG_MSG("JsonDbHandle::subscribe()");
    DEBUG_MSG(this);

    if (client)
        return;

    QString query;

    unsubscribe();

    JsonDbClient::NotifyTypes actions;
    getNotificationQueryAndActions(getWholePath(QStringLiteral("")), query, actions);

    DEBUG_MSG("Notification query: " + query);

    //if (!client)
        client = new JsonDbClient(JsonDbConnection::instance(), this);

    DEBUG_MSG(client);

    connect(client,
            SIGNAL(notified(const JsonDbClient::NotifyTypes&, const QtAddOn::JsonDb::JsonDbNotification &)),
            this,
            SLOT(onNotified(const JsonDbClient::NotifyTypes&, const QtAddOn::JsonDb::JsonDbNotification &)));

    //int res;

    try {
        client->registerNotification(actions, query);

        DEBUG_MSG("NOTIFICATION " + query + " CREATED");
    } catch(...) { }
}

void JsonDbHandle::getNotificationQueryAndActions(QString path, QString& query, JsonDbClient::NotifyTypes& actions)
{
    QVariant object;

    actions = JsonDbClient::NotifyUpdate;

    if (!value(QStringLiteral(""), &object)){
        // Neither a settings object nor a setting in a settings object was found
        // Probably the caller wants to subscribe somewhere on the top in the namespace
        query = QString(QStringLiteral("[?identifier=~\"/%1.*/\"]%2")).
                    arg(path.replace(QStringLiteral("."), QStringLiteral("\\."))).
                    arg(SETTINGS_FILTER);

        actions |= JsonDbClient::NotifyCreate | JsonDbClient::NotifyRemove;
    } else {
        QVariantMap objectMap = object.toMap();

        if (objectMap.contains(QStringLiteral("_type"))) {
            // Path matches a settings object subscribe for that object
            query = QString(QStringLiteral("[?identifier=\"%1\"]%2")).
                        arg(path).
                        arg(SETTINGS_FILTER);

            actions |= JsonDbClient::NotifyCreate | JsonDbClient::NotifyRemove;
        } else {
            // Path matches a setting in a settings object subscribe for that setting
            QStringList parts = JsonDbPath::getIdentifier(path);
            query = QString(QStringLiteral("[?identifier=\"%1\"]%2[={value:settings.%3}]")).
                    arg(parts[0]).
                    arg(SETTINGS_FILTER).
                    arg(parts[1]);
        }
    }
}

void JsonDbHandle::unsubscribe()
{
    DEBUG_MSG("JsonDbHandle::unsubscribe()");

    /*if (!notificationUUID.isEmpty()) {
        //client->remove(QString("{\"_uuid\":\"%1\"}").arg(notificationUUID));
        JsonDbConnection::instance()->sync(JsonDbConnection::makeRemoveRequest(QString("{\"_uuid\":\"%1\"}").arg(notificationUUID)));
        notificationUUID = "";
    }*/

    if (client) {
        DEBUG_MSG("DISCONNECT");
        disconnect(client,
                   SIGNAL(notified(const JsonDbClient::NotifyTypes&, const QtAddOn::JsonDb::JsonDbNotification &)),
                   this,
                   SLOT(onNotified(const JsonDbClient::NotifyTypes&, const QtAddOn::JsonDb::JsonDbNotification &)));

        delete client;
        client = NULL;
    }
}

void JsonDbHandle::onNotified(const JsonDbClient::NotifyTypes&, const JsonDbNotification &)
{
    DEBUG_MSG("--- JsonDbHandle::notified(QString, QVariant, QString) ---");
    /*DEBUG_MSG(sender());
    DEBUG_MSG("First: " + first);
    DEBUG_MSG("Second:");
    QVariantMap map = second.toMap();//.value<QVariantMap>();
    foreach (QString key, map.keys()) {
        DEBUG_MSG("\t" + key + " : " + map[key].toString());
    }
    DEBUG_MSG("Third: " + third);*/

    emit valueChanged();
}

bool JsonDbHandle::checkIfObjectValid(const QVariantMap &object)
{
    return  object.contains(QStringLiteral("length")) &&
            (object[QStringLiteral("length")].toInt() == 1) &&
            object.contains(QStringLiteral("data"));
}

bool JsonDbHandle::checkIfObjectValidZero(const QVariantMap &object)
{
    return  object.contains(QStringLiteral("length")) &&
            (object[QStringLiteral("length")].toInt() > 0) &&
            object.contains(QStringLiteral("data"));
}

QSet<QString> JsonDbHandle::children()
{
    DEBUG_MSG("JsonDbHandle::children()");

    QString identifier = getWholePath(QStringLiteral("")).
            replace(QStringLiteral("."), QStringLiteral("\\."));

    QSet<QString> result;

    // [=identifier] because we only need identifier
    QVariantMap map = getResponse(QString(QStringLiteral("[?identifier=~\"/^%1.+/\"]%2[=identifier]")).
      arg(identifier.length() == 0 ? QStringLiteral("") : identifier.append(QStringLiteral("\\."))).
      arg(SETTINGS_FILTER)).toMap();

    if (!checkIfObjectValidZero(map))
        return result;

    QVariant object;
    QRegExp reg = QRegExp(QStringLiteral("^") + identifier.trimmed());

    foreach (object, map[QStringLiteral("data")].toList()) {
        // We only return the first part of each child subpath
        QString value = object.toString().remove(reg).split(QLatin1Char('.'))[0];
        if (!result.contains(value))
            result << value;
    }

    return result;
}

bool JsonDbHandle::removeSubTree()
{
    DEBUG_MSG("JsonDbLayer::removeSubTree()");

    return false;
}






JsonDbHandle* JsonDbLayer::handleToJsonDbHandle(Handle handle)
{
    DEBUG_MSG("JsonDbLayer::handleToJsonDbHandle(Handle handle)");

    if (handle != InvalidHandle)
        return reinterpret_cast<JsonDbHandle*>(handle);

    return NULL;
}

JsonDbLayer::JsonDbLayer()
{
    DEBUG_MSG("JsonDbLayer::JsonDbLayer()");
}

QSet<QString> JsonDbLayer::children(Handle handle)
{
    DEBUG_MSG("JsonDbLayer::children(Handle handle)");

    JsonDbHandle *h = handleToJsonDbHandle(handle);
    return h->children();
}

QUuid JsonDbLayer::id()
{
    DEBUG_MSG("JsonDbLayer::id()");

    return QVALUESPACE_JSONDB_LAYER;
}

QAbstractValueSpaceLayer::Handle JsonDbLayer::item(QAbstractValueSpaceLayer::Handle parent, const QString & subPath)
{
    DEBUG_MSG("JsonDbLayer::item(QAbstractValueSpaceLayer::Handle parent, const QString & subPath)");

    JsonDbHandle *parentHandle = handleToJsonDbHandle(parent);
    JsonDbHandle *h = new JsonDbHandle(parentHandle, subPath, this->layerOptions());

    return reinterpret_cast<Handle>(h);
}

QValueSpace::LayerOptions JsonDbLayer::layerOptions() const
{
    DEBUG_MSG("JsonDbLayer::layerOptions()");

    return QValueSpace::WritableLayer | QValueSpace::PermanentLayer;
}

QString JsonDbLayer::name()
{
    DEBUG_MSG("JsonDbLayer::name()");

    return QStringLiteral("JSON DB Layer");
}

bool JsonDbLayer::notifyInterest(Handle handle, bool interested)
{
    DEBUG_MSG("JsonDbLayer::notifyInterest(Handle handle, bool interested)");

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

unsigned int JsonDbLayer::order()
{
    DEBUG_MSG("JsonDbLayer::order()");

    return 0x1000;
}

void JsonDbLayer::removeHandle(Handle handle)
{
    DEBUG_MSG("JsonDbLayer::removeHandle(Handle handle)");

    JsonDbHandle *h = handleToJsonDbHandle(handle);

    if (h) {
        DEBUG_MSG("Delete handle");
        delete h;
    }
}

bool JsonDbLayer::removeSubTree(QValueSpacePublisher*, Handle handle)
{
    DEBUG_MSG("JsonDbLayer::removeSubTree(QValueSpacePublisher *creator, Handle handle)");

    JsonDbHandle *h = handleToJsonDbHandle(handle);
    if (!h) {
        return false;
    }

    return h->removeSubTree();
}

// Virtual methods inherited from QAbstractValueSpaceLayer
void JsonDbLayer::addWatch(QValueSpacePublisher*, Handle)
{
    DEBUG_MSG("JsonDbLayer::addWatch(QValueSpacePublisher *creator, Handle handle)");

    // Currently not implemented

    /*JsonDbHandle* handler = handleToJsonDbHandle(handle);
    connect(handler, SIGNAL(interestChanged(QString,bool)),
            creator, SIGNAL(interestChanged(QString,bool)));*/
}

void JsonDbLayer::removeWatches(QValueSpacePublisher*, Handle)
{
    DEBUG_MSG("JsonDbLayer::removeWatches(QValueSpacePublisher *creator, Handle parent)");

    // Currently not implemented

    /*JsonDbHandle* handler = handleToJsonDbHandle(parent);
    disconnect(handler, SIGNAL(interestChanged(QString,bool)),
               creator, SIGNAL(interestChanged(QString,bool)));*/
}

void JsonDbLayer::setProperty(Handle handle, Properties properties)
{
    DEBUG_MSG("JsonDbLayer::setProperty(Handle handle, Properties property)");

    JsonDbHandle *h = handleToJsonDbHandle(handle);
    if (!h) {
        return;
    }

    if (properties & Publish) {
        connect(h, SIGNAL(valueChanged()), this, SLOT(jsonDbHandleChanged()));
        h->subscribe();
    }
    else {
        disconnect(h, SIGNAL(valueChanged()), this, SLOT(jsonDbHandleChanged()));
        h->unsubscribe();
    }
}

bool JsonDbLayer::startup(Type)
{
    DEBUG_MSG("JsonDbLayer::startup(Type type)");

    return true;
}

bool JsonDbLayer::supportsInterestNotification() const
{
    DEBUG_MSG("JsonDbLayer::supportsInterestNotification()");

    return true;
}

void JsonDbLayer::sync()
{
    DEBUG_MSG("JsonDbLayer::sync()");
}

bool JsonDbLayer::value(Handle handle, QVariant *data)
{
    DEBUG_MSG("JsonDbLayer::value(Handle handle, QVariant *data)");

    JsonDbHandle *h = handleToJsonDbHandle(handle);
    if (h) {
        return h->value(QStringLiteral(""), data);
    }
    else {
        return false;
    }
}

bool JsonDbLayer::value(Handle handle, const QString &subPath, QVariant *data)
{
    DEBUG_MSG("JsonDbLayer::value(Handle handle, const QString &subPath, QVariant *data)");

    JsonDbHandle *h = handleToJsonDbHandle(handle);
    if (h) {
        if (subPath.startsWith(QStringLiteral("/"))) {
            QString tmp = subPath;
            return h->value(tmp.remove(0, 1), data);
        } else {
            return h->value(subPath, data);
        }
    } else {
        return false;
    }
}

bool JsonDbLayer::setValue(QValueSpacePublisher*, Handle handle, const QString &subPath, const QVariant &value)
{
    DEBUG_MSG("JsonDbLayer::setValue(QValueSpacePublisher *creator, Handle handle, const QString &subPath, const QVariant &value)");

    JsonDbHandle *h = handleToJsonDbHandle(handle);
    if (h) {
        if (subPath.startsWith(QStringLiteral("/"))) {
            QString tmp = subPath;
            return h->setValue(tmp.remove(0, 1), value);
        } else {
            return h->setValue(subPath, value);
        }
    } else {
        return false;
    }
}

bool JsonDbLayer::removeValue(QValueSpacePublisher*, Handle handle, const QString & subPath)
{
    DEBUG_MSG("JsonDbLayer::removeValue(QValueSpacePublisher *creator, Handle handle, const QString & subPath)");

    JsonDbHandle *h = handleToJsonDbHandle(handle);
        if (h) {
            if (subPath.startsWith(QStringLiteral("/"))) {
                QString tmp = subPath;
                return h->unsetValue(tmp.remove(0, 1));
            } else {
                return h->unsetValue(subPath);
            }
        } else {
            return false;
        }
}

void JsonDbLayer::jsonDbHandleChanged()
{
    DEBUG_MSG("--- JsonDbLayer::jsonDbHandleChanged() ---");

    emit handleChanged(quintptr(sender()));
}



QVALUESPACE_AUTO_INSTALL_LAYER(JsonDbLayer)
Q_GLOBAL_STATIC(JsonDbLayer, jsonDbLayer)



JsonDbLayer *JsonDbLayer::instance()
{
    DEBUG_MSG("JsonDbLayer::instance()");
    return jsonDbLayer();
}

QT_END_NAMESPACE
