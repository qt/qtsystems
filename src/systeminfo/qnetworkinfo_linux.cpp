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

#if !defined(QT_NO_OFONO)
#include "qofonowrapper_p.h"
#endif // QT_NO_OFONO

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>

#if !defined(QT_NO_BLUEZ)
#include <bluetooth/bluetooth.h>
#include <bluetooth/bnep.h>
#endif // QT_NO_BLUEZ

#include <math.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/wireless.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_OFONO)
Q_GLOBAL_STATIC(QOfonoWrapper, ofonoWrapper)
#endif // QT_NO_OFONO

static const QString BLUETOOTH_SYSFS_PATH("/sys/class/bluetooth/");
static const QString NETWORK_SYSFS_PATH("/sys/class/net/");

QNetworkInfoPrivate::QNetworkInfoPrivate(QNetworkInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
{
}

int QNetworkInfoPrivate::networkSignalStrength(QNetworkInfo::NetworkMode mode, int interface)
{
    switch(mode) {
    case QNetworkInfo::WlanMode: {
        QFile file("/proc/net/wireless");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return -1;

        QTextStream in(&file);
        QString interfaceName = interfaceForMode(QNetworkInfo::WlanMode, interface).name();
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
        if (networkStatus(QNetworkInfo::EthernetMode, interface) == QNetworkInfo::Connected)
            return 100;
        else
            return -1;

    case QNetworkInfo::GsmMode:
    case QNetworkInfo::CdmaMode:
    case QNetworkInfo::WcdmaMode:
    case QNetworkInfo::WimaxMode:
    case QNetworkInfo::LteMode:
#if !defined(QT_NO_OFONO)
        if (ofonoWrapper()->isOfonoAvailable()) {
            QString modem = ofonoWrapper()->allModems().at(interface);
            if (!modem.isEmpty())
                return ofonoWrapper()->signalStrength(modem);
        }
#endif // QT_NO_OFONO
        break;

//    case QNetworkInfo::BluetoothMode:
    default:
        break;
    };

    return -1;
}

QNetworkInfo::CellDataTechnology QNetworkInfoPrivate::currentCellDataTechnology(int interface)
{
#if !defined(QT_NO_OFONO)
    if (ofonoWrapper()->isOfonoAvailable()) {
        QString modem = ofonoWrapper()->allModems().at(interface);
        if (!modem.isEmpty())
            return ofonoWrapper()->currentCellDataTechnology(modem);
    }
#else
    Q_UNUSED(interface)
#endif
    return QNetworkInfo::UnknownDataTechnology;
}

QNetworkInfo::NetworkMode QNetworkInfoPrivate::currentNetworkMode()
{
    // TODO multiple-interface support
    if (networkStatus(QNetworkInfo::EthernetMode, 0) == QNetworkInfo::Connected)
        return QNetworkInfo::EthernetMode;
    else if (networkStatus(QNetworkInfo::WlanMode, 0) == QNetworkInfo::Connected)
        return QNetworkInfo::WlanMode;
    else if (networkStatus(QNetworkInfo::BluetoothMode, 0) == QNetworkInfo::Connected)
        return QNetworkInfo::BluetoothMode;
    else if (networkStatus(QNetworkInfo::WimaxMode, 0) == QNetworkInfo::Connected)
        return QNetworkInfo::WimaxMode;
    else if (networkStatus(QNetworkInfo::LteMode, 0) == QNetworkInfo::Connected)
        return QNetworkInfo::LteMode;
    else if (networkStatus(QNetworkInfo::WcdmaMode, 0) == QNetworkInfo::Connected)
        return QNetworkInfo::WcdmaMode;
    else if (networkStatus(QNetworkInfo::CdmaMode, 0) == QNetworkInfo::Connected)
        return QNetworkInfo::GsmMode;
    else if (networkStatus(QNetworkInfo::GsmMode, 0) == QNetworkInfo::Connected)
        return QNetworkInfo::GsmMode;
    else
        return QNetworkInfo::UnknownMode;
}

QNetworkInfo::NetworkStatus QNetworkInfoPrivate::networkStatus(QNetworkInfo::NetworkMode mode, int interface)
{
    switch(mode) {
    case QNetworkInfo::WlanMode: {
        const QString dir = QDir(NETWORK_SYSFS_PATH).entryList(QStringList() << "wlan*").at(interface);
        if (dir.isEmpty())
            return QNetworkInfo::UnknownStatus;
        QFile carrier(NETWORK_SYSFS_PATH + dir + "/carrier");
        if (carrier.open(QIODevice::ReadOnly)) {
            char state;
            if (carrier.read(&state, 1) == 1 && state == '1')
                return QNetworkInfo::Connected;
        }
        return QNetworkInfo::NoNetworkAvailable;
    }

    case QNetworkInfo::EthernetMode: {
        const QString dir = QDir(NETWORK_SYSFS_PATH).entryList(QStringList() << "eth*" << "usb*").at(interface);
        if (dir.isEmpty())
            return QNetworkInfo::UnknownStatus;
        QFile carrier(NETWORK_SYSFS_PATH + dir + "/carrier");
        if (carrier.open(QIODevice::ReadOnly)) {
            char state;
            if (carrier.read(&state, 1) == 1 && state == '1')
                return QNetworkInfo::Connected;
        }
        return QNetworkInfo::NoNetworkAvailable;
    }

    case QNetworkInfo::BluetoothMode: {
#if !defined(QT_NO_BLUEZ)
        // TODO multiple-interface support
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

    case QNetworkInfo::GsmMode:
    case QNetworkInfo::CdmaMode:
    case QNetworkInfo::WcdmaMode:
    case QNetworkInfo::WimaxMode:
    case QNetworkInfo::LteMode:
#if !defined(QT_NO_OFONO)
    if (ofonoWrapper()->isOfonoAvailable()) {
        QString modem = ofonoWrapper()->allModems().at(interface);
        if (!modem.isEmpty())
            return ofonoWrapper()->networkStatus(modem);
    }
#endif
        break;

    default:
        break;
    };

    return QNetworkInfo::UnknownStatus;
}

QNetworkInterface QNetworkInfoPrivate::interfaceForMode(QNetworkInfo::NetworkMode mode, int interface)
{
    switch(mode) {
    case QNetworkInfo::WlanMode: {
        const QString dir = QDir(NETWORK_SYSFS_PATH).entryList(QStringList() << "wlan*").at(interface);
        if (dir.isEmpty())
            break;
        QNetworkInterface interface = QNetworkInterface::interfaceFromName(dir);
        if (interface.isValid())
            return interface;
        break;
    }

    case QNetworkInfo::EthernetMode: {
        const QString dir = QDir(NETWORK_SYSFS_PATH).entryList(QStringList() << "eth*" << "usb*").at(interface);
        if (dir.isEmpty())
            break;
        QNetworkInterface interface = QNetworkInterface::interfaceFromName(dir);
        if (interface.isValid())
            return interface;
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

QString QNetworkInfoPrivate::cellId(int interface)
{
#if !defined(QT_NO_OFONO)
    if (ofonoWrapper()->isOfonoAvailable()) {
        QString modem = ofonoWrapper()->allModems().at(interface);
        if (!modem.isEmpty())
            return ofonoWrapper()->cellId(modem);
    }
#else
    Q_UNUSED(interface)
#endif
    return QString();
}

QString QNetworkInfoPrivate::currentMobileCountryCode(int interface)
{
#if !defined(QT_NO_OFONO)
    if (ofonoWrapper()->isOfonoAvailable()) {
        QString modem = ofonoWrapper()->allModems().at(interface);
        if (!modem.isEmpty())
            return ofonoWrapper()->currentMcc(modem);
    }
#else
    Q_UNUSED(interface)
#endif
    return QString();
}

QString QNetworkInfoPrivate::currentMobileNetworkCode(int interface)
{
#if !defined(QT_NO_OFONO)
    if (ofonoWrapper()->isOfonoAvailable()) {
        QString modem = ofonoWrapper()->allModems().at(interface);
        if (!modem.isEmpty())
            return ofonoWrapper()->currentMnc(modem);
    }
#else
    Q_UNUSED(interface)
#endif
    return QString();
}

QString QNetworkInfoPrivate::homeMobileCountryCode(int interface)
{
#if !defined(QT_NO_OFONO)
    if (ofonoWrapper()->isOfonoAvailable()) {
        QString modem = ofonoWrapper()->allModems().at(interface);
        if (!modem.isEmpty())
            return ofonoWrapper()->homeMcc(modem);
    }
#else
    Q_UNUSED(interface)
#endif
    return QString();
}

QString QNetworkInfoPrivate::homeMobileNetworkCode(int interface)
{
#if !defined(QT_NO_OFONO)
    if (ofonoWrapper()->isOfonoAvailable()) {
        QString modem = ofonoWrapper()->allModems().at(interface);
        if (!modem.isEmpty())
            return ofonoWrapper()->homeMnc(modem);
    }
#else
    Q_UNUSED(interface)
#endif
    return QString();
}

QString QNetworkInfoPrivate::imsi(int interface)
{
#if !defined(QT_NO_OFONO)
    if (ofonoWrapper()->isOfonoAvailable()) {
        QString modem = ofonoWrapper()->allModems().at(interface);
        if (!modem.isEmpty())
            return ofonoWrapper()->imsi(modem);
    }
#else
    Q_UNUSED(interface)
#endif
    return QString();
}

QString QNetworkInfoPrivate::locationAreaCode(int interface)
{
#if !defined(QT_NO_OFONO)
    if (ofonoWrapper()->isOfonoAvailable()) {
        QString modem = ofonoWrapper()->allModems().at(interface);
        if (!modem.isEmpty())
            return ofonoWrapper()->lac(modem);
    }
#else
    Q_UNUSED(interface)
#endif
    return QString();
}

QString QNetworkInfoPrivate::macAddress(QNetworkInfo::NetworkMode mode, int interface)
{
    switch(mode) {
    case QNetworkInfo::WlanMode: {
        const QString dir = QDir(NETWORK_SYSFS_PATH).entryList(QStringList() << "wlan*").at(interface);
        if (dir.isEmpty())
            break;
        QFile carrier(NETWORK_SYSFS_PATH + dir + "/address");
        if (carrier.open(QIODevice::ReadOnly))
            return carrier.readAll().simplified();
        break;
    }

    case QNetworkInfo::EthernetMode: {
        const QString dir = QDir(NETWORK_SYSFS_PATH).entryList(QStringList() << "eth*" << "usb*").at(interface);
        if (dir.isEmpty())
            break;
        QFile carrier(NETWORK_SYSFS_PATH + dir + "/address");
        if (carrier.open(QIODevice::ReadOnly))
            return carrier.readAll().simplified();
        break;
    }

    case QNetworkInfo::BluetoothMode: {
        const QString dir = QDir(BLUETOOTH_SYSFS_PATH).entryList(QStringList() << "*").at(interface);
        if (dir.isEmpty())
            break;
        QFile carrier(BLUETOOTH_SYSFS_PATH + dir + "/address");
        if (carrier.open(QIODevice::ReadOnly))
            return carrier.readAll().simplified();
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

QString QNetworkInfoPrivate::networkName(QNetworkInfo::NetworkMode mode, int interface)
{
    switch(mode) {
    case QNetworkInfo::WlanMode: {
        const QString dir = QDir(NETWORK_SYSFS_PATH).entryList(QStringList() << "wlan*").at(interface);
        if (dir.isEmpty())
            break;
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
        break;
    }

    case QNetworkInfo::EthernetMode: {
        // TODO multiple-interface support
        char domainName[64];
        if (getdomainname(domainName, 64) == 0) {
            if (strcmp(domainName, "(none)") != 0)
                return domainName;
        }
        break;
    }

    case QNetworkInfo::BluetoothMode: {
        const QString dir = QDir(BLUETOOTH_SYSFS_PATH).entryList(QStringList() << "*").at(interface);
        if (dir.isEmpty())
            break;
        QFile carrier(BLUETOOTH_SYSFS_PATH + dir + "/name");
        if (carrier.open(QIODevice::ReadOnly))
            return carrier.readAll().simplified();
        break;
    }

    case QNetworkInfo::GsmMode:
    case QNetworkInfo::CdmaMode:
    case QNetworkInfo::WcdmaMode:
    case QNetworkInfo::WimaxMode:
    case QNetworkInfo::LteMode:
#if !defined(QT_NO_OFONO)
        if (ofonoWrapper()->isOfonoAvailable()) {
            QString modem = ofonoWrapper()->allModems().at(interface);
            if (!modem.isEmpty())
                return ofonoWrapper()->operatorName(modem);
        }
#endif // QT_NO_OFONO
        break;

    default:
        break;
    };

    return QString();
}

void QNetworkInfoPrivate::connectNotify(const char *signal)
{
#if !defined(QT_NO_OFONO)
    connect(ofonoWrapper(), signal, this, signal, Qt::UniqueConnection);
#else
    Q_UNUSED(signal)
#endif // QT_NO_OFONO
}

void QNetworkInfoPrivate::disconnectNotify(const char *signal)
{
#if !defined(QT_NO_OFONO)
    disconnect(ofonoWrapper(), signal, this, signal);
#else
    Q_UNUSED(signal)
#endif // QT_NO_OFONO
}

QT_END_NAMESPACE
