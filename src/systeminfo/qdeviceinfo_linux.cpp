/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeviceinfo_linux_p.h"

#if !defined(QT_NO_OFONO)
#include "qofonowrapper_p.h"
#endif // QT_NO_OFONO

#include <QtCore/qdir.h>
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
    , q_ptr(parent)
    , watchThermalState(false)
    , timer(0)
#if !defined(QT_NO_OFONO)
    , ofonoWrapper(0)
#endif // QT_NO_OFONO
{
}

bool QDeviceInfoPrivate::hasFeature(QDeviceInfo::Feature feature)
{
    switch (feature) {
    case QDeviceInfo::Bluetooth:
        if (QDir(QString::fromAscii("/sys/class/bluetooth/")).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() > 0)
            return true;
        return false;

    case QDeviceInfo::Camera: {
        const QString devfsPath(QString::fromAscii("/dev/"));
        const QStringList dirs = QDir(devfsPath).entryList(QStringList() << QString::fromAscii("video*"), QDir::System);
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
        if (QDir(QString::fromAscii("/sys/class/video4linux/")).entryList(QStringList() << QString::fromAscii("radio*")).size() > 0)
            return true;
        return false;

    case QDeviceInfo::FmTransmitter: {
        const QString devfsPath(QString::fromAscii("/dev/"));
        const QStringList dirs = QDir(devfsPath).entryList(QStringList() << QString::fromAscii("radio*"), QDir::System);
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
        if (QDir(QString::fromAscii("/sys/class/leds/")).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() > 0)
            return true;
        return false;

    case QDeviceInfo::MemoryCard:
        if (QDir(QString::fromAscii("/sys/class/mmc_host/")).entryList(QStringList() << QString::fromAscii("mmc*")).size() > 0)
            return true;
        return false;

    case QDeviceInfo::Usb:
        if (QDir(QString::fromAscii("/sys/bus/usb/devices/")).entryList(QStringList() << QString::fromAscii("usb*")).size() > 0)
            return true;
        return false;

    case QDeviceInfo::Vibration:
        // TODO: find the kernel interface for this
        return false;

    case QDeviceInfo::Wlan:
        if (QDir(QString::fromAscii("/sys/class/ieee80211/")).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() > 0)
            return true;
        return false;

    case QDeviceInfo::Sim:
#if !defined(QT_NO_OFONO)
    if (QOfonoWrapper::isOfonoAvailable()) {
        if (!ofonoWrapper)
            ofonoWrapper = new QOfonoWrapper(this);
        QStringList modems = ofonoWrapper->allModems();
        foreach (const QString &modem, modems) {
            if (!ofonoWrapper->imei(modem).isEmpty())
                return true;
        }
    }
#endif
        return false;

    case QDeviceInfo::Positioning:
        // TODO: find the kernel interface for this
        return false;

    case QDeviceInfo::VideoOut: {
        const QString devfsPath(QString::fromAscii("/dev/"));
        const QStringList dirs = QDir(devfsPath).entryList(QStringList() << QString::fromAscii("video*"), QDir::System);
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
        return false;
    }

    case QDeviceInfo::Haptics:
        if (QDir(QString::fromAscii("/sys/class/haptic/")).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() > 0)
            return true;
        return false;

    case QDeviceInfo::Nfc:
        // As of now, it's the only supported NFC device in the kernel
        return QFile::exists(QString::fromAscii("/dev/pn544"));
    }

    return false;
}

QDeviceInfo::LockTypeFlags QDeviceInfoPrivate::activatedLocks()
{
    return QDeviceInfo::NoLock;
}

QDeviceInfo::LockTypeFlags QDeviceInfoPrivate::enabledLocks()
{
    return QDeviceInfo::NoLock;
}

QDeviceInfo::ThermalState QDeviceInfoPrivate::thermalState()
{
    if (watchThermalState)
        return currentThermalState;
    else
        return getThermalState();
}

QString QDeviceInfoPrivate::imei(int interface)
{
#if !defined(QT_NO_OFONO)
    if (QOfonoWrapper::isOfonoAvailable()) {
        if (!ofonoWrapper)
            ofonoWrapper = new QOfonoWrapper(this);
        QString modem = ofonoWrapper->allModems().at(interface);
        if (!modem.isEmpty())
            return ofonoWrapper->imei(modem);
    }
#else
    Q_UNUSED(interface)
#endif
    return QString();
}

QString QDeviceInfoPrivate::manufacturer()
{
    if (manufacturerBuffer.isEmpty()) {
        QFile file(QString::fromAscii("/sys/devices/virtual/dmi/id/sys_vendor"));
        if (file.open(QIODevice::ReadOnly))
            manufacturerBuffer = QString::fromAscii(file.readAll().simplified().data());
    }
    return manufacturerBuffer;
}

QString QDeviceInfoPrivate::model()
{
    if (modelBuffer.isEmpty()) {
        QFile file(QString::fromAscii("/sys/devices/virtual/dmi/id/product_name"));
        if (file.open(QIODevice::ReadOnly))
            modelBuffer = QString::fromAscii(file.readAll().simplified().data());
    }
    return modelBuffer;
}

QString QDeviceInfoPrivate::productName()
{
    if (productNameBuffer.isEmpty()) {
        QFile file(QString::fromAscii("/etc/lsb-release"));
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream textStream(&file);
            while (!textStream.atEnd()) {
                QString line = textStream.readLine();
                if (line.contains(QString::fromAscii("DISTRIB_CODENAME")))
                    productNameBuffer = line.split(QString::fromAscii("=")).at(1).simplified();
            }
        }
    }
    return productNameBuffer;
}

QString QDeviceInfoPrivate::uniqueDeviceID()
{
    if (uniqueDeviceIDBuffer.isEmpty()) {
        QFile file(QString::fromAscii("/sys/devices/virtual/dmi/id/product_uuid"));
        if (file.open(QIODevice::ReadOnly))
            modelBuffer = QString::fromAscii(file.readAll().simplified().data());
    }
    return uniqueDeviceIDBuffer;
}

QString QDeviceInfoPrivate::version(QDeviceInfo::Version type)
{
    switch(type) {
    case QDeviceInfo::Os:
        if (versionBuffer[0].isEmpty()) {
            QFile file(QString::fromAscii("/etc/lsb-release"));
            if (file.open(QIODevice::ReadOnly)) {
                QTextStream textStream(&file);
                while (!textStream.atEnd()) {
                    QString line = textStream.readLine();
                    if (line.contains(QString::fromAscii("DISTRIB_RELEASE")))
                        versionBuffer[0] = line.split(QString::fromAscii("=")).at(1).simplified();
                }
            }
        }
        return versionBuffer[0];

    case QDeviceInfo::Firmware:
        if (versionBuffer[1].isEmpty()) {
            QFile file(QString::fromAscii("/proc/sys/kernel/osrelease"));
            if (file.open(QIODevice::ReadOnly))
                versionBuffer[1] = QString::fromAscii(file.readAll().simplified().data());
        }
        return versionBuffer[1];
    };

    return QString();
}

void QDeviceInfoPrivate::connectNotify(const char *signal)
{
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
    if (strcmp(signal, SIGNAL(thermalStateChanged(QDeviceInfo::ThermalState))) == 0)
        watchThermalState = false;

    if (!watchThermalState)
        timer->stop();
}

void QDeviceInfoPrivate::onTimeout()
{
    if (watchThermalState) {
        QDeviceInfo::ThermalState newState = getThermalState();
        if (newState != currentThermalState) {
            currentThermalState = newState;
            Q_EMIT thermalStateChanged(currentThermalState);
        }
    }
}

QDeviceInfo::ThermalState QDeviceInfoPrivate::getThermalState()
{
    QDeviceInfo::ThermalState state = QDeviceInfo::UnknownThermal;

    const QString hwmonRoot(QString::fromAscii("/sys/class/hwmon/"));
    const QStringList hwmonDirs(QDir(hwmonRoot).entryList(QStringList() << QString::fromAscii("hwmon*")));
    foreach (const QString &dir, hwmonDirs) {
        int index = 1;
        const QString input(hwmonRoot + dir + QDir::separator() + QString::fromAscii("temp%1_input"));
        const QString critical(hwmonRoot + dir + QDir::separator() + QString::fromAscii("temp%1_crit"));
        const QString emergency(hwmonRoot + dir + QDir::separator() + QString::fromAscii("temp%1_emergency"));
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
