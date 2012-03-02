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

#include <QtCore/qeventloop.h>
#include <QtCore/qtimer.h>
#include <QtJsonDb/qjsondbreadrequest.h>
#include <QtJsonDb/qjsondbwatcher.h>

QT_USE_NAMESPACE_JSONDB

QT_BEGIN_NAMESPACE

const int JSON_EXPIRATION_TIMER(2000);

QJsonDbWrapper::QJsonDbWrapper(QObject *parent)
    : QObject(parent)
    , jsonDbWatcher(0)
    , waitLoop(0)
    , timer(0)
    , watchActivatedLocks(false)
    , watchEnabledLocks(false)
{
    connect(&jsonDbConnection, SIGNAL(error(QtJsonDb::QJsonDbConnection::ErrorCode,QString)),
            this, SLOT(onJsonDbConnectionError(QtJsonDb::QJsonDbConnection::ErrorCode,QString)));

    jsonDbConnection.connectToServer();
}

QJsonDbWrapper::~QJsonDbWrapper()
{
}

QDeviceInfo::LockTypeFlags QJsonDbWrapper::getActivatedLocks()
{
    if (watchActivatedLocks)
        return activatedLocks;

    QDeviceInfo::LockTypeFlags activeLocks = QDeviceInfo::NoLock;

    if (getSystemPropertyValue(QString(QStringLiteral("SecurityLock")), QString(QStringLiteral("active"))).toBool())
        activeLocks |= QDeviceInfo::PinLock;

    return activeLocks;
}

QDeviceInfo::LockTypeFlags QJsonDbWrapper::getEnabledLocks()
{
    if (watchEnabledLocks)
        return enabledLocks;

    QDeviceInfo::LockTypeFlags enabledLocks = QDeviceInfo::NoLock;

    if (hasSystemObject(QString(QStringLiteral("SecurityLock"))))
        enabledLocks |= QDeviceInfo::PinLock;

    return enabledLocks;
}

bool QJsonDbWrapper::hasFeaturePositioning()
{
    return getSystemSettingValue(QString(QStringLiteral("location")),
                                 QString(QStringLiteral("locationServicesFeatureEnabled"))).toBool();
}

bool QJsonDbWrapper::hasFeatureVibration()
{
    return !getSystemSettingValue(QString(QStringLiteral("sounds")),
                                  QString(QStringLiteral("vibrationOn"))).isNull();
}

QString QJsonDbWrapper::getUniqueDeviceID()
{
    return getSystemPropertyValue(QString(QStringLiteral("DeviceInfo")),
                                  QString(QStringLiteral("uniqueDeviceId"))).toString();
}

bool QJsonDbWrapper::isVibrationActivated()
{
    return getSystemSettingValue(QString(QStringLiteral("sounds")),
                                 QString(QStringLiteral("vibrationOn"))).toBool();
}

int QJsonDbWrapper::getRingtoneVolume()
{
    int volume = getSystemSettingValue(QString(QStringLiteral("sounds")),
                                       QString(QStringLiteral("ringerVolume"))).toDouble();
    if (volume >= 0 && volume <= 100)
        return volume;

    return -1;
}

