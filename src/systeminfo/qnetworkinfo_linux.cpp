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

#include "qnetworkinfo_linux_p.h"

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>

#if !defined(QT_NO_BLUEZ)
#include <bluetooth/bluetooth.h>
#include <bluetooth/bnep.h>
#endif // QT_NO_BLUEZ

#include <math.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>

QT_BEGIN_NAMESPACE

static const QString BLUETOOTH_SYSFS_PATH("/sys/class/bluetooth/");
static const QString NETWORK_SYSFS_PATH("/sys/class/net/");

QNetworkInfoPrivate::QNetworkInfoPrivate(QNetworkInfo *parent)
    : q_ptr(parent)
{
}

int QNetworkInfoPrivate::networkSignalStrength(QNetworkInfo::NetworkMode mode)
{
    switch(mode) {
    case QNetworkInfo::WlanMode: {
        QFile file("/proc/net/wireless");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return -1;

        QTextStream in(&file);
        QString interfaceName = interfaceForMode(QNetworkInfo::WlanMode).name();
        QString line = in.readLine();
        while (!line.isNull()) {
            if (line.left(6).contains(interfaceName)) {
                QString token = line.section(" ", 4, 5).simplified();
                token.chop(1);
                bool ok;
                int signalStrength = (int)rint((log(token.toInt(&ok)) / log(92)) * 100.0);
                if (ok)
                    return signalStrength;
                else
                    return -1;
            }
            line = in.readLine();
        }

        break;
    }

    case QNetworkInfo::EthernetMode:
        if (networkStatus(QNetworkInfo::EthernetMode) == QNetworkInfo::Connected)
            return 100;
        else
            return -1;

//    case QNetworkInfo::BluetoothMode:
//    case QNetworkInfo::GsmMode:
//    case QNetworkInfo::CdmaMode:
//    case QNetworkInfo::WcdmaMode:
//    case QNetworkInfo::WimaxMode:
//    case QNetworkInfo::LteMode:
    default:
        break;
    };

    return -1;
}

QNetworkInfo::CellDataTechnology QNetworkInfoPrivate::currentCellDataTechnology(int sim)
{
    Q_UNUSED(sim)
    return QNetworkInfo::UnknownDataTechnology;
}

QNetworkInfo::NetworkMode QNetworkInfoPrivate::currentNetworkMode()
{
    if (networkStatus(QNetworkInfo::EthernetMode) == QNetworkInfo::Connected)
        return QNetworkInfo::EthernetMode;
    else if (networkStatus(QNetworkInfo::WlanMode) == QNetworkInfo::Connected)
        return QNetworkInfo::WlanMode;
    else if (networkStatus(QNetworkInfo::BluetoothMode) == QNetworkInfo::Connected)
        return QNetworkInfo::BluetoothMode;
    else if (networkStatus(QNetworkInfo::WimaxMode) == QNetworkInfo::Connected)
        return QNetworkInfo::WimaxMode;
    else if (networkStatus(QNetworkInfo::LteMode) == QNetworkInfo::Connected)
        return QNetworkInfo::LteMode;
    else if (networkStatus(QNetworkInfo::WcdmaMode) == QNetworkInfo::Connected)
        return QNetworkInfo::WcdmaMode;
    else if (networkStatus(QNetworkInfo::CdmaMode) == QNetworkInfo::Connected)
        return QNetworkInfo::GsmMode;
    else if (networkStatus(QNetworkInfo::GsmMode) == QNetworkInfo::Connected)
        return QNetworkInfo::GsmMode;
    else
        return QNetworkInfo::UnknownMode;
}

