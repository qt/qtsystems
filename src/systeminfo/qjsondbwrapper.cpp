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
    , locksWatcher(0)
    , backlightWatcher(0)
    , waitLoop(0)
    , timer(0)
    , watchActivatedLocks(false)
    , watchEnabledLocks(false)
    , watchBacklightState(false)
    , activatedLocks(QDeviceInfo::UnknownLock)
    , enabledLocks(QDeviceInfo::UnknownLock)
    , isLockTypeRequested(false)
{
    connect(&jsonDbConnection, SIGNAL(error(QtJsonDb::QJsonDbConnection::ErrorCode,QString)),
            this, SLOT(onJsonDbConnectionError(QtJsonDb::QJsonDbConnection::ErrorCode,QString)));

    jsonDbConnection.connectToServer();
}

QJsonDbWrapper::~QJsonDbWrapper()
{
}

QDeviceInfo::LockTypeFlags QJsonDbWrapper::activatedLockTypes()
{
    if (!isLockTypeRequested && activatedLocks == QDeviceInfo::UnknownLock) {
       isLockTypeRequested = true;
       sendJsonDbLockObjectsReadRequest(QStringLiteral("Ephemeral"));
    }
    return activatedLocks;
}

QDeviceInfo::LockTypeFlags QJsonDbWrapper::enabledLockTypes()
{
    if (!isLockTypeRequested && enabledLocks == QDeviceInfo::UnknownLock) {
       isLockTypeRequested = true;
       sendJsonDbLockObjectsReadRequest(QStringLiteral("Ephemeral"));
    }

    return enabledLocks;
}

bool QJsonDbWrapper::hasFeaturePositioning()
{
    return getSystemSettingValue(QString(QStringLiteral("location")),
                                 QString(QStringLiteral("locationServicesFeatureEnabled"))).toBool();
}

