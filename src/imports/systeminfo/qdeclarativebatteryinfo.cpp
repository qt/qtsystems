/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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

#include "qdeclarativebatteryinfo_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmlclass BatteryInfo QDeclarativeBatteryInfo
    \inmodule QtSystems
    \ingroup qml-systeminfo
    \brief The BatteryInfo element provides various information of the battery status.
*/

/*!
    \internal
*/
QDeclarativeBatteryInfo::QDeclarativeBatteryInfo(QObject *parent)
    : QObject(parent)
    , batteryInfo(new QBatteryInfo(this))
    , isMonitorBatteryCount(false)
    , isMonitorChargerType(false)
    , isMonitorChargingState(false)
    , isMonitorCurrentFlow(false)
    , isMonitorRemainingCapacity(false)
    , isMonitorRemainingChargingTime(false)
    , isMonitorVoltage(false)
{
}

/*!
    \internal
 */
QDeclarativeBatteryInfo::~QDeclarativeBatteryInfo()
{
}

/*!
    \qmlproperty bool BatteryInfo::monitorBatteryCount

    This property holds whether or not monitor the change of battery counts.
 */
bool QDeclarativeBatteryInfo::monitorBatteryCount() const
{
    return isMonitorBatteryCount;
}

void QDeclarativeBatteryInfo::setMonitorBatteryCount(bool monitor)
{
    if (monitor != isMonitorBatteryCount) {
        isMonitorBatteryCount = monitor;
        if (monitor) {
            connect(batteryInfo, SIGNAL(batteryCountChanged(int)),
                    this, SIGNAL(batteryCountChanged(int)));
        } else {
            disconnect(batteryInfo, SIGNAL(batteryCountChanged(int)),
                       this, SIGNAL(batteryCountChanged(int)));
        }
        emit monitorBatteryCountChanged();
    }
}

/*!
    \qmlproperty int BatteryInfo::batteryCount

    This property holds the number of batteries available, or -1 on error or the information is not
    available.
*/
int QDeclarativeBatteryInfo::batteryCount() const
{
    return batteryInfo->batteryCount();
}

/*!
    \qmlproperty bool BatteryInfo::monitorChargerType

    This property holds whether or not monitor the change of charger type.
 */
bool QDeclarativeBatteryInfo::monitorChargerType() const
{
    return isMonitorChargerType;
}

void QDeclarativeBatteryInfo::setMonitorChargerType(bool monitor)
{
    if (monitor != isMonitorChargerType) {
        isMonitorChargerType = monitor;
        if (monitor) {
            connect(batteryInfo, SIGNAL(chargerTypeChanged(QBatteryInfo::ChargerType)),
                    this, SLOT(_q_chargerTypeChanged(QBatteryInfo::ChargerType)));
        } else {
            disconnect(batteryInfo, SIGNAL(chargerTypeChanged(QBatteryInfo::ChargerType)),
                       this, SLOT(_q_chargerTypeChanged(QBatteryInfo::ChargerType)));
        }
        emit monitorChargerTypeChanged();
    }
}

/*!
    \qmlproperty enum BatteryInfo::chargerType

    This property holds the type of the charger. Possible values are:
    \list
    \o BatteryInfo.UnknownCharger
    \o BatteryInfo.WallCharger
    \o BatteryInfo.USBCharger
    \o BatteryInfo.VariableCurrentCharger
    \endlist
*/
QDeclarativeBatteryInfo::ChargerType QDeclarativeBatteryInfo::chargerType() const
{
    return static_cast<ChargerType>(batteryInfo->chargerType());
}

void QDeclarativeBatteryInfo::_q_chargerTypeChanged(QBatteryInfo::ChargerType type)
{
    emit chargerTypeChanged(static_cast<ChargerType>(type));
}

/*!
    \qmlproperty bool BatteryInfo::monitorCurrentFlow

    This property holds whether or not monitor the flow of batteries.
 */
bool QDeclarativeBatteryInfo::monitorCurrentFlow() const
{
    return isMonitorCurrentFlow;
}

void QDeclarativeBatteryInfo::setMonitorCurrentFlow(bool monitor)
{
    if (monitor != isMonitorCurrentFlow) {
        isMonitorCurrentFlow = monitor;
        if (monitor) {
            connect(batteryInfo, SIGNAL(currentFlowChanged(int,int)),
                    this, SIGNAL(currentFlowChanged(int,int)));
        } else {
            disconnect(batteryInfo, SIGNAL(currentFlowChanged(int,int)),
                       this, SIGNAL(currentFlowChanged(int,int)));
        }
        emit monitorCurrentFlowChanged();
    }
}

/*!
    \qmlmethod int BatteryInfo::currentFlow(int battery)

    Returns the current flow of the \a battery.
*/
int QDeclarativeBatteryInfo::currentFlow(int battery) const
{
    return batteryInfo->currentFlow(battery);
}

/*!
    \qmlproperty bool BatteryInfo::monitorRemainingCapacity

    This property holds whether or not monitor the remaining capacity of batteries.
 */
bool QDeclarativeBatteryInfo::monitorRemainingCapacity() const
{
    return isMonitorRemainingCapacity;
}

