/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qnetworkinfo_win_p.h"

#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

QNetworkInfoPrivate::QNetworkInfoPrivate(QNetworkInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
{
}

int QNetworkInfoPrivate::networkInterfaceCount(QNetworkInfo::NetworkMode mode)
{
    switch (mode) {
    case QNetworkInfo::WlanMode:
    case QNetworkInfo::EthernetMode:
    case QNetworkInfo::BluetoothMode:

//    case QNetworkInfo::GsmMode:
//    case QNetworkInfo::CdmaMode:
//    case QNetworkInfo::WcdmaMode:
//    case QNetworkInfo::WimaxMode:
//    case QNetworkInfo::LteMode:
//    case QNetworkInfo::TdscdmaMode:
    default:
        break;
    };

    return -1;
}

int QNetworkInfoPrivate::networkSignalStrength(QNetworkInfo::NetworkMode mode, int interface)
{
    Q_UNUSED(interface)

    switch (mode) {
    case QNetworkInfo::WlanMode:
    case QNetworkInfo::EthernetMode:
    case QNetworkInfo::BluetoothMode:

//    case QNetworkInfo::GsmMode:
//    case QNetworkInfo::CdmaMode:
//    case QNetworkInfo::WcdmaMode:
//    case QNetworkInfo::WimaxMode:
//    case QNetworkInfo::LteMode:
//    case QNetworkInfo::TdscdmaMode:
    default:
        break;
    };

    return -1;
}

QNetworkInfo::CellDataTechnology QNetworkInfoPrivate::currentCellDataTechnology(int interface)
{
    Q_UNUSED(interface)
    return QNetworkInfo::UnknownDataTechnology;
}

QNetworkInfo::NetworkMode QNetworkInfoPrivate::currentNetworkMode()
{
    // TODO multiple-interface support
    if (networkStatus(QNetworkInfo::EthernetMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::EthernetMode;
    else if (networkStatus(QNetworkInfo::WlanMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::WlanMode;
    else if (networkStatus(QNetworkInfo::BluetoothMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::BluetoothMode;
    else if (networkStatus(QNetworkInfo::WimaxMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::WimaxMode;
    else if (networkStatus(QNetworkInfo::LteMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::LteMode;
    else if (networkStatus(QNetworkInfo::WcdmaMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::WcdmaMode;
    else if (networkStatus(QNetworkInfo::CdmaMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::CdmaMode;
    else if (networkStatus(QNetworkInfo::GsmMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::GsmMode;
    else if (networkStatus(QNetworkInfo::TdscdmaMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::TdscdmaMode;
    else if (networkStatus(QNetworkInfo::WimaxMode, 0) == QNetworkInfo::Roaming)
        return QNetworkInfo::WimaxMode;
    else if (networkStatus(QNetworkInfo::LteMode, 0) == QNetworkInfo::Roaming)
        return QNetworkInfo::LteMode;
    else if (networkStatus(QNetworkInfo::WcdmaMode, 0) == QNetworkInfo::Roaming)
        return QNetworkInfo::WcdmaMode;
    else if (networkStatus(QNetworkInfo::CdmaMode, 0) == QNetworkInfo::Roaming)
        return QNetworkInfo::CdmaMode;
    else if (networkStatus(QNetworkInfo::GsmMode, 0) == QNetworkInfo::Roaming)
        return QNetworkInfo::GsmMode;
    else if (networkStatus(QNetworkInfo::TdscdmaMode, 0) == QNetworkInfo::Roaming)
        return QNetworkInfo::TdscdmaMode;
    else
        return QNetworkInfo::UnknownMode;
}

QNetworkInfo::NetworkStatus QNetworkInfoPrivate::networkStatus(QNetworkInfo::NetworkMode mode, int interface)
{
    Q_UNUSED(interface)

    switch (mode) {
    case QNetworkInfo::WlanMode:
    case QNetworkInfo::EthernetMode:
    case QNetworkInfo::BluetoothMode:

//    case QNetworkInfo::GsmMode:
//    case QNetworkInfo::CdmaMode:
//    case QNetworkInfo::WcdmaMode:
//    case QNetworkInfo::WimaxMode:
//    case QNetworkInfo::LteMode:
//    case QNetworkInfo::TdscdmaMode:
        break;

    default:
        break;
    };

    return QNetworkInfo::UnknownStatus;
}

#ifndef QT_NO_NETWORKINTERFACE
QNetworkInterface QNetworkInfoPrivate::interfaceForMode(QNetworkInfo::NetworkMode mode, int interface)
{
    Q_UNUSED(interface)

    switch (mode) {
    case QNetworkInfo::WlanMode:
    case QNetworkInfo::EthernetMode:

//    case QNetworkInfo::BluetoothMode:
//    case QNetworkInfo::GsmMode:
//    case QNetworkInfo::CdmaMode:
//    case QNetworkInfo::WcdmaMode:
//    case QNetworkInfo::WimaxMode:
//    case QNetworkInfo::LteMode:
//    case QNetworkInfo::TdscdmaMode:
    default:
        break;
    };

    return QNetworkInterface();
}
#endif // QT_NO_NETWORKINTERFACE

QString QNetworkInfoPrivate::cellId(int interface)
{
    Q_UNUSED(interface)
    return QString();
}

QString QNetworkInfoPrivate::currentMobileCountryCode(int interface)
{
    Q_UNUSED(interface)
    return QString();
}

QString QNetworkInfoPrivate::currentMobileNetworkCode(int interface)
{
    Q_UNUSED(interface)
    return QString();
}

QString QNetworkInfoPrivate::homeMobileCountryCode(int interface)
{
    Q_UNUSED(interface)
    return QString();
}

QString QNetworkInfoPrivate::homeMobileNetworkCode(int interface)
{
    Q_UNUSED(interface)
    return QString();
}

QString QNetworkInfoPrivate::imsi(int interface)
{
    Q_UNUSED(interface)
    return QString();
}

QString QNetworkInfoPrivate::locationAreaCode(int interface)
{
    Q_UNUSED(interface)
    return QString();
}

QString QNetworkInfoPrivate::macAddress(QNetworkInfo::NetworkMode mode, int interface)
{
    Q_UNUSED(interface)

    switch (mode) {
    case QNetworkInfo::WlanMode:
    case QNetworkInfo::EthernetMode:
    case QNetworkInfo::BluetoothMode:

//    case QNetworkInfo::GsmMode:
//    case QNetworkInfo::CdmaMode:
//    case QNetworkInfo::WcdmaMode:
//    case QNetworkInfo::WimaxMode:
//    case QNetworkInfo::LteMode:
//    case QNetworkInfo::TdscdmaMode:
    default:
        break;
    };

    return QString();
}

QString QNetworkInfoPrivate::networkName(QNetworkInfo::NetworkMode mode, int interface)
{
    Q_UNUSED(interface)

    switch (mode) {
    case QNetworkInfo::WlanMode:
    case QNetworkInfo::EthernetMode:
    case QNetworkInfo::BluetoothMode:

//    case QNetworkInfo::GsmMode:
//    case QNetworkInfo::CdmaMode:
//    case QNetworkInfo::WcdmaMode:
//    case QNetworkInfo::WimaxMode:
//    case QNetworkInfo::LteMode:
//    case QNetworkInfo::TdscdmaMode:
    default:
        break;
    };

    return QString();
}

void QNetworkInfoPrivate::connectNotify(const QMetaMethod &signal)
{
    Q_UNUSED(signal)
}

void QNetworkInfoPrivate::disconnectNotify(const QMetaMethod &signal)
{
    Q_UNUSED(signal)
}

QT_END_NAMESPACE
