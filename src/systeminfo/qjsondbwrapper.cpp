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

#include "qjsondbwrapper_p.h"

#include <QTimer>
#include <QEventLoop>

#include <jsondb-client.h>

Q_USE_JSONDB_NAMESPACE

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(const QString, SETTING_PATH, (QStringLiteral("com.nokia.mt.settings.")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SETTING_LOCATION, (QStringLiteral("location")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SETTING_SOUNDS, (QStringLiteral("sounds")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SYSTEM_PATH, (QStringLiteral("com.nokia.mt.system.")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SYSTEM_SECURITYLOCK, (QStringLiteral("SecurityLock")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SYSTEM_DEVICEINFO, (QStringLiteral("DeviceInfo")))

Q_GLOBAL_STATIC_WITH_ARGS(const QString, SYSTEM_OBJECT_QUERY, (QStringLiteral("[?_type=\"%1\"]")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SYSTEM_OBJECT_NOTIFY, (QStringLiteral("[_type=\"%1\"]")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SYSTEM_OBJECT_REMOVE, (QStringLiteral("{\"_uuid\":\"%1\"}")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SYSTEM_PROPERTY_QUERY, (QStringLiteral("[?_type=\"%1\"][=%2]")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SETTING_QUERY, (QStringLiteral("[?_type=\"com.nokia.mt.settings.SystemSettings\"][?identifier=\"%1\"][=settings]")))

const int JSON_EXPIRATION_TIMER(2000);

QJsonDbWrapper::QJsonDbWrapper(QObject *parent)
    : QObject(parent)
    , jsonclient(new JsonDbClient(this))
    , waitLoop(0)
    , timer(0)
    , watchActivatedLocks(false)
    , watchEnabledLocks(false)
{
    connect(jsonclient, SIGNAL(error(int, int, QString)),
            this, SLOT(onError(int, int, QString)));
    connect(jsonclient,SIGNAL(response(int, QVariant)),
            this,SLOT(onResponse(int, QVariant)));
}

QJsonDbWrapper::~QJsonDbWrapper()
{
}

QDeviceInfo::LockTypeFlags QJsonDbWrapper::getActivatedLocks()
{
    if (watchActivatedLocks)
        return activatedLocks;

    QDeviceInfo::LockTypeFlags activeLocks = QDeviceInfo::NoLock;

    if (getSystemPropertyValue(*SYSTEM_SECURITYLOCK(), QStringLiteral("active")).toBool())
        activeLocks |= QDeviceInfo::PinLock;

    return activeLocks;
}

QDeviceInfo::LockTypeFlags QJsonDbWrapper::getEnabledLocks()
{
    if (watchEnabledLocks)
        return enabledLocks;

    QDeviceInfo::LockTypeFlags enabledLocks = QDeviceInfo::NoLock;

    if (hasSystemObject(*SYSTEM_SECURITYLOCK()))
        enabledLocks |= QDeviceInfo::PinLock;

    return enabledLocks;
}

bool QJsonDbWrapper::hasFeaturePositioning()
{
    return getSystemSettingValue(*SETTING_LOCATION(), QStringLiteral("locationServicesFeatureEnabled")).toBool();
}

bool QJsonDbWrapper::hasFeatureVibration()
{
    QVariant vibration = getSystemSettingValue(*SETTING_SOUNDS(), QStringLiteral("vibrationOn"));
    if (!vibration.isNull())
        return true;

    return false;
}

QString QJsonDbWrapper::getUniqueDeviceID()
{
    return getSystemPropertyValue(*SYSTEM_DEVICEINFO(), QStringLiteral("uniqueDeviceId")).toString();
}

bool QJsonDbWrapper::isVibrationActivated()
{
    return getSystemSettingValue(*SETTING_SOUNDS(), QStringLiteral("vibrationOn")).toBool();
}

int QJsonDbWrapper::getRingtoneVolume()
{
    int volume = getSystemSettingValue(*SETTING_SOUNDS(), QStringLiteral("ringerVolume")).toInt();
    if (volume >= 0 && volume <= 100)
        return volume;

    return -1;
}

QVariant QJsonDbWrapper::getSystemPropertyValue(const QString &objectType, const QString &property)
{
    int reqId = 0;
    QVariant response;

    if (!jsonclient->isConnected())
        return response;

    reqId = jsonclient->query(SYSTEM_PROPERTY_QUERY()->arg(*SYSTEM_PATH() + objectType).arg(property));
    if (waitForResponse()) {
        QVariant data = responses.take(reqId).toMap().value(QStringLiteral("data"));
        QVariantList tempList(data.toList());
        if (tempList.size() > 0)
            response = tempList.at(0);
    }

    return response;
}

QVariant QJsonDbWrapper::getSystemSettingValue(const QString &settingId, const QString &setting)
{
    int reqId = 0;
    QVariant response;

    if (!jsonclient->isConnected())
        return response;

    reqId = jsonclient->query(SETTING_QUERY()->arg(*SETTING_PATH() + settingId));
    if (waitForResponse()) {
        QVariant data = responses.take(reqId).toMap().value(QStringLiteral("data"));
        QVariantList tempList(data.toList());
        if (tempList.size() > 0)
            response = tempList.at(0).toMap().value(setting);
    }

    return response;
}

bool QJsonDbWrapper::hasSystemObject(const QString &objectType)
{
    int reqId = 0;

    if (!jsonclient->isConnected())
        return false;

    reqId = jsonclient->query(SYSTEM_OBJECT_QUERY()->arg(*SYSTEM_PATH() + objectType));
    if (waitForResponse()) {
        QVariant data = responses.take(reqId).toMap().value(QStringLiteral("data"));
        QVariantList tempList(data.toList());
        if (tempList.size() > 0)
            return true;
    }

    return false;
}

QString QJsonDbWrapper::registerOnChanges(const QString &objectType)
{
    int reqId = 0;
    QString uuid;

    if (!jsonclient->isConnected())
        return uuid;

    reqId = jsonclient->notify(JsonDbClient::NotifyUpdate | JsonDbClient::NotifyCreate | JsonDbClient::NotifyRemove,
                               SYSTEM_OBJECT_NOTIFY()->arg(*SYSTEM_PATH() + objectType));
    if (waitForResponse())
        uuid = responses.take(reqId).toMap().value(QStringLiteral("_uuid")).toString().remove(QRegExp(QStringLiteral("[{}]")));

    return uuid;
}

bool QJsonDbWrapper::unregisterOnChanges(const QString &uuid)
{
    int reqId = 0;

    if (!jsonclient->isConnected())
        return false;

    reqId = jsonclient->remove(SYSTEM_OBJECT_REMOVE()->arg(uuid));
    if (waitForResponse()) {
        QVariant data = responses.take(reqId).toMap().value(QStringLiteral("data"));
        if (data.toList().count() > 0)
            return true;
    }

    return false;
}

void QJsonDbWrapper::connectNotify(const char *signal)
{
    if ((strcmp(signal, SIGNAL(activatedLocksChanged(QDeviceInfo::LockTypeFlags))) == 0
         || strcmp(signal, SIGNAL(enabledLocksChanged(QDeviceInfo::LockTypeFlags))) == 0)
            && (!watchActivatedLocks) && (!watchEnabledLocks)) {
        connect(jsonclient,SIGNAL(notified(QString,QVariant,QString)),
                this,SLOT(onNotification(QString,QVariant,QString)),
                Qt::UniqueConnection);
    }

    if (strcmp(signal, SIGNAL(activatedLocksChanged(QDeviceInfo::LockTypeFlags))) == 0) {
        activatedLocks = getActivatedLocks();
        if (uuidSecurityLockNotifier.isEmpty())
            uuidSecurityLockNotifier = registerOnChanges(*SYSTEM_SECURITYLOCK());
        if (!uuidSecurityLockNotifier.isEmpty())
            watchActivatedLocks = true;
    } else if (strcmp(signal, SIGNAL(enabledLocksChanged(QDeviceInfo::LockTypeFlags))) == 0) {
        enabledLocks = getEnabledLocks();
        if (uuidSecurityLockNotifier.isEmpty())
            uuidSecurityLockNotifier = registerOnChanges(*SYSTEM_SECURITYLOCK());
        if (!uuidSecurityLockNotifier.isEmpty())
            watchEnabledLocks = true;
    }
}

void QJsonDbWrapper::disconnectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(activatedLocksChanged(QDeviceInfo::LockTypeFlags))) == 0)
        watchActivatedLocks = false;
    else if (strcmp(signal, SIGNAL(enabledLocksChanged(QDeviceInfo::LockTypeFlags))) == 0)
        watchEnabledLocks = false;

    if ((strcmp(signal, SIGNAL(activatedLocksChanged(QDeviceInfo::LockTypeFlags))) == 0
         || strcmp(signal, SIGNAL(enabledLocksChanged(QDeviceInfo::LockTypeFlags))) == 0)
            && (!watchActivatedLocks)
            && (!watchEnabledLocks)) {
        if (unregisterOnChanges(uuidSecurityLockNotifier))
            uuidSecurityLockNotifier.clear();
        disconnect(jsonclient,SIGNAL(notified(QString,QVariant,QString)),
                   this,SLOT(onNotification(QString,QVariant,QString)));
    }
}

void QJsonDbWrapper::onNotification(const QString &uuid,
                                    const QVariant &notification,
                                    const QString &action)
{
    Q_UNUSED(uuid)

    QVariantMap data = notification.toMap();
    QString objectType = data.value(QStringLiteral("_type")).toString();

    if (objectType.compare(*SYSTEM_PATH() + *SYSTEM_SECURITYLOCK()) == 0) {
        if (watchActivatedLocks) {
            if (action.compare(QStringLiteral("remove")) == 0
                    && (activatedLocks & QDeviceInfo::PinLock) == QDeviceInfo::PinLock) {
                activatedLocks &= !QDeviceInfo::PinLock;
            } else if (data.value(QStringLiteral("active")).toBool()) {
                if (!(activatedLocks & QDeviceInfo::PinLock) == QDeviceInfo::PinLock) {
                    activatedLocks |= QDeviceInfo::PinLock;
                    emit activatedLocksChanged(activatedLocks);
                }
            } else if (!data.value(QStringLiteral("active")).toBool()) {
                if ((activatedLocks & QDeviceInfo::PinLock) == QDeviceInfo::PinLock) {
                    activatedLocks &= !QDeviceInfo::PinLock;
                    emit activatedLocksChanged(activatedLocks);
                }
            }
        }
        if (watchEnabledLocks && action.compare(QStringLiteral("update")) != 0) {
            if (action == QStringLiteral("create")) {
                if (!(enabledLocks & QDeviceInfo::PinLock) == QDeviceInfo::PinLock) {
                    enabledLocks |= QDeviceInfo::PinLock;
                    emit enabledLocksChanged(enabledLocks);
                }
            } else if (action == QStringLiteral("remove")) {
                if ((enabledLocks & QDeviceInfo::PinLock) == QDeviceInfo::PinLock) {
                    enabledLocks &= !QDeviceInfo::PinLock;
                    emit enabledLocksChanged(enabledLocks);
                }
            }
        }
    }
}

void QJsonDbWrapper::onResponse(int reqId, const QVariant &response)
{
    timer->stop();
    waitLoop->exit(0);
    responses.insert(reqId, response);
}

void QJsonDbWrapper::onError(int reqId, int code, const QString &message)
{
    Q_UNUSED (reqId)
    Q_UNUSED (code)
    Q_UNUSED (message)

    timer->stop();
    waitLoop->exit(0);
}

void QJsonDbWrapper::onTimerExpired()
{
    timer->stop();
    waitLoop->exit(0);
}

bool QJsonDbWrapper::waitForResponse()
{
    if (!jsonclient->isConnected())
        return false;
    if (!waitLoop)
        waitLoop = new QEventLoop (this);
    if (!timer)
        timer = new QTimer (this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimerExpired()));
    timer->start(JSON_EXPIRATION_TIMER);
    waitLoop->exec(QEventLoop::AllEvents);

    return true;
}

QT_END_NAMESPACE