QNetworkInfo::NetworkStatus QNetworkInfoPrivate::networkStatus(QNetworkInfo::NetworkMode mode)
{
    switch(mode) {
    case QNetworkInfo::WlanMode: {
        const QStringList dirs = QDir(NETWORK_SYSFS_PATH).entryList(QStringList() << "wlan*");
        if (dirs.size() == 0)
            return QNetworkInfo::UnknownStatus;
        foreach (const QString &dir, dirs) {
            QFile carrier(NETWORK_SYSFS_PATH + dir + "/carrier");
            if (carrier.open(QIODevice::ReadOnly)) {
                char state;
                if (carrier.read(&state, 1) == 1 && state == '1')
                    return QNetworkInfo::Connected;
            }
        }
        return QNetworkInfo::NoNetworkAvailable;
    }

    case QNetworkInfo::EthernetMode: {
        const QStringList dirs = QDir(NETWORK_SYSFS_PATH).entryList(QStringList() << "eth*" << "usb*");
        if (dirs.size() == 0)
            return QNetworkInfo::UnknownStatus;
        foreach (const QString &dir, dirs) {
            QFile carrier(NETWORK_SYSFS_PATH + dir + "/carrier");
            if (carrier.open(QIODevice::ReadOnly)) {
                char state;
                if (carrier.read(&state, 1) == 1 && state == '1')
                    return QNetworkInfo::Connected;
            }
        }
        return QNetworkInfo::NoNetworkAvailable;
    }

    case QNetworkInfo::BluetoothMode: {
#if !defined(QT_NO_BLUEZ)
        int ctl = socket(PF_BLUETOOTH, SOCK_RAW, BTPROTO_BNEP);
        if (ctl < 0)
            return QNetworkInfo::UnknownStatus;

        struct bnep_conninfo info[36];
        struct bnep_connlist_req req;

        req.ci = info;
        req.cnum = 36;

        if (ioctl(ctl, BNEPGETCONNLIST, &req) < 0)
            return QNetworkInfo::UnknownStatus;

        for (uint j = 0; j< req.cnum; j++) {
            if (info[j].state == BT_CONNECTED)
                return QNetworkInfo::Connected;
        }

        close(ctl);
#endif // QT_NO_BLUEZ

        return QNetworkInfo::UnknownStatus;
    }

//    case QNetworkInfo::GsmMode:
//    case QNetworkInfo::CdmaMode:
//    case QNetworkInfo::WcdmaMode:
//    case QNetworkInfo::WimaxMode:
//    case QNetworkInfo::LteMode:
    default:
        break;
    };

    return QNetworkInfo::UnknownStatus;
}

QNetworkInterface QNetworkInfoPrivate::interfaceForMode(QNetworkInfo::NetworkMode mode)
{
    switch(mode) {
    case QNetworkInfo::WlanMode: {
        const QStringList dirs = QDir(NETWORK_SYSFS_PATH).entryList(QStringList() << "wlan*");
        foreach (const QString &dir, dirs) {
            QNetworkInterface interface = QNetworkInterface::interfaceFromName(dir);
            if (interface.isValid())
                return interface;
        }
        break;
    }

    case QNetworkInfo::EthernetMode: {
        const QStringList dirs = QDir(NETWORK_SYSFS_PATH).entryList(QStringList() << "eth*" << "usb*");
        foreach (const QString &dir, dirs) {
            QNetworkInterface interface = QNetworkInterface::interfaceFromName(dir);
            if (interface.isValid())
                return interface;
        }
        break;
    }

//    case QNetworkInfo::BluetoothMode:
//    case QNetworkInfo::GsmMode:
//    case QNetworkInfo::CdmaMode:
//    case QNetworkInfo::WcdmaMode:
//    case QNetworkInfo::WimaxMode:
//    case QNetworkInfo::LteMode:
    default:
        break;
    };

    return QNetworkInterface();
}

QString QNetworkInfoPrivate::cellId(int sim)
{
    Q_UNUSED(sim)
    return QString();
}

QString QNetworkInfoPrivate::currentMobileCountryCode(int sim)
{
    Q_UNUSED(sim)
    return QString();
}

QString QNetworkInfoPrivate::currentMobileNetworkCode(int sim)
{
    Q_UNUSED(sim)
    return QString();
}

QString QNetworkInfoPrivate::homeMobileCountryCode(int sim)
{
    Q_UNUSED(sim)
    return QString();
}

QString QNetworkInfoPrivate::homeMobileNetworkCode(int sim)
{
    Q_UNUSED(sim)
    return QString();
}

