/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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

#define handleToJsonDbHandle(handle) reinterpret_cast<JsonDbHandle*>(handle)

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




JsonDbHandle::JsonDbHandle( JsonDbHandle *parent,
                            const QString &root,
                            QValueSpace::LayerOptions) : QObject(parent), client(NULL)
{
    DEBUG_MSG("JsonDbHandle::JsonDbHandle(JsonDbHandle *parent, const QString &root, QValueSpace::LayerOptions opts)");
    DEBUG_MSG(this);

    if (parent != NULL)
        this->path = parent->path + root;
    else
        this->path = JsonDbPath(root);

    notificationUUID = QString();
}

JsonDbHandle::~JsonDbHandle()
{
    DEBUG_MSG("JsonDbHandle::~JsonDbHandle()");
    DEBUG_MSG(this);

    unsubscribe();
}

bool JsonDbHandle::value(const QString &path, QVariant *data)
{
    DEBUG_MSG("JsonDbHandle::value(const QString &path, QVariant *data)");
    DEBUG_MSG(this);

    QString wholePath = getWholePath(path);

    // Assume caller wants to get the value of a setting
    QVariantMap objectMap = getResponse(getSettingQuery(wholePath)).toMap();

    if (!checkIfObjectValid(objectMap)) {
        // Assume the caller wants to get the whole settings object
        objectMap = getResponse(getObjectQuery(wholePath)).toMap();

        if (!checkIfObjectValid(objectMap)) {
            return false;
        }
    }

    *data = objectMap[QStringLiteral("data")].toList()[0];

    return !data->isNull();
}

bool JsonDbHandle::setValue(const QString &path, const QVariant &data)
{
    DEBUG_MSG("JsonDbHandle::setValue(const QString &path, const QVariant &data)");
    DEBUG_MSG(this);

    QStringList parts = JsonDbPath::getIdentifier(getWholePath(path));

    QVariantMap updateObject = getObject(parts[0], IDENTIFIER);

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
    DEBUG_MSG(this);
    DEBUG_MSG("getWholePath() path is: " + path);

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
    DEBUG_MSG(this);

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
    JsonDbClient::NotifyTypes actions;
    getNotificationQueryAndActions(getWholePath(QStringLiteral("")), query, actions);

    DEBUG_MSG("Notification query: " + query);

    client = new JsonDbClient();

    notificationUUID = client->registerNotification(actions,
                                                    query,
                                                    QString(),
                                                    this,
                                                    SLOT(onNotified(const QString, const JsonDbNotification)),
                                                    0,
                                                    0,
                                                    SLOT(onError(int,int,QString)));
}

void JsonDbHandle::onError(int id, int code, const QString & message)
{
    DEBUG_MSG(this);
    qDebug()<<"Error:";
    qDebug()<<"\tid: "<<id;
    qDebug()<<"\tcode: "<<code;
    qDebug()<<"\tmessage: "<<message;
}

void JsonDbHandle::getNotificationQueryAndActions(QString path, QString& query, JsonDbClient::NotifyTypes& actions)
{
    DEBUG_MSG(this);

    QVariant object;

    actions = JsonDbClient::NotifyUpdate;

    if (!value(QStringLiteral(""), &object)) {
        // Neither a settings object nor a setting in a settings object was found
        // Probably the caller wants to subscribe somewhere on the top in the namespace
        query = QString(QStringLiteral("[?identifier=~\"/%1.*/\"]%2")).
                    arg(path.replace(QStringLiteral("."), QStringLiteral("\\."))).
                    arg(SETTINGS_FILTER);

        actions |= JsonDbClient::NotifyCreate | JsonDbClient::NotifyRemove;
    } else {
        if (object.toMap().contains(QStringLiteral("_type"))) {
            // Path matches a settings object subscribe for that object
            query = getObjectQuery(path);

            actions |= JsonDbClient::NotifyCreate | JsonDbClient::NotifyRemove;
        } else {
            // Path matches a setting in a settings object subscribe for that setting
            query = getSettingQuery(path);
        }
    }
}

QString JsonDbHandle::getObjectQuery(QString &identifier)
{
    return QString(QStringLiteral("[?identifier=\"%1\"]%2")).
            arg(identifier).
            arg(SETTINGS_FILTER);
}

QString JsonDbHandle::getSettingQuery(QString &identifier)
{
    QStringList parts = JsonDbPath::getIdentifier(identifier);
    return QString(QStringLiteral("[?identifier=\"%1\"]%2[=settings.%3]")).
                arg(parts[0]).
                arg(SETTINGS_FILTER).
                arg(parts[1]);
}

void JsonDbHandle::unsubscribe()
{
    DEBUG_MSG("JsonDbHandle::unsubscribe()");
    DEBUG_MSG(this);

    if (client) {
        DEBUG_MSG("DISCONNECT");

        if (!notificationUUID.isEmpty()) {
            DEBUG_MSG("Unregister notification");
            client->unregisterNotification(notificationUUID);
        }

        delete client;
        client = NULL;
    }
}

