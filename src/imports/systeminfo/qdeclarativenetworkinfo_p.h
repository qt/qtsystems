/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDECLARATIVENETWORKINFO_P_H
#define QDECLARATIVENETWORKINFO_P_H

#include <qnetworkinfo.h>

QT_BEGIN_NAMESPACE

class QDeclarativeNetworkInfo : public QObject
{
    Q_OBJECT

    Q_ENUMS(CellDataTechnology)
    Q_ENUMS(NetworkMode)
    Q_ENUMS(NetworkStatus)

    Q_PROPERTY(bool monitorNetworkSignalStrength READ monitorNetworkSignalStrength WRITE setMonitorNetworkSignalStrength NOTIFY monitorNetworkSignalStrengthChanged)
    Q_PROPERTY(bool monitorNetworkStatus READ monitorNetworkStatus WRITE setMonitorNetworkStatus NOTIFY monitorNetworkStatusChanged)
    Q_PROPERTY(bool monitorNetworkName READ monitorNetworkName WRITE setMonitorNetworkName NOTIFY monitorNetworkNameChanged)
    Q_PROPERTY(bool monitorCurrentNetworkMode READ monitorCurrentNetworkMode WRITE setMonitorCurrentNetworkMode NOTIFY monitorCurrentNetworkModeChanged)

    Q_PROPERTY(NetworkMode currentNetworkMode READ currentNetworkMode NOTIFY currentNetworkModeChanged)

    // obsoleted
    Q_PROPERTY(bool monitorNetworkInterfaceCount READ monitorNetworkInterfaceCount WRITE setMonitorNetworkInterfaceCount NOTIFY monitorNetworkInterfaceCountChanged)
    Q_PROPERTY(bool monitorCurrentCellDataTechnology READ monitorCurrentCellDataTechnology WRITE setMonitorCurrentCellDataTechnology NOTIFY monitorCurrentCellDataTechnologyChanged)
    Q_PROPERTY(bool monitorCellId READ monitorCellId WRITE setMonitorCellId NOTIFY monitorCellIdChanged)
    Q_PROPERTY(bool monitorCurrentMobileCountryCode READ monitorCurrentMobileCountryCode WRITE setMonitorCurrentMobileCountryCode NOTIFY monitorCurrentMobileCountryCodeChanged)
    Q_PROPERTY(bool monitorCurrentMobileNetworkCode READ monitorCurrentMobileNetworkCode WRITE setMonitorCurrentMobileNetworkCode NOTIFY monitorCurrentMobileNetworkCodeChanged)
    Q_PROPERTY(bool monitorLocationAreaCode READ monitorLocationAreaCode WRITE setMonitorLocationAreaCode NOTIFY monitorLocationAreaCodeChanged)

public:
    enum CellDataTechnology {
        UnknownDataTechnology = QNetworkInfo::UnknownDataTechnology,
        GprsDataTechnology = QNetworkInfo::GprsDataTechnology,
        EdgeDataTechnology = QNetworkInfo::EdgeDataTechnology,
        UmtsDataTechnology = QNetworkInfo::UmtsDataTechnology,
        HspaDataTechnology = QNetworkInfo::HspaDataTechnology
    };

    enum NetworkMode {
        UnknownMode = QNetworkInfo::UnknownMode,
        GsmMode = QNetworkInfo::GsmMode,
        CdmaMode = QNetworkInfo::CdmaMode,
        WcdmaMode = QNetworkInfo::WcdmaMode,
        WlanMode = QNetworkInfo::WlanMode,
        EthernetMode = QNetworkInfo::EthernetMode,
        BluetoothMode = QNetworkInfo::BluetoothMode,
        WimaxMode = QNetworkInfo::WimaxMode,
        LteMode = QNetworkInfo::LteMode,
        TdscdmaMode = QNetworkInfo::TdscdmaMode
    };

    enum NetworkStatus {
        UnknownStatus = QNetworkInfo::UnknownStatus,
        NoNetworkAvailable = QNetworkInfo::NoNetworkAvailable,
        EmergencyOnly = QNetworkInfo::EmergencyOnly,
        Searching = QNetworkInfo::Searching,
        Busy = QNetworkInfo::Busy,
        Denied = QNetworkInfo::Denied,
        HomeNetwork = QNetworkInfo::HomeNetwork,
        Roaming = QNetworkInfo::Roaming
    };

    QDeclarativeNetworkInfo(QObject *parent = 0);
    virtual ~QDeclarativeNetworkInfo();

