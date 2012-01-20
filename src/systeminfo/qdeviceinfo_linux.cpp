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

#include "qdeviceinfo_linux_p.h"

#if !defined(QT_NO_JSONDB)
#include "qjsondbwrapper_p.h"
#endif // QT_NO_JSONDB

#if !defined(QT_NO_OFONO)
#include "qofonowrapper_p.h"
#endif // QT_NO_OFONO

#include "qscreensaver_linux_p.h"

#include <QtCore/qdir.h>
#include <QtCore/qprocess.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qtimer.h>

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

QT_BEGIN_NAMESPACE

QDeviceInfoPrivate::QDeviceInfoPrivate(QDeviceInfo *parent)
    : QObject(parent)
#if !defined(QT_SIMULATOR)
    , q_ptr(parent)
#endif // QT_SIMULATOR
    , watchThermalState(false)
    , timer(0)
#if !defined(QT_NO_JSONDB)
    , jsondbWrapper(0)
#endif // QT_NO_JSONDB
#if !defined(QT_NO_OFONO)
    , ofonoWrapper(0)
#endif // QT_NO_OFONO
{
}

bool QDeviceInfoPrivate::hasFeature(QDeviceInfo::Feature feature)
{
    switch (feature) {
    case QDeviceInfo::Bluetooth:
        if (QDir(QStringLiteral("/sys/class/bluetooth/")).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() > 0)
            return true;
        return false;

    case QDeviceInfo::Camera: {
        const QString devfsPath(QStringLiteral("/dev/"));
        const QStringList dirs = QDir(devfsPath).entryList(QStringList() << QStringLiteral("video*"), QDir::System);
        foreach (const QString &dir, dirs) {
            QFile dev(devfsPath + dir);
            if (!dev.open(QIODevice::ReadWrite))
                continue;
            struct v4l2_capability capability;
            memset(&capability, 0, sizeof(struct v4l2_capability));
            if (ioctl(dev.handle(), VIDIOC_QUERYCAP, &capability) != -1
                && (capability.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE) {
                return true;
            }
        }
        return false;
    }

    case QDeviceInfo::FmRadio:
        if (QDir(QStringLiteral("/sys/class/video4linux/")).entryList(QStringList() << QStringLiteral("radio*")).size() > 0)
            return true;
        return false;

    case QDeviceInfo::FmTransmitter: {
        const QString devfsPath(QStringLiteral("/dev/"));
        const QStringList dirs = QDir(devfsPath).entryList(QStringList() << QStringLiteral("radio*"), QDir::System);
        foreach (const QString &dir, dirs) {
            QFile dev(devfsPath + dir);
            if (!dev.open(QIODevice::ReadWrite))
                continue;
            struct v4l2_capability capability;
            memset(&capability, 0, sizeof(struct v4l2_capability));
            if (ioctl(dev.handle(), VIDIOC_QUERYCAP, &capability) != -1
                && (capability.capabilities & (V4L2_CAP_RADIO | V4L2_CAP_MODULATOR)) == (V4L2_CAP_RADIO | V4L2_CAP_MODULATOR)) {
                return true;
            }
        }
        return false;
    }

    case QDeviceInfo::Infrared:
        // TODO: find the kernel interface for this
        return false;

    case QDeviceInfo::Led:
        if (QDir(QStringLiteral("/sys/class/leds/")).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() > 0)
            return true;
        return false;

    case QDeviceInfo::MemoryCard:
        if (QDir(QStringLiteral("/sys/class/mmc_host/")).entryList(QStringList() << QStringLiteral("mmc*")).size() > 0)
            return true;
        return false;

    case QDeviceInfo::Usb:
        if (QDir(QStringLiteral("/sys/bus/usb/devices/")).entryList(QStringList() << QStringLiteral("usb*")).size() > 0)
            return true;
        return false;

    case QDeviceInfo::Vibration:
        // TODO: find the kernel interface for this
#if !defined(QT_NO_JSONDB)
        if (!jsondbWrapper)
            jsondbWrapper = new QJsonDbWrapper(this);
        return jsondbWrapper->hasFeatureVibration();
#endif
        return false;

    case QDeviceInfo::Wlan:
        if ((QDir(QStringLiteral("/sys/class/ieee80211/")).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() > 0)
                || (QDir(QStringLiteral("/sys/class/net/")).entryList(QStringList() << QStringLiteral("wlan*")).size() > 0))
            return true;
        return false;

    case QDeviceInfo::Sim:
#if !defined(QT_NO_OFONO)
    if (QOfonoWrapper::isOfonoAvailable()) {
        if (!ofonoWrapper)
            ofonoWrapper = new QOfonoWrapper(this);
        return (ofonoWrapper->allModems().size() > 0);
    }
#endif
        return false;

    case QDeviceInfo::Positioning:
        // TODO: find the kernel interface for this
#if !defined(QT_NO_JSONDB)
        if (!jsondbWrapper)
            jsondbWrapper = new QJsonDbWrapper(this);
        return jsondbWrapper->hasFeaturePositioning();
#endif
        return false;

    case QDeviceInfo::VideoOut: {
        const QString devfsPath(QStringLiteral("/dev/"));
        const QStringList dirs = QDir(devfsPath).entryList(QStringList() << QStringLiteral("video*"), QDir::System);
        foreach (const QString &dir, dirs) {
            QFile dev(devfsPath + dir);
            if (!dev.open(QIODevice::ReadWrite))
                continue;
            struct v4l2_capability capability;
            memset(&capability, 0, sizeof(struct v4l2_capability));
            if (ioctl(dev.handle(), VIDIOC_QUERYCAP, &capability) != -1
                && (capability.capabilities & V4L2_CAP_VIDEO_OUTPUT) == V4L2_CAP_VIDEO_OUTPUT) {
                return true;
            }
        }
        if (QDir(QStringLiteral("/sys/class/video_output/")).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() > 0)
            return true;
        return false;
    }

    case QDeviceInfo::Haptics:
        if (QDir(QStringLiteral("/sys/class/haptic/")).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() > 0)
            return true;
        return false;

    case QDeviceInfo::Nfc:
        // As of now, it's the only supported NFC device in the kernel
        return QFile::exists(QStringLiteral("/dev/pn544"));
    }

    return false;
}

QDeviceInfo::LockTypeFlags QDeviceInfoPrivate::activatedLocks()
{
#if !defined(QT_NO_JSONDB)
        if (!jsondbWrapper)
            jsondbWrapper = new QJsonDbWrapper(this);
        return jsondbWrapper->getActivatedLocks();
#endif
    return QDeviceInfo::NoLock;
}

QDeviceInfo::LockTypeFlags QDeviceInfoPrivate::enabledLocks()
{
#if !defined(QT_NO_JSONDB)
        if (!jsondbWrapper)
            jsondbWrapper = new QJsonDbWrapper(this);
        return jsondbWrapper->getEnabledLocks();
#endif

    QDeviceInfo::LockTypeFlags enabledLocks = QDeviceInfo::NoLock;

    QScreenSaverPrivate screenSaver(0);
    if (screenSaver.screenSaverEnabled())
        enabledLocks = QDeviceInfo::TouchOrKeyboardLock;

    return enabledLocks;
}

QDeviceInfo::ThermalState QDeviceInfoPrivate::thermalState()
{
    if (watchThermalState)
        return currentThermalState;
    else
        return getThermalState();
}

int QDeviceInfoPrivate::imeiCount()
{
#if !defined(QT_NO_OFONO)
    if (QOfonoWrapper::isOfonoAvailable()) {
        if (!ofonoWrapper)
            ofonoWrapper = new QOfonoWrapper(this);
        return ofonoWrapper->allModems().size();
    }
#endif
    return -1;
}

QString QDeviceInfoPrivate::imei(int interface)
{
#if !defined(QT_NO_OFONO)
    if (QOfonoWrapper::isOfonoAvailable()) {
        if (!ofonoWrapper)
            ofonoWrapper = new QOfonoWrapper(this);
        QStringList modems = ofonoWrapper->allModems();
        if (interface >= 0 && interface < modems.size()) {
            QString modem = ofonoWrapper->allModems().at(interface);
            if (!modem.isEmpty())
                return ofonoWrapper->imei(modem);
        }
    }
#else
    Q_UNUSED(interface)
#endif
    return QString();
}

QString QDeviceInfoPrivate::manufacturer()
{
    if (manufacturerBuffer.isEmpty()) {
        QStringList paths;
        paths << QStringLiteral("/sys/kernel/info/make") << QStringLiteral("/sys/devices/virtual/dmi/id/sys_vendor");
        foreach (const QString &path, paths) {
            QFile file(path);
            if (file.open(QIODevice::ReadOnly)) {
                manufacturerBuffer = QString::fromLocal8Bit(file.readAll().simplified().data());
                break;
            }
        }
    }
    return manufacturerBuffer;
}

QString QDeviceInfoPrivate::model()
{
    if (modelBuffer.isEmpty()) {
        QStringList paths;
        paths << QStringLiteral("/sys/kernel/info/model") << QStringLiteral("/sys/devices/virtual/dmi/id/product_name");
        foreach (const QString &path, paths) {
            QFile file(path);
            if (file.open(QIODevice::ReadOnly)) {
                modelBuffer = QString::fromLocal8Bit(file.readAll().simplified().data());
                break;
            }
        }
    }
    return modelBuffer;
}

QString QDeviceInfoPrivate::productName()
{
    if (productNameBuffer.isEmpty()) {
        QFile file(QStringLiteral("/sys/kernel/info/type"));
        if (file.open(QIODevice::ReadOnly))
            productNameBuffer = QString::fromLocal8Bit((file.readAll().simplified().data()));
    }
    if (productNameBuffer.isEmpty()) {
        QProcess lsbRelease;
        lsbRelease.start(QStringLiteral("/usr/bin/lsb_release"),
                         QStringList() << QStringLiteral("-c"));
        if (lsbRelease.waitForFinished()) {
            QString buffer(QString::fromLocal8Bit(lsbRelease.readAllStandardOutput().constData()));
            productNameBuffer = buffer.section(QChar::fromAscii('\t'), 1, 1).simplified();
        }
    }
    return productNameBuffer;
}

QString QDeviceInfoPrivate::uniqueDeviceID()
{
    if (uniqueDeviceIDBuffer.isEmpty()) {
#if !defined(QT_NO_JSONDB)
        if (!jsondbWrapper)
            jsondbWrapper = new QJsonDbWrapper(this);
        uniqueDeviceIDBuffer = jsondbWrapper->getUniqueDeviceID();
#else
        QFile file(QStringLiteral("/sys/devices/virtual/dmi/id/product_uuid"));
        if (file.open(QIODevice::ReadOnly))
            uniqueDeviceIDBuffer = QString::fromLocal8Bit(file.readAll().simplified().data());
#endif // QT_NO_JSONDB
    }

    return uniqueDeviceIDBuffer;
}

QString QDeviceInfoPrivate::version(QDeviceInfo::Version type)
{
    switch (type) {
    case QDeviceInfo::Os:
        if (versionBuffer[0].isEmpty() && QFile::exists(QStringLiteral("/usr/bin/lsb_release"))) {
            QProcess lsbRelease;
            lsbRelease.start(QStringLiteral("/usr/bin/lsb_release"),
                             QStringList() << QStringLiteral("-r"));
            if (lsbRelease.waitForFinished()) {
                QString buffer(QString::fromLocal8Bit(lsbRelease.readAllStandardOutput().constData()));
                versionBuffer[0] = buffer.section(QChar::fromAscii('\t'), 1, 1).simplified();
            }
        }
        if (versionBuffer[0].isEmpty()) {
            QStringList releaseFies = QDir(QStringLiteral("/etc/")).entryList(QStringList() << QStringLiteral("*-release"));
            foreach (const QString &file, releaseFies) {
                QFile release(QStringLiteral("/etc/") + file);
                if (release.open(QIODevice::ReadOnly)) {
                    QString all(QString::fromLocal8Bit(release.readAll().constData()));
                    QRegExp regExp(QStringLiteral("\\d+(\\.\\d+)*"));
                    if (-1 != regExp.indexIn(all)) {
                        versionBuffer[0] = regExp.cap(0);
                        break;
                    }
                }
            }
        }
        return versionBuffer[0];

    case QDeviceInfo::Firmware:
        if (versionBuffer[1].isEmpty()) {
            QFile file(QStringLiteral("/proc/sys/kernel/osrelease"));
            if (file.open(QIODevice::ReadOnly))
                versionBuffer[1] = QString::fromLocal8Bit(file.readAll().simplified().data());
        }
        return versionBuffer[1];
    }

    return QString();
}

void QDeviceInfoPrivate::connectNotify(const char *signal)
{
#if !defined(QT_NO_JSONDB)
    if (strcmp(signal, SIGNAL(activatedLocksChanged(QDeviceInfo::LockTypeFlags))) == 0
            || strcmp(signal, SIGNAL(enabledLocksChanged(QDeviceInfo::LockTypeFlags))) == 0) {
        if (!jsondbWrapper)
            jsondbWrapper = new QJsonDbWrapper(this);
        connect(jsondbWrapper, signal, this, signal, Qt::UniqueConnection);
        return;
    }
#endif // // QT_NO_JSONDB

    if (timer == 0) {
        timer = new QTimer;
        timer->setInterval(2000);
        connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    }

    if (!timer->isActive())
        timer->start();

    if (strcmp(signal, SIGNAL(thermalStateChanged(QDeviceInfo::ThermalState))) == 0) {
        watchThermalState = true;
        currentThermalState = getThermalState();
    }
}

void QDeviceInfoPrivate::disconnectNotify(const char *signal)
{
#if !defined(QT_NO_JSONDB)
    if ((strcmp(signal, SIGNAL(activatedLocksChanged(QDeviceInfo::LockTypeFlags))) == 0
         || strcmp(signal, SIGNAL(enabledLocksChanged(QDeviceInfo::LockTypeFlags))) == 0)
            && jsondbWrapper) {
        disconnect(jsondbWrapper, signal, this, signal);
        return;
    }
#endif // QT_NO_JSONDB

    if (strcmp(signal, SIGNAL(thermalStateChanged(QDeviceInfo::ThermalState))) == 0) {
        watchThermalState = false;
        currentThermalState = QDeviceInfo::UnknownThermal;
    }

    if (!watchThermalState)
        timer->stop();
}

void QDeviceInfoPrivate::onTimeout()
{
    if (watchThermalState) {
        QDeviceInfo::ThermalState newState = getThermalState();
        if (newState != currentThermalState) {
            currentThermalState = newState;
            emit thermalStateChanged(currentThermalState);
        }
    }
}

QDeviceInfo::ThermalState QDeviceInfoPrivate::getThermalState()
{
    QDeviceInfo::ThermalState state = QDeviceInfo::UnknownThermal;

    const QString hwmonRoot(QStringLiteral("/sys/class/hwmon/"));
    const QStringList hwmonDirs(QDir(hwmonRoot).entryList(QStringList() << QStringLiteral("hwmon*")));
    foreach (const QString &dir, hwmonDirs) {
        int index = 1;
        const QString input(hwmonRoot + dir + QDir::separator() + QStringLiteral("temp%1_input"));
        const QString critical(hwmonRoot + dir + QDir::separator() + QStringLiteral("temp%1_crit"));
        const QString emergency(hwmonRoot + dir + QDir::separator() + QStringLiteral("temp%1_emergency"));
        while (true) {
            QFile file(input.arg(index));
            if (!file.open(QIODevice::ReadOnly))
                break;
            bool ok(false);
            int currentTemp = file.readAll().simplified().toInt(&ok);
            if (ok) {
                if (state == QDeviceInfo::UnknownThermal)
                    state = QDeviceInfo::NormalThermal;

                // Only check if we are below WarningThermal
                if (state < QDeviceInfo::WarningThermal) {
                    file.close();
                    file.setFileName(critical.arg(index));
                    if (file.open(QIODevice::ReadOnly)) {
                        int criticalTemp = file.readAll().simplified().toInt(&ok);
                        if (ok && currentTemp > criticalTemp)
                            state = QDeviceInfo::WarningThermal;
                    }
                }

                // Only check if we are below AlertThermal
                if (state < QDeviceInfo::AlertThermal) {
                    file.close();
                    file.setFileName(emergency.arg(index));
                    if (file.open(QIODevice::ReadOnly)) {
                        int emergencyTemp = file.readAll().simplified().toInt(&ok);
                        if (ok && currentTemp > emergencyTemp) {
                            state = QDeviceInfo::AlertThermal;
                            break; // No need for further checking, as we can't get the ErrorThermal state
                        }
                    }
                }
            } else {
                break;
            }

            ++index;
        }
    }

    return state;
}

QT_END_NAMESPACE