QString QNetworkInfoPrivate::imsi(int sim)
{
    Q_UNUSED(sim)
    return QString();
}

QString QNetworkInfoPrivate::locationAreaCode(int sim)
{
    Q_UNUSED(sim)
    return QString();
}

QString QNetworkInfoPrivate::macAddress(QNetworkInfo::NetworkMode mode)
{
    switch(mode) {
    case QNetworkInfo::WlanMode: {
        const QStringList dirs = QDir(NETWORK_SYSFS_PATH).entryList(QStringList() << "wlan*");
        foreach (const QString &dir, dirs) {
            QFile carrier(NETWORK_SYSFS_PATH + dir + "/address");
            if (carrier.open(QIODevice::ReadOnly))
                return carrier.readAll().simplified();
        }
        break;
    }

    case QNetworkInfo::EthernetMode: {
        const QStringList dirs = QDir(NETWORK_SYSFS_PATH).entryList(QStringList() << "eth*" << "usb*");
        foreach (const QString &dir, dirs) {
            QFile carrier(NETWORK_SYSFS_PATH + dir + "/address");
            if (carrier.open(QIODevice::ReadOnly))
                return carrier.readAll().simplified();
        }
        break;
    }

    case QNetworkInfo::BluetoothMode: {
        const QStringList dirs = QDir(BLUETOOTH_SYSFS_PATH).entryList(QStringList() << "*");
        foreach (const QString &dir, dirs) {
            QFile carrier(BLUETOOTH_SYSFS_PATH + dir + "/address");
            if (carrier.open(QIODevice::ReadOnly))
                return carrier.readAll().simplified();
        }
        break;
    }

//    case QNetworkInfo::GsmMode:
//    case QNetworkInfo::CdmaMode:
//    case QNetworkInfo::WcdmaMode:
//    case QNetworkInfo::WimaxMode:
//    case QNetworkInfo::LteMode:
    default:
        break;
    };

    return QString();
}

QString QNetworkInfoPrivate::networkName(QNetworkInfo::NetworkMode mode)
{
    switch(mode) {
    case QNetworkInfo::WlanMode: {
        const QStringList dirs = QDir(NETWORK_SYSFS_PATH).entryList(QStringList() << "wlan*");
        foreach (const QString &dir, dirs) {
            int sock = socket(PF_INET, SOCK_DGRAM, 0);
            if (sock > 0) {
                char buffer[IW_ESSID_MAX_SIZE + 1];
                iwreq iwInfo;

                iwInfo.u.essid.pointer = (caddr_t)&buffer;
                iwInfo.u.essid.length = IW_ESSID_MAX_SIZE + 1;
                iwInfo.u.essid.flags = 0;

                strncpy(iwInfo.ifr_name, dir.toLocal8Bit().data(), IFNAMSIZ);

                if (ioctl(sock, SIOCGIWESSID, &iwInfo) == 0) {
                    close(sock);
                    return (const char *)iwInfo.u.essid.pointer;
                }

                close(sock);
            }
        }
        break;
    }

    case QNetworkInfo::EthernetMode: {
        char domainName[64];
        if (getdomainname(domainName, 64) == 0) {
            if (strcmp(domainName, "(none)") != 0)
                return domainName;
        }
        break;
    }

    case QNetworkInfo::BluetoothMode: {
        const QStringList dirs = QDir(BLUETOOTH_SYSFS_PATH).entryList(QStringList() << "*");
        foreach (const QString &dir, dirs) {
            QFile carrier(BLUETOOTH_SYSFS_PATH + dir + "/name");
            if (carrier.open(QIODevice::ReadOnly))
                return carrier.readAll().simplified();
        }
        break;
    }

//    case QNetworkInfo::GsmMode:
//    case QNetworkInfo::CdmaMode:
//    case QNetworkInfo::WcdmaMode:
//    case QNetworkInfo::WimaxMode:
//    case QNetworkInfo::LteMode:
    default:
        break;
    };

    return QString();
}

QT_END_NAMESPACE