void JsonDbHandle::onNotified(const QString&, const JsonDbNotification&)
{
    DEBUG_MSG("!--- onNotified(const QString, const JsonDbNotification) ---!");
    DEBUG_MSG(this);

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
    DEBUG_MSG(this);

    QString identifier = getWholePath(QStringLiteral("")).
            replace(QStringLiteral("."), QStringLiteral("\\."));

    // [=identifier] because we only need identifier
    QVariantMap map = getResponse(QString(QStringLiteral("[?identifier=~\"/^%1.+/\"]%2[=identifier]")).
      arg(identifier.length() == 0 ? QStringLiteral("") : identifier.append(QStringLiteral("\\."))).
      arg(SETTINGS_FILTER)).toMap();

    QSet<QString> result;

    if (!checkIfObjectValidZero(map))
        return result;

    QRegExp reg = QRegExp(QStringLiteral("^") + identifier.trimmed());

    foreach (QVariant object, map[QStringLiteral("data")].toList()) {
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
    DEBUG_MSG(this);

    return false;
}






JsonDbLayer::JsonDbLayer()
{
    DEBUG_MSG("JsonDbLayer::JsonDbLayer()");
}

QSet<QString> JsonDbLayer::children(Handle handle)
{
    DEBUG_MSG("JsonDbLayer::children(Handle handle)");

    return handleToJsonDbHandle(handle)->children();
}

QUuid JsonDbLayer::id()
{
    DEBUG_MSG("JsonDbLayer::id()");

    return QVALUESPACE_JSONDB_LAYER;
}

QAbstractValueSpaceLayer::Handle JsonDbLayer::item(QAbstractValueSpaceLayer::Handle parent, const QString & subPath)
{
    DEBUG_MSG("JsonDbLayer::item(QAbstractValueSpaceLayer::Handle parent, const QString & subPath)");

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

    DEBUG_MSG("Delete handle");
    delete handleToJsonDbHandle(handle);
}

bool JsonDbLayer::removeSubTree(QValueSpacePublisher*, Handle handle)
{
    DEBUG_MSG("JsonDbLayer::removeSubTree(QValueSpacePublisher *creator, Handle handle)");

    return handleToJsonDbHandle(handle)->removeSubTree();
}

// Virtual methods inherited from QAbstractValueSpaceLayer
void JsonDbLayer::addWatch(QValueSpacePublisher*, Handle)
{
    DEBUG_MSG("JsonDbLayer::addWatch(QValueSpacePublisher *creator, Handle handle)");
}

void JsonDbLayer::removeWatches(QValueSpacePublisher*, Handle)
{
    DEBUG_MSG("JsonDbLayer::removeWatches(QValueSpacePublisher *creator, Handle parent)");
}

void JsonDbLayer::setProperty(Handle handle, Properties properties)
{
    DEBUG_MSG("JsonDbLayer::setProperty(Handle handle, Properties property)");

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

    return handleToJsonDbHandle(handle)->value(QStringLiteral(""), data);
}

bool JsonDbLayer::value(Handle handle, const QString &subPath, QVariant *data)
{
    DEBUG_MSG("JsonDbLayer::value(Handle handle, const QString &subPath, QVariant *data)");

    JsonDbHandle *h = handleToJsonDbHandle(handle);

    if (subPath.startsWith(QStringLiteral("/"))) {
        return h->value(subPath.mid(1), data);
    } else {
        return h->value(subPath, data);
    }
}

bool JsonDbLayer::setValue(QValueSpacePublisher*, Handle handle, const QString &subPath, const QVariant &value)
{
    DEBUG_MSG("JsonDbLayer::setValue(QValueSpacePublisher *creator, Handle handle, const QString &subPath, const QVariant &value)");

    JsonDbHandle *h = handleToJsonDbHandle(handle);

    if (subPath.startsWith(QStringLiteral("/"))) {
        return h->setValue(subPath.mid(1), value);
    } else {
        return h->setValue(subPath, value);
    }
}

bool JsonDbLayer::removeValue(QValueSpacePublisher*, Handle handle, const QString & subPath)
{
    DEBUG_MSG("JsonDbLayer::removeValue(QValueSpacePublisher *creator, Handle handle, const QString & subPath)");

    JsonDbHandle *h = handleToJsonDbHandle(handle);

    if (subPath.startsWith(QStringLiteral("/"))) {
        return h->unsetValue(subPath.mid(1));
    } else {
        return h->unsetValue(subPath);
    }
}

void JsonDbLayer::jsonDbHandleChanged()
{
    DEBUG_MSG("--- JsonDbLayer::jsonDbHandleChanged() ---");

    emit handleChanged(quintptr(sender()));
}



Q_GLOBAL_STATIC(JsonDbLayer, jsonDbLayer)
QVALUESPACE_AUTO_INSTALL_LAYER(JsonDbLayer)



JsonDbLayer *JsonDbLayer::instance()
{
    DEBUG_MSG("JsonDbLayer::instance()");
    return jsonDbLayer();
}

QT_END_NAMESPACE