    bool monitorCurrentNetworkMode() const;
    void setMonitorCurrentNetworkMode(bool monitor);
    NetworkMode currentNetworkMode() const;

    bool monitorNetworkSignalStrength() const;
    void setMonitorNetworkSignalStrength(bool monitor);
    Q_INVOKABLE int networkSignalStrength(NetworkMode mode, int interface) const;

    bool monitorNetworkInterfaceCount() const;
    void setMonitorNetworkInterfaceCount(bool monitor);
    Q_INVOKABLE int networkInterfaceCount(NetworkMode mode) const;

    bool monitorCurrentCellDataTechnology() const;
    void setMonitorCurrentCellDataTechnology(bool monitor);
    Q_INVOKABLE int currentCellDataTechnology(int interface) const;

    bool monitorNetworkStatus() const;
    void setMonitorNetworkStatus(bool monitor);
    Q_INVOKABLE int networkStatus(NetworkMode mode, int interface) const;

    bool monitorCellId() const;
    void setMonitorCellId(bool monitor);
    Q_INVOKABLE QString cellId(int interface) const;

    bool monitorCurrentMobileCountryCode() const;
    void setMonitorCurrentMobileCountryCode(bool monitor);
    Q_INVOKABLE QString currentMobileCountryCode(int interface) const;

    bool monitorCurrentMobileNetworkCode() const;
    void setMonitorCurrentMobileNetworkCode(bool monitor);
    Q_INVOKABLE QString currentMobileNetworkCode(int interface) const;

    bool monitorLocationAreaCode() const;
    void setMonitorLocationAreaCode(bool monitor);
    Q_INVOKABLE QString locationAreaCode(int interface) const;

    bool monitorNetworkName() const;
    void setMonitorNetworkName(bool monitor);
    Q_INVOKABLE QString networkName(NetworkMode mode, int interface) const;

    Q_INVOKABLE QString homeMobileCountryCode(int interface) const;
    Q_INVOKABLE QString homeMobileNetworkCode(int interface) const;
    Q_INVOKABLE QString imsi(int interface) const;
    Q_INVOKABLE QString macAddress(NetworkMode mode, int interface) const;

Q_SIGNALS:
    void monitorCurrentCellDataTechnologyChanged();
    void monitorCurrentNetworkModeChanged();
    void monitorNetworkSignalStrengthChanged();
    void monitorNetworkInterfaceCountChanged();
    void monitorNetworkStatusChanged();
    void monitorCellIdChanged();
    void monitorCurrentMobileCountryCodeChanged();
    void monitorCurrentMobileNetworkCodeChanged();
    void monitorLocationAreaCodeChanged();
    void monitorNetworkNameChanged();

    void cellIdChanged(int interfaceIndex, const QString &id);
    void currentCellDataTechnologyChanged(int interfaceIndex, int tech);
    void currentMobileCountryCodeChanged(int interfaceIndex, const QString &mcc);
    void currentMobileNetworkCodeChanged(int interfaceIndex, const QString &mnc);
    void currentNetworkModeChanged();
    void locationAreaCodeChanged(int interfaceIndex, const QString &lac);
    void networkInterfaceCountChanged(int mode, int count);
    void networkNameChanged(int mode, int interfaceIndex, const QString &name);
    void networkSignalStrengthChanged(int mode, int interfaceIndex, int strength);
    void networkStatusChanged(int mode, int interfaceIndex, int status);

private Q_SLOTS:
    void _q_currentCellDataTechnologyChanged(int interface, QNetworkInfo::CellDataTechnology tech);
    void _q_networkInterfaceCountChanged(QNetworkInfo::NetworkMode mode, int count);
    void _q_networkSignalStrengthChanged(QNetworkInfo::NetworkMode mode, int interface, int strength);
    void _q_networkStatusChanged(QNetworkInfo::NetworkMode mode, int interface, QNetworkInfo::NetworkStatus status);
    void _q_networkNameChanged(QNetworkInfo::NetworkMode mode, int interface, const QString &name);

private:
    QNetworkInfo *networkInfo;

    bool isMonitorCurrentNetworkMode;
    bool isMonitorNetworkSignalStrength;
    bool isMonitorNetworkInterfaceCount;
    bool isMonitorCurrentCellDataTechnology;
    bool isMonitorNetworkStatus;
    bool isMonitorCellId;
    bool isMonitorCurrentMobileCountryCode;
    bool isMonitorCurrentMobileNetworkCode;
    bool isMonitorLocationAreaCode;
    bool isMonitorNetworkName;
};

QT_END_NAMESPACE

#endif // QDECLARATIVENETWORKINFO_P_H
