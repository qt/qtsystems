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

#ifndef QNETWORKINFO_H
#define QNETWORKINFO_H

#include "qsysteminfo_p.h"
#include <QtCore/qobject.h>
#include <QtNetwork/qnetworkinterface.h>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QNetworkInfoPrivate;

class Q_SYSTEMINFO_EXPORT QNetworkInfo : public QObject
{
    Q_OBJECT

    Q_ENUMS(CellDataTechnology)
    Q_ENUMS(NetworkMode)
    Q_ENUMS(NetworkStatus)

    Q_PROPERTY(QNetworkInfo::NetworkMode currentNetworkMode READ currentNetworkMode NOTIFY currentNetworkModeChanged)

public:
    enum CellDataTechnology {
        UnknownDataTechnology = 0,
        GprsDataTechnology,
        EdgeDataTechnology,
        UmtsDataTechnology,
        HspaDataTechnology
    };

    enum NetworkMode {
        UnknownMode = 0,
        GsmMode,
        CdmaMode,
        WcdmaMode,
        WlanMode,
        EthernetMode,
        BluetoothMode,
        WimaxMode,
        LteMode
    };

    enum NetworkStatus {
        UnknownStatus = 0,
        NoNetworkAvailable,
        EmergencyOnly,
        Searching,
        Busy,
        Connected,
        Denied,
        HomeNetwork,
        Roaming
    };

    QNetworkInfo(QObject *parent = 0);
    virtual ~QNetworkInfo();

    Q_INVOKABLE int networkSignalStrength(QNetworkInfo::NetworkMode mode) const;
    Q_INVOKABLE QNetworkInfo::CellDataTechnology currentCellDataTechnology(int sim) const;
    Q_INVOKABLE QNetworkInfo::NetworkStatus networkStatus(QNetworkInfo::NetworkMode mode) const;
    Q_INVOKABLE QNetworkInterface interfaceForMode(QNetworkInfo::NetworkMode mode) const;
    Q_INVOKABLE QString cellId(int sim) const;
    Q_INVOKABLE QString currentMobileCountryCode(int sim) const;
    Q_INVOKABLE QString currentMobileNetworkCode(int sim) const;
    Q_INVOKABLE QString homeMobileCountryCode(int sim) const;
    Q_INVOKABLE QString homeMobileNetworkCode(int sim) const;
    Q_INVOKABLE QString imsi(int sim) const;
    Q_INVOKABLE QString locationAreaCode(int sim) const;
    Q_INVOKABLE QString macAddress(QNetworkInfo::NetworkMode mode) const;
    Q_INVOKABLE QString networkName(QNetworkInfo::NetworkMode mode) const;

    QNetworkInfo::NetworkMode currentNetworkMode() const;

Q_SIGNALS:
    void cellIdChanged(int sim, const QString &id);
    void currentCellDataTechnologyChanged(int sim, QNetworkInfo::CellDataTechnology tech);
    void currentMobileCountryCodeChanged(int sim, const QString &mcc);
    void currentMobileNetworkCodeChanged(int sim, const QString &mnc);
    void currentNetworkModeChanged(QNetworkInfo::NetworkMode mode);
    void locationAreaCodeChanged(int sim, const QString &lac);
    void networkNameChanged(QNetworkInfo::NetworkMode mode, const QString &name);
    void networkSignalStrengthChanged(QNetworkInfo::NetworkMode mode, int strength);
    void networkStatusChanged(QNetworkInfo::NetworkMode mode, QNetworkInfo::NetworkStatus status);

private:
    Q_DISABLE_COPY(QNetworkInfo)
    QNetworkInfoPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(QNetworkInfo)
};

QT_END_NAMESPACE
QT_END_HEADER

#endif // QNETWORKINFO_H
