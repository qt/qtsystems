/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystemKit module of the Qt Toolkit.
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

#include <QtCore/qdir.h>
#include <QtCore/qtextstream.h>

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

QDeviceInfoPrivate::QDeviceInfoPrivate(QDeviceInfo *parent)
    : q_ptr(parent)
{
}

bool QDeviceInfoPrivate::hasFeature(QDeviceInfo::Feature feature)
{
    switch (feature) {
    case QDeviceInfo::Bluetooth:
        if (QDir("/sys/class/bluetooth/").entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() > 0)
            return true;
        return false;

    case QDeviceInfo::Camera: {
        const QString devfsPath("/dev/");
        const QStringList dirs = QDir(devfsPath).entryList(QStringList() << "video*", QDir::System);
        foreach (const QString &dir, dirs) {
            int fd = open((devfsPath + dir).toStdString().c_str(), O_RDWR);
            if (fd == -1)
                continue;
            struct v4l2_capability capability;
            memset(&capability, 0, sizeof(struct v4l2_capability));
            if (ioctl(fd, VIDIOC_QUERYCAP, &capability) != -1
                && (capability.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE) {
                close(fd);
                return true;
            }
            close(fd);
        }
        return false;
    }

    case QDeviceInfo::FmRadio:
        if (QDir("/sys/class/video4linux/").entryList(QStringList() << "radio*").size() > 0)
            return true;
        return false;

    case QDeviceInfo::FmTransmitter: {
        const QString devfsPath("/dev/");
        const QStringList dirs = QDir(devfsPath).entryList(QStringList() << "radio*", QDir::System);
        foreach (const QString &dir, dirs) {
            int fd = open((devfsPath + dir).toStdString().c_str(), O_RDWR);
            if (fd == -1)
                continue;
            struct v4l2_capability capability;
            memset(&capability, 0, sizeof(struct v4l2_capability));
            if (ioctl(fd, VIDIOC_QUERYCAP, &capability) != -1
                && (capability.capabilities & (V4L2_CAP_RADIO | V4L2_CAP_MODULATOR)) == (V4L2_CAP_RADIO | V4L2_CAP_MODULATOR)) {
                close(fd);
                return true;
            }
            close(fd);
        }
        return false;
    }
    case QDeviceInfo::Infrared:
        // TODO: find the kernel interface for this
        return false;

    case QDeviceInfo::Led:
        if (QDir("/sys/class/leds/").entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() > 0)
            return true;
        return false;

    case QDeviceInfo::MemoryCard:
        if (QDir("/sys/class/mmc_host/").entryList(QStringList() << "mmc*").size() > 0)
            return true;
        return false;

    case QDeviceInfo::Usb:
        if (QDir("/sys/bus/usb/devices/").entryList(QStringList() << "usb*").size() > 0)
            return true;
        return false;

    case QDeviceInfo::Vibration:
        // TODO: find the kernel interface for this
        return false;

    case QDeviceInfo::Wlan:
        if (QDir("/sys/class/ieee80211/").entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() > 0)
            return true;
        return false;

    case QDeviceInfo::Sim:
        // TODO: find the kernel interface for this
        return false;

    case QDeviceInfo::Positioning:
        // TODO: find the kernel interface for this
        return false;

    case QDeviceInfo::VideoOut: {
        const QString devfsPath("/dev/");
        const QStringList dirs = QDir(devfsPath).entryList(QStringList() << "video*", QDir::System);
        foreach (const QString &dir, dirs) {
            int fd = open((devfsPath + dir).toStdString().c_str(), O_RDWR);
            if (fd == -1)
                continue;
            struct v4l2_capability capability;
            memset(&capability, 0, sizeof(struct v4l2_capability));
            if (ioctl(fd, VIDIOC_QUERYCAP, &capability) != -1
                && (capability.capabilities & V4L2_CAP_VIDEO_OUTPUT) == V4L2_CAP_VIDEO_OUTPUT) {
                close(fd);
                return true;
            }
            close(fd);
        }
        return false;
    }

    case QDeviceInfo::Haptics:
        if (QDir("/sys/class/haptic/").entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() > 0)
            return true;
        return false;
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
    return QDeviceInfo::UnknownThermal;
}

QByteArray QDeviceInfoPrivate::uniqueDeviceID()
{
    return QByteArray();
}

QString QDeviceInfoPrivate::imei()
{
    return QString();
}

QString QDeviceInfoPrivate::manufacturer()
{
    QFile vendor("/sys/devices/virtual/dmi/id/board_vendor");
    if (vendor.open(QIODevice::ReadOnly))
        return vendor.readAll().simplified();

    return QString();
}

QString QDeviceInfoPrivate::model()
{
    QFile cpuInfo("/proc/cpuinfo");
    if (cpuInfo.open(QIODevice::ReadOnly)) {
        QTextStream textStream(&cpuInfo);
        while (!textStream.atEnd()) {
            QString line = textStream.readLine();
            if (line.contains("model name"))
                return line.split(":").at(1).simplified();
        }
    }

    return QString();
}

QString QDeviceInfoPrivate::productName()
{
    QFile lsbRelease("/etc/lsb-release");
    if (lsbRelease.open(QIODevice::ReadOnly)) {
        QTextStream textStream(&lsbRelease);
        while (!textStream.atEnd()) {
            QString line = textStream.readLine();
            if (line.contains("DISTRIB_DESCRIPTION"))
                return line.split("=").at(1).simplified();
        }
    }

    return QString();
}

QString QDeviceInfoPrivate::version(QDeviceInfo::Version type)
{
    switch(type) {
    case QDeviceInfo::Os: {
        QFile os("/etc/issue");
        if (os.open(QIODevice::ReadOnly)) {
            QByteArray content = os.readAll();
            if (!content.isEmpty()) {
                QList<QByteArray> list(content.split(' '));
                bool ok = false;
                foreach (const QByteArray &field, list) {
                    field.toDouble(&ok);
                    if (ok)
                        return field;
                }
            }
        }
        break;
    }

    case QDeviceInfo::Firmware: {
        QFile firmware("/proc/sys/kernel/osrelease");
        if (firmware.open(QIODevice::ReadOnly))
            return firmware.readAll().simplified();
        break;
    }

    default:
        break;
    };

    return QString();
}
