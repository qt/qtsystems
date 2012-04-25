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
#include <QtCore/qmetaobject.h>
#include <QtCore/qtimer.h>
#include <QtJsonDb/qjsondbreadrequest.h>
#include <QtJsonDb/qjsondbwatcher.h>

QT_USE_NAMESPACE_JSONDB

QT_BEGIN_NAMESPACE

const int JSON_EXPIRATION_TIMER(2000);

QJsonDbWrapper::QJsonDbWrapper(QObject *parent)
    : QObject(parent)
    , locksWatcher(0)
    , soundSettingsWatcher(0)
    , backlightWatcher(0)
    , waitLoop(0)
    , timer(0)
    , watchActivatedLocks(false)
    , watchEnabledLocks(false)
    , watchProfile(false)
    , watchBacklightState(false)
    , activatedLocks(QDeviceInfo::UnknownLock)
    , enabledLocks(QDeviceInfo::UnknownLock)
    , isLockTypeRequested(false)
    , isSoundSettingsRequested(false)
    , vibrationActivated(false)
    , ringerVolume(-1)
    , profileType(QDeviceProfile::UnknownProfile)
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
       sendJsonDbLockObjectsReadRequest();
    }

    return activatedLocks;
}

QDeviceInfo::LockTypeFlags QJsonDbWrapper::enabledLockTypes()
{
    if (!isLockTypeRequested && enabledLocks == QDeviceInfo::UnknownLock) {
       isLockTypeRequested = true;
       sendJsonDbLockObjectsReadRequest();
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
    if (!isSoundSettingsRequested && ringerVolume < 0) {
        isSoundSettingsRequested = true;
        sendJsonDbSoundSettingsReadRequest();
    }

    return vibrationActivated;
}

int QJsonDbWrapper::ringtoneVolume()
{
    if (!isSoundSettingsRequested && ringerVolume < 0) {
        isSoundSettingsRequested = true;
        sendJsonDbSoundSettingsReadRequest();
    }

    return ringerVolume;
}

QDeviceProfile::ProfileType QJsonDbWrapper::currentProfileType()
{
    if (!isSoundSettingsRequested && ringerVolume < 0) {
        isSoundSettingsRequested = true;
        sendJsonDbSoundSettingsReadRequest();
    }

    return profileType;
}

void QJsonDbWrapper::sendJsonDbLockObjectsReadRequest()
{
    QJsonDbReadRequest *request = new QtJsonDb::QJsonDbReadRequest();
    request->setQuery(QString(QStringLiteral("[?_type in [\"com.nokia.mt.system.SecurityLock\",\"com.nokia.mt.system.Lockscreen\"]]")));
    request->setPartition(QString(QStringLiteral("Ephemeral")));
    connect(request, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            this, SLOT(onJsonDbReadRequestError(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));
    connect(request, SIGNAL(finished()), this, SLOT(onJsonDbLockObjectsReadRequestFinished()));
    jsonDbConnection.send(request);
}

void QJsonDbWrapper::sendJsonDbSoundSettingsReadRequest()
{
    QJsonDbReadRequest *request = new QJsonDbReadRequest();
    request->setQuery(QString(QStringLiteral("[?_type=\"com.nokia.mt.settings.SystemSettings\"][?identifier=\"com.nokia.mt.settings.sounds\"][={settings:settings}]")));
    connect(request, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            this, SLOT(onJsonDbReadRequestError(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));
    connect(request, SIGNAL(finished()), this, SLOT(onJsonDbSoundSettingsReadRequestFinished()));
    jsonDbConnection.send(request);
}

QJsonValue QJsonDbWrapper::getSystemSettingValue(const QString &settingId, const QString &setting, const QString &partition)
{
    QJsonDbReadRequest request;
    if (!partition.isEmpty())
       request.setPartition(partition);
    request.setQuery(QString(QStringLiteral("[?_type=\"com.nokia.mt.settings.SystemSettings\"][?identifier=\"com.nokia.mt.settings.%1\"][={settings:settings}]"))
                     .arg(settingId));
    connect(&request, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            this, SLOT(onJsonDbSynchronousRequestError(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));
    connect(&request, SIGNAL(finished()), this, SLOT(onJsonDbSynchronousRequestFinished()));
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

void QJsonDbWrapper::connectNotify(const QMetaMethod &signal)
{
    static const QMetaMethod activatedLocksChangedSignal = QMetaMethod::fromSignal(&QJsonDbWrapper::activatedLocksChanged);
    static const QMetaMethod enabledLocksChangedSignal = QMetaMethod::fromSignal(&QJsonDbWrapper::enabledLocksChanged);
    static const QMetaMethod vibrationActivatedChangedSignal = QMetaMethod::fromSignal(&QJsonDbWrapper::vibrationActivatedChanged);
    static const QMetaMethod ringtoneVolumeChangedSignal = QMetaMethod::fromSignal(&QJsonDbWrapper::ringtoneVolumeChanged);
    static const QMetaMethod currentProfileTypeChangedSignal = QMetaMethod::fromSignal(&QJsonDbWrapper::currentProfileTypeChanged);
    static const QMetaMethod backlightStateChangedSignal = QMetaMethod::fromSignal(&QJsonDbWrapper::backlightStateChanged);

    if (watchActivatedLocks && watchEnabledLocks && watchProfile && watchBacklightState)
        return;

    bool needWatchActivatedLocks = (signal == activatedLocksChangedSignal);
    if (needWatchActivatedLocks) {
        if (watchActivatedLocks)
            return;
        activatedLocks = activatedLockTypes();
    }

    bool needWatchEnabledLocks = (signal == enabledLocksChangedSignal);
    if (needWatchEnabledLocks) {
        if (watchEnabledLocks)
            return;
        enabledLocks = enabledLockTypes();
    }

    if (needWatchActivatedLocks)
        watchActivatedLocks = true;

    if (needWatchEnabledLocks)
        watchEnabledLocks = true;

    bool needWatchProfiles = (signal == vibrationActivatedChangedSignal
                              || signal == ringtoneVolumeChangedSignal
                              || signal == currentProfileTypeChangedSignal);
    if (needWatchProfiles) {
        if (watchProfile)
            return;
        currentProfileType();
        watchProfile = true;
    }

    if (signal == backlightStateChangedSignal) {
        if (!backlightWatcher) {
            backlightWatcher = new QtJsonDb::QJsonDbWatcher(this);
            backlightWatcher->setPartition(QString(QStringLiteral("Ephemeral")));
            backlightWatcher->setWatchedActions(QJsonDbWatcher::Updated);
            backlightWatcher->setQuery(QString(QStringLiteral("[?_type=\"com.nokia.mt.system.DisplayState\"]")));
            connect(backlightWatcher, SIGNAL(notificationsAvailable(int)), this, SLOT(onJsonDbWatcherBacklightStateNotificationsAvailable()));
        }
        jsonDbConnection.addWatcher(backlightWatcher);
        watchBacklightState = true;
    }
}

void QJsonDbWrapper::disconnectNotify(const QMetaMethod &signal)
{
    if (!watchActivatedLocks && !watchEnabledLocks && !watchBacklightState)
        return;

    static const QMetaMethod activatedLocksChangedSignal = QMetaMethod::fromSignal(&QJsonDbWrapper::activatedLocksChanged);
    static const QMetaMethod enabledLocksChangedSignal = QMetaMethod::fromSignal(&QJsonDbWrapper::enabledLocksChanged);
    static const QMetaMethod backlightStateChangedSignal = QMetaMethod::fromSignal(&QJsonDbWrapper::backlightStateChanged);
    if (signal == activatedLocksChangedSignal)
        watchActivatedLocks = false;
    else if (signal == enabledLocksChangedSignal)
        watchEnabledLocks = false;
    else if (signal == backlightStateChangedSignal)
        watchBacklightState = false;

    if (!watchBacklightState)
        jsonDbConnection.removeWatcher(backlightWatcher);
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
            const QByteArray objectType = results.at(i).value(QStringLiteral("_type")).toString().toLatin1();
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
        connect(locksWatcher, SIGNAL(notificationsAvailable(int)), this, SLOT(onJsonDbWatcherLocksNotificationsAvailable()));
        jsonDbConnection.addWatcher(locksWatcher);
    }
}

void QJsonDbWrapper::onJsonDbSoundSettingsReadRequestFinished()
{
    QtJsonDb::QJsonDbRequest *request = qobject_cast<QtJsonDb::QJsonDbRequest *>(sender());
    if (!request)
       return;

    if (request && request->status() == QJsonDbRequest::Finished) {
        QList<QJsonObject> results = request->takeResults();
        request->deleteLater();
        if (results.size() > 0) {
            QJsonObject soundSettings = results.at(0).value(QString(QStringLiteral("settings"))).toObject();
            bool vibration = false;
            if (!soundSettings.value(QString(QStringLiteral("vibrationOn"))).isUndefined())
                vibration = soundSettings.value(QString(QStringLiteral("vibrationOn"))).toBool();
            int volume = -1;
            if (!soundSettings.value(QString(QStringLiteral("ringerVolume"))).isUndefined())
                volume = int(soundSettings.value(QString(QStringLiteral("ringerVolume"))).toDouble());

            if (volume > -1) {
                QDeviceProfile::ProfileType profile = QDeviceProfile::UnknownProfile;
                if (volume > 0) {
                    profile = QDeviceProfile::NormalProfile;
                } else {
                    if (vibration)
                        profile = QDeviceProfile::VibrationProfile;
                    else
                        profile = QDeviceProfile::SilentProfile;
                }
                if (ringerVolume != volume) {
                    ringerVolume = volume;
                    if (watchProfile)
                        emit ringtoneVolumeChanged(ringerVolume);
                }
                if (vibrationActivated != vibration) {
                    vibrationActivated = vibration;
                    if (watchProfile)
                        emit vibrationActivatedChanged(vibrationActivated);
                }
                if (profileType != profile) {
                    profileType = profile;
                    if (watchProfile)
                        emit currentProfileTypeChanged(profileType);
                }
            }
        }
    }

    if (!soundSettingsWatcher) {
        soundSettingsWatcher = new QJsonDbWatcher(this);
        soundSettingsWatcher->setWatchedActions(QJsonDbWatcher::Updated);
        soundSettingsWatcher->setQuery(QString(QStringLiteral("[?_type=\"com.nokia.mt.settings.SystemSettings\"][?identifier=\"com.nokia.mt.settings.sounds\"][={settings:settings}]")));
        connect(soundSettingsWatcher, SIGNAL(notificationsAvailable(int)),
                this, SLOT(onJsonDbWatcherSoundSettingsNotificationsAvailable()));
        jsonDbConnection.addWatcher(soundSettingsWatcher);
    }
}

void QJsonDbWrapper::onJsonDbWatcherLocksNotificationsAvailable()
{
    QList<QJsonDbNotification> notifications = locksWatcher->takeNotifications();
    if (notifications.size() > 0) {
        const QJsonDbNotification notification = notifications.at(0);
        const QByteArray objectType = notification.object().value(QStringLiteral("_type")).toString().toLatin1();
        if (notification.action() == QJsonDbWatcher::Removed) {
            if (objectType.size() == strlen("com.nokia.mt.system.Lockscreen")
                    && strcmp(objectType.constData(), "com.nokia.mt.system.Lockscreen") == 0) {
                if (activatedLocks.testFlag(QDeviceInfo::TouchOrKeyboardLock)) {
                    activatedLocks &= ~QDeviceInfo::TouchOrKeyboardLock;
                    if (watchActivatedLocks)
                        emit activatedLocksChanged(activatedLocks);
                }
                if (enabledLocks.testFlag(QDeviceInfo::TouchOrKeyboardLock)) {
                    enabledLocks &= ~QDeviceInfo::TouchOrKeyboardLock;
                    if (watchEnabledLocks)
                        emit enabledLocksChanged(enabledLocks);
                }
            } else if (objectType.size() == strlen("com.nokia.mt.system.SecurityLock")
                       && strcmp(objectType.constData(), "com.nokia.mt.system.SecurityLock") == 0) {
                if (activatedLocks.testFlag(QDeviceInfo::PinLock)) {
                    activatedLocks &= ~QDeviceInfo::PinLock;
                    if (watchActivatedLocks)
                        emit activatedLocksChanged(activatedLocks);
                }
                if (enabledLocks.testFlag(QDeviceInfo::PinLock)) {
                    enabledLocks &= ~QDeviceInfo::PinLock;
                    if (watchEnabledLocks)
                        emit enabledLocksChanged(enabledLocks);
                }
            }
        } else {
            if (objectType.size() == strlen("com.nokia.mt.system.Lockscreen")
                    && strcmp(objectType.constData(), "com.nokia.mt.system.Lockscreen") == 0) {
                if (notification.object().value(QString(QStringLiteral("isLocked"))).toBool()) {
                    if (!activatedLocks.testFlag(QDeviceInfo::TouchOrKeyboardLock)) {
                        activatedLocks |= QDeviceInfo::TouchOrKeyboardLock;
                        if (watchActivatedLocks)
                            emit activatedLocksChanged(activatedLocks);
                    }
                } else {
                    if (activatedLocks.testFlag(QDeviceInfo::TouchOrKeyboardLock)) {
                        activatedLocks &= ~QDeviceInfo::TouchOrKeyboardLock;
                        if (watchActivatedLocks)
                            emit activatedLocksChanged(activatedLocks);
                    }
                }
                if (notification.action() == QJsonDbWatcher::Created) {
                    if (!enabledLocks.testFlag(QDeviceInfo::TouchOrKeyboardLock)) {
                        enabledLocks |= QDeviceInfo::TouchOrKeyboardLock;
                        if (watchEnabledLocks)
                            emit enabledLocksChanged(enabledLocks);
                    }
                }
            } else if (objectType.size() == strlen("com.nokia.mt.system.SecurityLock")
                       && strcmp(objectType.constData(), "com.nokia.mt.system.SecurityLock") == 0) {
                if (notification.object().value(QString(QStringLiteral("active"))).toBool()) {
                    if (!activatedLocks.testFlag(QDeviceInfo::PinLock)) {
                        activatedLocks |= QDeviceInfo::PinLock;
                        if (watchActivatedLocks)
                            emit activatedLocksChanged(activatedLocks);
                    }
                } else {
                    if (activatedLocks.testFlag(QDeviceInfo::PinLock)) {
                        activatedLocks &= ~QDeviceInfo::PinLock;
                        if (watchActivatedLocks)
                            emit activatedLocksChanged(activatedLocks);
                    }
                }
                if (notification.action() == QJsonDbWatcher::Created) {
                    if (!enabledLocks.testFlag(QDeviceInfo::PinLock)) {
                        enabledLocks |= QDeviceInfo::PinLock;
                        if (watchEnabledLocks)
                            emit enabledLocksChanged(enabledLocks);
                    }
                }
            }
        }
    }
}

void QJsonDbWrapper::onJsonDbWatcherSoundSettingsNotificationsAvailable()
{
    QList<QJsonDbNotification> notifications = soundSettingsWatcher->takeNotifications();
    if (notifications.size() > 0) {
        const QJsonDbNotification notification = notifications.at(0);
        const QByteArray objectType = notification.object().value(QStringLiteral("_type")).toString().toLatin1();
        if (objectType.size() == strlen("com.nokia.mt.settings.SystemSettings")
                && strcmp(objectType.constData(), "com.nokia.mt.settings.SystemSettings") == 0) {
            const QByteArray identifier = notification.object().value(QStringLiteral("identifier")).toString().toLatin1();
            if (identifier.size() == strlen("com.nokia.mt.settings.sounds")
                    && strcmp(identifier.constData(), "com.nokia.mt.settings.sounds") == 0) {
                bool vibration = notification.object().value(QString(QStringLiteral("settings"))).toObject().value(QString(QStringLiteral("vibrationOn"))).toBool();
                int volume = int(notification.object().value(QString(QStringLiteral("settings"))).toObject().value(QString(QStringLiteral("ringerVolume"))).toDouble());
                if (volume != ringerVolume || vibration != vibrationActivated) {
                    QDeviceProfile::ProfileType profile = QDeviceProfile::UnknownProfile;
                    if (volume > 0) {
                        profile = QDeviceProfile::NormalProfile;
                    } else {
                        if (vibration)
                            profile = QDeviceProfile::VibrationProfile;
                        else
                            profile = QDeviceProfile::SilentProfile;
                    }
                    if (ringerVolume != volume) {
                        ringerVolume = volume;
                        if (watchProfile)
                            emit ringtoneVolumeChanged(ringerVolume);
                    }
                    if (vibrationActivated != vibration) {
                        vibrationActivated = vibration;
                        if (watchProfile)
                            emit vibrationActivatedChanged(vibrationActivated);
                    }
                    if (profileType != profile) {
                        profileType = profile;
                        if (watchProfile)
                            emit currentProfileTypeChanged(profileType);
                    }
                }
            }
        }
    }
}

void QJsonDbWrapper::onJsonDbWatcherBacklightStateNotificationsAvailable()
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

    if (timer)
        timer->stop();
    if (waitLoop)
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

    if (isSoundSettingsRequested && ringerVolume == -1)
        isSoundSettingsRequested = false;
}

void QJsonDbWrapper::onJsonDbSynchronousRequestError(QtJsonDb::QJsonDbRequest::ErrorCode error, const QString &message)
{
    Q_UNUSED(error)
    Q_UNUSED(message)

    timer->stop();
    waitLoop->exit(0);
}

void QJsonDbWrapper::onJsonDbSynchronousRequestFinished()
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