void QDeclarativeBatteryInfo::setMonitorRemainingCapacity(bool monitor)
{
    if (monitor != isMonitorRemainingCapacity) {
        isMonitorRemainingCapacity = monitor;
        if (monitor) {
            connect(batteryInfo, SIGNAL(remainingCapacityChanged(int,int)),
                    this, SIGNAL(remainingCapacityChanged(int,int)));
        } else {
            disconnect(batteryInfo, SIGNAL(remainingCapacityChanged(int,int)),
                       this, SIGNAL(remainingCapacityChanged(int,int)));
        }
        emit monitorRemainingCapacityChanged();
    }
}

/*!
    \qmlmethod int BatteryInfo::remainingCapacity(int battery)

    Returns the remaining capacity of the \a battery.
*/
int QDeclarativeBatteryInfo::remainingCapacity(int battery) const
{
    return batteryInfo->remainingCapacity(battery);
}

/*!
    \qmlproperty bool BatteryInfo::monitorRemainingChargingTime

    This property holds whether or not monitor the remaining charging time of batteries.
 */
bool QDeclarativeBatteryInfo::monitorRemainingChargingTime() const
{
    return isMonitorRemainingChargingTime;
}

void QDeclarativeBatteryInfo::setMonitorRemainingChargingTime(bool monitor)
{
    if (monitor != isMonitorRemainingChargingTime) {
        isMonitorRemainingChargingTime = monitor;
        if (monitor) {
            connect(batteryInfo, SIGNAL(remainingChargingTimeChanged(int,int)),
                    this, SIGNAL(remainingChargingTimeChanged(int,int)));
        } else {
            disconnect(batteryInfo, SIGNAL(remainingChargingTimeChanged(int,int)),
                       this, SIGNAL(remainingChargingTimeChanged(int,int)));
        }
        emit monitorRemainingChargingTimeChanged();
    }
}

/*!
    \qmlmethod int BatteryInfo::remainingChargingTime(int battery)

    Returns the remaining charging time of the \a battery.
*/
int QDeclarativeBatteryInfo::remainingChargingTime(int battery) const
{
    return batteryInfo->remainingChargingTime(battery);
}

/*!
    \qmlproperty bool BatteryInfo::monitorVoltage

    This property holds whether or not monitor the voltage of batteries.
 */
bool QDeclarativeBatteryInfo::monitorVoltage() const
{
    return isMonitorVoltage;
}

void QDeclarativeBatteryInfo::setMonitorVoltage(bool monitor)
{
    if (monitor != isMonitorVoltage) {
        isMonitorVoltage = monitor;
        if (monitor) {
            connect(batteryInfo, SIGNAL(voltageChanged(int,int)),
                    this, SIGNAL(voltageChanged(int,int)));
        } else {
            disconnect(batteryInfo, SIGNAL(voltageChanged(int,int)),
                       this, SIGNAL(voltageChanged(int,int)));
        }
        emit monitorVoltageChanged();
    }
}

/*!
    \qmlmethod int BatteryInfo::voltage(int battery)

    Returns the voltage of the \a battery.
*/
int QDeclarativeBatteryInfo::voltage(int battery) const
{
    return batteryInfo->voltage(battery);
}

/*!
    \qmlproperty bool BatteryInfo::monitorChargingState

    This property holds whether or not monitor the change of charging state.
 */
bool QDeclarativeBatteryInfo::monitorChargingState() const
{
    return isMonitorChargingState;
}

void QDeclarativeBatteryInfo::setMonitorChargingState(bool monitor)
{
    if (monitor != isMonitorChargingState) {
        isMonitorChargingState = monitor;
        if (monitor) {
            connect(batteryInfo, SIGNAL(chargerTypeChanged(QBatteryInfo::ChargerType)),
                    this, SLOT(_q_chargingStateChanged(int,QBatteryInfo::ChargingState)));
        } else {
            disconnect(batteryInfo, SIGNAL(chargerTypeChanged(QBatteryInfo::ChargerType)),
                       this, SLOT(_q_chargingStateChanged(int,QBatteryInfo::ChargingState)));
        }
        emit monitorChargingStateChanged();
    }
}

/*!
    \qmlproperty enum BatteryInfo::chargingState

    This property holds the charging state. Possible values are:
    \list
    \o BatteryInfo.UnknownChargingState
    \o BatteryInfo.NotCharging
    \o BatteryInfo.Charging
    \o BatteryInfo.Discharging
    \endlist
*/
QDeclarativeBatteryInfo::ChargingState QDeclarativeBatteryInfo::chargingState(int battery) const
{
    return static_cast<ChargingState>(batteryInfo->chargingState(battery));
}

void QDeclarativeBatteryInfo::_q_chargingStateChanged(int battery, QBatteryInfo::ChargingState state)
{
    emit chargingStateChanged(battery, static_cast<ChargingState>(state));
}

/*!
    \qmlproperty enum BatteryInfo::energyUnit

    This property holds the energy unit used. Possible values are:
    \list
    \o BatteryInfo.UnitUnknown
    \o BatteryInfo.UnitmAh
    \o BatteryInfo.UnitmWh
    \endlist
*/
QDeclarativeBatteryInfo::EnergyUnit QDeclarativeBatteryInfo::energyUnit() const
{
    return static_cast<EnergyUnit>(batteryInfo->energyUnit());
}

/*!
    \qmlmethod int BatteryInfo::maximumCapacity(int battery)

    Returns the maximum capacity of the \a battery
*/
int QDeclarativeBatteryInfo::maximumCapacity(int battery) const
{
    return batteryInfo->maximumCapacity(battery);
}

QT_END_NAMESPACE