QString QJsonDbWrapper::model()
{
    return getSystemSettingValue(QString(QStringLiteral("sw_variant_configuration")),
                                 QString(QStringLiteral("productName"))).toString();
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

QJsonValue QJsonDbWrapper::getSystemSettingValue(const QString &settingId, const QString &setting, const QString &partition)
{
    QJsonDbReadRequest request;
    if (!partition.isEmpty())
       request.setPartition(partition);
    request.setQuery(QString(QStringLiteral("[?_type=\"com.nokia.mt.settings.SystemSettings\"][?identifier=\"com.nokia.mt.settings.%1\"][={settings:settings}]"))
                     .arg(settingId));
    connect(&request, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            this, SLOT(onJsonDbRequestError(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));
    connect(&request, SIGNAL(finished()), this, SLOT(onJsonDbRequestFinished()));
    if (jsonDbConnection.send(&request)) {
        waitForResponse();
        if (request.status() == QJsonDbRequest::Finished) {
            QList<QJsonObject> results = request.takeResults();
            if (results.size() > 0)
                return results.at(0).value(QStringLiteral("settings")).toObject().value(setting);
        }
    }
    return QJsonValue();
}

bool QJsonDbWrapper::hasSystemObject(const QString &objectType, const QString &partition)
{
    QJsonDbReadRequest request;
    if (!partition.isEmpty())
        request.setPartition(partition);
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

void QJsonDbWrapper::sendJsonDbLockObjectsReadRequest(const QString &partition)
{
    QJsonDbReadRequest *request = new QtJsonDb::QJsonDbReadRequest();
    if (!partition.isEmpty())
       request->setPartition(partition);

    request->setQuery(QString(QStringLiteral("[?_type in [\"com.nokia.mt.system.SecurityLock\",\"com.nokia.mt.system.Lockscreen\"]]")));

    connect(request, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            this, SLOT(onJsonDbReadRequestError(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));
    connect(request, SIGNAL(finished()), this, SLOT(onJsonDbLockObjectsReadRequestFinished()));
    jsonDbConnection.send(request);
}

void QJsonDbWrapper::connectNotify(const char *signal)
{
    if (watchActivatedLocks && watchEnabledLocks && watchBacklightState)
        return;

    bool needWatchActivatedLocks = (strcmp(signal, SIGNAL(activatedLocksChanged(QDeviceInfo::LockTypeFlags))) == 0);
    if (needWatchActivatedLocks) {
        if (watchActivatedLocks)
            return;
        activatedLocks = activatedLockTypes();
    }

    bool needWatchEnabledLocks = (strcmp(signal, SIGNAL(enabledLocksChanged(QDeviceInfo::LockTypeFlags))) == 0);
    if (needWatchEnabledLocks) {
        if (watchEnabledLocks)
            return;
        enabledLocks = enabledLockTypes();
    }

    if (needWatchActivatedLocks)
        watchActivatedLocks = true;

    if (needWatchEnabledLocks)
        watchEnabledLocks = true;

    if (strcmp(signal, SIGNAL(backlightStateChanged(int,QDisplayInfo::BacklightState))) == 0) {
        if (!backlightWatcher) {
            backlightWatcher = new QtJsonDb::QJsonDbWatcher(this);
            backlightWatcher->setPartition(QStringLiteral("Ephemeral"));
            backlightWatcher->setWatchedActions(QJsonDbWatcher::Updated);
            backlightWatcher->setQuery(QString(QStringLiteral("[?_type=\"com.nokia.mt.system.DisplayState\"]")));
            connect(backlightWatcher, SIGNAL(notificationsAvailable(int)), this, SLOT(onJsonDbWatcherNotificationsBacklightStateAvailable()));
        }
        jsonDbConnection.addWatcher(backlightWatcher);
        watchBacklightState = true;
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
    else if (strcmp(signal, SIGNAL(backlightStateChanged(int, QDisplayInfo::BacklightState))) == 0)
        watchBacklightState = false;

    if (!watchBacklightState)
        jsonDbConnection.removeWatcher(backlightWatcher);
}

void QJsonDbWrapper::onJsonDbWatcherNotificationsAvailable()
{
    if (!watchActivatedLocks && !watchEnabledLocks)
        return;

    QList<QJsonDbNotification> notifications = locksWatcher->takeNotifications();
    if (notifications.size() > 0) {
        const QJsonDbNotification notification = notifications.at(0);
        const QByteArray objectType = notification.object().value(QStringLiteral("_type")).toString().toAscii();
        if (watchActivatedLocks) {
            if (notification.action() == QJsonDbWatcher::Removed) {
                if (activatedLocks.testFlag(QDeviceInfo::TouchOrKeyboardLock)
                        && strcmp(objectType.constData(), "com.nokia.mt.system.Lockscreen") == 0) {
                    activatedLocks &= ~QDeviceInfo::TouchOrKeyboardLock;
                    emit activatedLocksChanged(activatedLocks);
                } else if (activatedLocks.testFlag(QDeviceInfo::PinLock)
                           && strcmp(objectType.constData(), "com.nokia.mt.system.SecurityLock") == 0) {
                    activatedLocks &= ~QDeviceInfo::PinLock;
                    emit activatedLocksChanged(activatedLocks);
                }
            } else {
                if (objectType.size() == strlen("com.nokia.mt.system.Lockscreen") &&
                    strcmp(objectType.constData(), "com.nokia.mt.system.Lockscreen") == 0) {
                    if (notification.object().value(QString(QStringLiteral("isLocked"))).toBool()) {
                        if (!activatedLocks.testFlag(QDeviceInfo::TouchOrKeyboardLock)) {
                            activatedLocks |= QDeviceInfo::TouchOrKeyboardLock;
                            emit activatedLocksChanged(activatedLocks);
                        }
                    } else {
                        if (activatedLocks.testFlag(QDeviceInfo::TouchOrKeyboardLock)) {
                            activatedLocks &= ~QDeviceInfo::TouchOrKeyboardLock;
                            emit activatedLocksChanged(activatedLocks);
                        }
                    }
                } else if (objectType.size() == strlen("com.nokia.mt.system.SecurityLock") &&
                           strcmp(objectType.constData(), "com.nokia.mt.system.SecurityLock") == 0) {
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
        }
        if (watchEnabledLocks) {
            if (notification.action() == QJsonDbWatcher::Created) {
                if (objectType.size() == strlen("com.nokia.mt.system.Lockscreen") &&
                    strcmp(objectType.constData(), "com.nokia.mt.system.Lockscreen") == 0)
                    enabledLocks |= QDeviceInfo::TouchOrKeyboardLock;
                else if (objectType.size() == strlen("com.nokia.mt.system.SecurityLock") &&
                         strcmp(objectType.constData(), "com.nokia.mt.system.SecurityLock") == 0)
                    enabledLocks |= QDeviceInfo::PinLock;
                emit enabledLocksChanged(enabledLocks);
            } else if (notification.action() == QJsonDbWatcher::Removed) {
                if (objectType.size() == strlen("com.nokia.mt.system.Lockscreen") &&
                    strcmp(objectType.constData(), "com.nokia.mt.system.Lockscreen") == 0)
                    enabledLocks &= ~QDeviceInfo::TouchOrKeyboardLock;
                else if (objectType.size() == strlen("com.nokia.mt.system.SecurityLock") &&
                         strcmp(objectType.constData(), "com.nokia.mt.system.SecurityLock") == 0)
                    enabledLocks &= ~QDeviceInfo::PinLock;
                emit enabledLocksChanged(enabledLocks);
            }
        }
    }
}

void QJsonDbWrapper::onJsonDbWatcherNotificationsBacklightStateAvailable()
{
    QList<QJsonDbNotification> notifications = backlightWatcher->takeNotifications();
    if (notifications.size() > 0) {
        const QJsonDbNotification notification = notifications.at(0);
        const int screen = notification.object().value(QStringLiteral("displayIndex")).toString().toInt();
        QDisplayInfo::BacklightState state;
        if (notification.object().value(QStringLiteral("active")).toBool())
           state = QDisplayInfo::BacklightOn;
        else
           state = QDisplayInfo::BacklightOff;
        emit backlightStateChanged(screen, state);
    }
}

void QJsonDbWrapper::onJsonDbConnectionError(QtJsonDb::QJsonDbConnection::ErrorCode error, const QString &message)
{
    Q_UNUSED(error)
    Q_UNUSED(message)

    timer->stop();
    waitLoop->exit(0);
}

void QJsonDbWrapper::onJsonDbSynchronousRequestError(QtJsonDb::QJsonDbRequest::ErrorCode error, const QString &message)
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

void QJsonDbWrapper::onJsonDbReadRequestError(QtJsonDb::QJsonDbRequest::ErrorCode error, const QString &message)
{
    Q_UNUSED(error)
    Q_UNUSED(message)

    QtJsonDb::QJsonDbRequest *request = qobject_cast<QtJsonDb::QJsonDbRequest *>(sender());
    if (request)
       request->deleteLater();

    if (isLockTypeRequested && activatedLocks == QDeviceInfo::UnknownLock)
       isLockTypeRequested = false;
}

void QJsonDbWrapper::onJsonDbLockObjectsReadRequestFinished()
{
    QtJsonDb::QJsonDbRequest *request = qobject_cast<QtJsonDb::QJsonDbRequest *>(sender());
    if (!request)
       return;
    QList<QJsonObject> results = request->takeResults();
    if (request)
       request->deleteLater();

    if (activatedLocks.testFlag(QDeviceInfo::UnknownLock))
        activatedLocks &= ~QDeviceInfo::UnknownLock;
    if (enabledLocks.testFlag(QDeviceInfo::UnknownLock))
        enabledLocks &= ~QDeviceInfo::UnknownLock;

    if (results.size() > 0) {
        for (int i = 0; i < results.size(); i++) {
            const QByteArray objectType = results.at(i).value(QStringLiteral("_type")).toString().toAscii();
            if (objectType.size() == strlen("com.nokia.mt.system.Lockscreen") &&
                strcmp(objectType.constData(), "com.nokia.mt.system.Lockscreen") == 0) {
                if (!enabledLocks.testFlag(QDeviceInfo::TouchOrKeyboardLock))
                   enabledLocks |= QDeviceInfo::TouchOrKeyboardLock;
                if (results.at(i).value(QString(QStringLiteral("isLocked"))).toBool()) {
                    if (!activatedLocks.testFlag(QDeviceInfo::TouchOrKeyboardLock)) {
                        activatedLocks |= QDeviceInfo::TouchOrKeyboardLock;
                        emit activatedLocksChanged(activatedLocks);
                    }
                } else {
                    if (activatedLocks.testFlag(QDeviceInfo::TouchOrKeyboardLock)) {
                        activatedLocks &= ~QDeviceInfo::TouchOrKeyboardLock;
                        emit activatedLocksChanged(activatedLocks);
                    }
                }
            } else if (objectType.size() == strlen("com.nokia.mt.system.SecurityLock") &&
                       strcmp(objectType.constData(), "com.nokia.mt.system.SecurityLock") == 0) {
                if (!enabledLocks.testFlag(QDeviceInfo::PinLock))
                   enabledLocks |= QDeviceInfo::PinLock;
                if (results.at(i).value(QString(QStringLiteral("active"))).toBool()) {
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
    } else {
        if (activatedLocks.testFlag(QDeviceInfo::TouchOrKeyboardLock)) {
            activatedLocks &= ~QDeviceInfo::TouchOrKeyboardLock;
            emit activatedLocksChanged(activatedLocks);
        }
        if (activatedLocks.testFlag(QDeviceInfo::PinLock)) {
            activatedLocks &= ~QDeviceInfo::PinLock;
            emit activatedLocksChanged(activatedLocks);
        }

        if (enabledLocks.testFlag(QDeviceInfo::TouchOrKeyboardLock)) {
            enabledLocks &= ~QDeviceInfo::TouchOrKeyboardLock;
            emit enabledLocksChanged(enabledLocks);
        }
        if (enabledLocks.testFlag(QDeviceInfo::PinLock)) {
            enabledLocks &= ~QDeviceInfo::PinLock;
            emit enabledLocksChanged(enabledLocks);
        }
    }

    if (!locksWatcher) {
        locksWatcher = new QJsonDbWatcher(this);
        locksWatcher->setPartition(QStringLiteral("Ephemeral"));
        locksWatcher->setWatchedActions(QJsonDbWatcher::All);
        locksWatcher->setQuery(QString(QStringLiteral("[?_type in [\"com.nokia.mt.system.SecurityLock\",\"com.nokia.mt.system.Lockscreen\"]]")));
        connect(locksWatcher, SIGNAL(notificationsAvailable(int)), this, SLOT(onJsonDbWatcherNotificationsAvailable()));
        jsonDbConnection.addWatcher(locksWatcher);
        watchActivatedLocks = true;
        watchEnabledLocks = true;
    }
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
