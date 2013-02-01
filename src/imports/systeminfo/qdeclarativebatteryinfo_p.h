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

#ifndef QDECLARATIVEBATTERYINFO_P_H
#define QDECLARATIVEBATTERYINFO_P_H

#include <qbatteryinfo.h>

QT_BEGIN_NAMESPACE

class QDeclarativeBatteryInfo : public QObject
{
    Q_OBJECT

    Q_ENUMS(ChargerType)
    Q_ENUMS(ChargingState)
    Q_ENUMS(EnergyUnit)
    Q_ENUMS(BatteryStatus)

    Q_PROPERTY(bool monitorBatteryCount READ monitorBatteryCount WRITE setMonitorBatteryCount NOTIFY monitorBatteryCountChanged)
    Q_PROPERTY(bool monitorChargerType READ monitorChargerType WRITE setMonitorChargerType NOTIFY monitorChargerTypeChanged)
    Q_PROPERTY(bool monitorCurrentFlow READ monitorCurrentFlow WRITE setMonitorCurrentFlow NOTIFY monitorCurrentFlowChanged)
    Q_PROPERTY(bool monitorRemainingCapacity READ monitorRemainingCapacity WRITE setMonitorRemainingCapacity NOTIFY monitorRemainingCapacityChanged)
    Q_PROPERTY(bool monitorRemainingChargingTime READ monitorRemainingChargingTime WRITE setMonitorRemainingChargingTime NOTIFY monitorRemainingChargingTimeChanged)
    Q_PROPERTY(bool monitorVoltage READ monitorVoltage WRITE setMonitorVoltage NOTIFY monitorVoltageChanged)
    Q_PROPERTY(bool monitorChargingState READ monitorChargingState WRITE setMonitorChargingState NOTIFY monitorChargingStateChanged)
    Q_PROPERTY(bool monitorBatteryStatus READ monitorBatteryStatus WRITE setMonitorBatteryStatus NOTIFY monitorBatteryStatusChanged)

    Q_PROPERTY(int batteryCount READ batteryCount NOTIFY batteryCountChanged)
    Q_PROPERTY(ChargerType chargerType READ chargerType NOTIFY chargerTypeChanged)
    Q_PROPERTY(EnergyUnit energyUnit READ energyUnit)

public:
    enum ChargerType {
        UnknownCharger = QBatteryInfo::UnknownCharger,
        WallCharger = QBatteryInfo::WallCharger,
        USBCharger = QBatteryInfo::USBCharger,
        VariableCurrentCharger = QBatteryInfo::VariableCurrentCharger
    };

    enum ChargingState {
        UnknownChargingState = QBatteryInfo::UnknownChargingState,
        NotCharging = QBatteryInfo::NotCharging,
        Charging = QBatteryInfo::Charging,
        Discharging = QBatteryInfo::Discharging,
        Full = QBatteryInfo::Full
    };

    enum EnergyUnit {
        UnitUnknown = QBatteryInfo::UnitUnknown,
        UnitmAh = QBatteryInfo::UnitmAh,
        UnitmWh = QBatteryInfo::UnitmWh
    };

    enum BatteryStatus {
        BatteryStatusUnknown = QBatteryInfo::BatteryStatusUnknown,
        BatteryEmpty = QBatteryInfo::BatteryEmpty,
        BatteryLow = QBatteryInfo::BatteryLow,
        BatteryOk = QBatteryInfo::BatteryOk,
        BatteryFull = QBatteryInfo::BatteryFull
    };

    QDeclarativeBatteryInfo(QObject *parent = 0);
    virtual ~QDeclarativeBatteryInfo();

    bool monitorBatteryCount() const;
    void setMonitorBatteryCount(bool monitor);
    int batteryCount() const;

    bool monitorChargerType() const;
    void setMonitorChargerType(bool monitor);
    ChargerType chargerType() const;

    bool monitorCurrentFlow() const;
    void setMonitorCurrentFlow(bool monitor);
    Q_INVOKABLE int currentFlow(int battery) const;

    bool monitorRemainingCapacity() const;
    void setMonitorRemainingCapacity(bool monitor);
    Q_INVOKABLE int remainingCapacity(int battery) const;

    bool monitorRemainingChargingTime() const;
    void setMonitorRemainingChargingTime(bool monitor);
    Q_INVOKABLE int remainingChargingTime(int battery) const;

    bool monitorVoltage() const;
    void setMonitorVoltage(bool monitor);
    Q_INVOKABLE int voltage(int battery) const;

    bool monitorChargingState() const;
    void setMonitorChargingState(bool monitor);
    Q_INVOKABLE int chargingState(int battery) const;

    EnergyUnit energyUnit() const;
    Q_INVOKABLE int maximumCapacity(int battery) const;

    bool monitorBatteryStatus() const;
    void setMonitorBatteryStatus(bool monitor);
    Q_INVOKABLE int batteryStatus(int battery) const;

Q_SIGNALS:
    void monitorBatteryCountChanged();
    void monitorChargerTypeChanged();
    void monitorChargingStateChanged();
    void monitorCurrentFlowChanged();
    void monitorRemainingCapacityChanged();
    void monitorRemainingChargingTimeChanged();
    void monitorVoltageChanged();
    void monitorBatteryStatusChanged();

    void batteryCountChanged(int count);
    void chargerTypeChanged(int type);
    void chargingStateChanged(int battery, int state);
    void currentFlowChanged(int battery, int flow);
    void remainingCapacityChanged(int battery, int capacity);
    void remainingChargingTimeChanged(int battery, int seconds);
    void voltageChanged(int battery, int voltage);
    void batteryStatusChanged(int battery, int status);

private Q_SLOTS:
    void _q_chargerTypeChanged(QBatteryInfo::ChargerType type);
    void _q_chargingStateChanged(int battery, QBatteryInfo::ChargingState state);
    void _q_batteryStatusChanged(int battery, QBatteryInfo::BatteryStatus status);

private:
    QBatteryInfo *batteryInfo;

    bool isMonitorBatteryCount;
    bool isMonitorChargerType;
    bool isMonitorChargingState;
    bool isMonitorCurrentFlow;
    bool isMonitorRemainingCapacity;
    bool isMonitorRemainingChargingTime;
    bool isMonitorVoltage;
    bool isMonitorBatteryStatus;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEBATTERYINFO_P_H