QJsonValue QJsonDbWrapper::getSystemPropertyValue(const QString &objectType, const QString &property)
{
    QJsonDbReadRequest request;
    request.setQuery(QString(QStringLiteral("[?_type=\"com.nokia.mt.system.%1\"]")).arg(objectType));
    connect(&request, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            this, SLOT(onJsonDbRequestError(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));
    connect(&request, SIGNAL(finished()), this, SLOT(onJsonDbRequestFinished()));
    if (jsonDbConnection.send(&request)) {
        waitForResponse();
        if (request.status() == QJsonDbRequest::Finished) {
            QList<QJsonObject> results = request.takeResults();
            if (results.size() > 0)
                return results.at(0).value(property);
        }
    }
    return QJsonValue();
}

QJsonValue QJsonDbWrapper::getSystemSettingValue(const QString &settingId, const QString &setting)
{
    QJsonDbReadRequest request;
    request.setQuery(QString(QStringLiteral("[?_type=\"com.nokia.mt.settings.SystemSettings\"][?identifier=\"com.nokia.mt.settings.%1\"]"))
                     .arg(settingId));
    connect(&request, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            this, SLOT(onJsonDbRequestError(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));
    connect(&request, SIGNAL(finished()), this, SLOT(onJsonDbRequestFinished()));
    if (jsonDbConnection.send(&request)) {
        waitForResponse();
        if (request.status() == QJsonDbRequest::Finished) {
            QList<QJsonObject> results = request.takeResults();
            if (results.size() > 0)
                return results.at(0).value(setting);
        }
    }
    return QJsonValue();
}

bool QJsonDbWrapper::hasSystemObject(const QString &objectType)
{
    QJsonDbReadRequest request;
    request.setQuery(QString(QStringLiteral("[?_type=\"com.nokia.mt.system.%1\"]")).arg(objectType));
    connect(&request, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            this, SLOT(onJsonDbRequestError(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));
    connect(&request, SIGNAL(finished()), this, SLOT(onJsonDbRequestFinished()));
    if (jsonDbConnection.send(&request)) {
        waitForResponse();
        return (request.status() == QJsonDbRequest::Finished && request.takeResults().size() > 0);
    }
    return false;
}

void QJsonDbWrapper::connectNotify(const char *signal)
{
    if (watchActivatedLocks && watchEnabledLocks)
        return;

    bool needWatchActivatedLocks = (strcmp(signal, SIGNAL(activatedLocksChanged(QDeviceInfo::LockTypeFlags))) == 0);
    if (needWatchActivatedLocks) {
        if (watchActivatedLocks)
            return;
        activatedLocks = getActivatedLocks();
    }

    bool needWatchEnabledLocks = (strcmp(signal, SIGNAL(enabledLocksChanged(QDeviceInfo::LockTypeFlags))) == 0);
    if (needWatchEnabledLocks) {
        if (watchEnabledLocks)
            return;
        enabledLocks = getEnabledLocks();
    }

    if (needWatchActivatedLocks || needWatchEnabledLocks) {
        if (!jsonDbWatcher) {
            jsonDbWatcher = new QJsonDbWatcher(this);
            jsonDbWatcher->setWatchedActions(QJsonDbWatcher::All);
            jsonDbWatcher->setQuery(QString(QStringLiteral("[_type=\"com.nokia.mt.system.SecurityLock\"]")));
            connect(jsonDbWatcher, SIGNAL(notificationsAvailable(int)),
                    this, SLOT(onJsonDbWatcherNotificationsAvailable()));
            // TODO: error handling for watcher
        }
        jsonDbConnection.addWatcher(jsonDbWatcher);
    }
}

void QJsonDbWrapper::disconnectNotify(const char *signal)
{
    if (!watchActivatedLocks && !watchEnabledLocks)
        return;

    if (strcmp(signal, SIGNAL(activatedLocksChanged(QDeviceInfo::LockTypeFlags))) == 0)
        watchActivatedLocks = false;
    else if (strcmp(signal, SIGNAL(enabledLocksChanged(QDeviceInfo::LockTypeFlags))) == 0)
        watchEnabledLocks = false;

    if (!watchActivatedLocks && !watchEnabledLocks)
        jsonDbConnection.removeWatcher(jsonDbWatcher);
}

void QJsonDbWrapper::onJsonDbWatcherNotificationsAvailable()
{
    if (!watchActivatedLocks && !watchEnabledLocks)
        return;

    QList<QJsonDbNotification> notifications = jsonDbWatcher->takeNotifications();
    if (notifications.size() > 0) {
        const QJsonDbNotification notification = notifications.at(0);
        if (watchActivatedLocks) {
            if (notification.action() == QJsonDbWatcher::Removed) {
                if (watchActivatedLocks && activatedLocks.testFlag(QDeviceInfo::PinLock)) {
                    activatedLocks &= ~QDeviceInfo::PinLock;
                    emit activatedLocksChanged(activatedLocks);
                }
            } else {
                if (notification.object().value(QString(QStringLiteral("active"))).toBool()) {
                    if (!activatedLocks.testFlag(QDeviceInfo::PinLock)) {
                        activatedLocks |= QDeviceInfo::PinLock;
                        emit activatedLocksChanged(activatedLocks);
                    }
                } else {
                    if (activatedLocks.testFlag(QDeviceInfo::PinLock)) {
                        activatedLocks &= ~QDeviceInfo::PinLock;
                        emit activatedLocksChanged(activatedLocks);
                    }
                }
            }
        }
        if (watchEnabledLocks) {
            if (notification.action() == QJsonDbWatcher::Created) {
                enabledLocks |= QDeviceInfo::PinLock;
                emit enabledLocksChanged(enabledLocks);
            } else if (notification.action() == QJsonDbWatcher::Removed) {
                enabledLocks &= !QDeviceInfo::PinLock;
                emit enabledLocksChanged(enabledLocks);
            }
        }
    }
}

void QJsonDbWrapper::onJsonDbConnectionError(QtJsonDb::QJsonDbConnection::ErrorCode error, const QString &message)
{
    Q_UNUSED(error)
    Q_UNUSED(message)

    timer->stop();
    waitLoop->exit(0);
}

void QJsonDbWrapper::onJsonDbRequestError(QtJsonDb::QJsonDbRequest::ErrorCode error, const QString &message)
{
    Q_UNUSED(error)
    Q_UNUSED(message)

    timer->stop();
    waitLoop->exit(0);
}

void QJsonDbWrapper::onJsonDbRequestFinished()
{
    timer->stop();
    waitLoop->exit(0);
}

bool QJsonDbWrapper::waitForResponse()
{
    if (!waitLoop)
        waitLoop = new QEventLoop(this);
    if (!timer) {
        timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, SIGNAL(timeout()), waitLoop, SLOT(quit()));
    }
    timer->start(JSON_EXPIRATION_TIMER);
    waitLoop->exec(QEventLoop::AllEvents);

    return true;
}

QT_END_NAMESPACE
