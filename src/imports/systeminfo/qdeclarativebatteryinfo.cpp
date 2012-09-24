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

#include "qdeclarativebatteryinfo_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype BatteryInfo
    \instantiates QDeclarativeBatteryInfo
    \inqmlmodule QtSystemInfo
    \ingroup qml-systeminfo
    \brief The BatteryInfo element provides various information about the battery status.
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
    , isMonitorBatteryStatus(false)
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

    \sa batteryCount
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

    \sa chargerType
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
    \qmlproperty enumeration BatteryInfo::chargerType

    This property holds the type of the charger. Possible values are:
    \list
    \li BatteryInfo.UnknownCharger           - The charger type is unknown, or no charger.
    \li BatteryInfo.WallCharger              - Using wall (mains) charger.
    \li BatteryInfo.USBCharger               - Using USB charger when the system cannot differentiate the current.
    \li BatteryInfo.VariableCurrentCharger   - Using variable current charger such as bicycle or solar.
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

    \sa onCurrentFlowChanged
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
    \qmlsignal BatteryInfo::onCurrentFlowChanged(int battery, int flow)

    This handler is called when current flow of \a battery has changed to \a flow.
    Note that it won't be called unless monitorCurrentFlow is set true.

    \sa currentFlow, monitorCurrentFlow
 */

/*!
    \qmlmethod int BatteryInfo::currentFlow(int battery)

    Returns the current flow of the \a battery.

    \sa onCurrentFlowChanged
*/
int QDeclarativeBatteryInfo::currentFlow(int battery) const
{
    return batteryInfo->currentFlow(battery);
}

/*!
    \qmlproperty bool BatteryInfo::monitorRemainingCapacity

    This property holds whether or not monitor the remaining capacity of batteries.

    \sa onRemainingCapacityChanged
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
    \qmlsignal BatteryInfo::onRemainingCapacityChanged(int battery, int capacity)

    This handler is called when remaining capacity of \a battery has changed to \a capacity.
    Note that it won't be called unless monitorRemainingCapacity is set true.

    \sa remainingCapacity, monitorRemainingCapacity
 */

/*!
    \qmlmethod int BatteryInfo::remainingCapacity(int battery)

    Returns the remaining capacity of the \a battery.

    \sa onRemainingCapacityChanged
*/
int QDeclarativeBatteryInfo::remainingCapacity(int battery) const
{
    return batteryInfo->remainingCapacity(battery);
}

/*!
    \qmlproperty bool BatteryInfo::monitorRemainingChargingTime

    This property holds whether or not monitor the remaining charging time of batteries.

    \sa onRemainingChargingTimeChanged
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
    \qmlsignal BatteryInfo::onRemainingChargingTimeChanged(int battery, int seconds)

    This handler is called when remaining charging time of \a battery has changed to \a seconds.
    Note that it won't be called unless monitorRemainingChargingTime is set true.

    \sa remainingChargingTime, monitorRemainingChargingTime
 */

/*!
    \qmlmethod int BatteryInfo::remainingChargingTime(int battery)

    Returns the remaining charging time of the \a battery.

    \sa onRemainingChargingTimeChanged
*/
int QDeclarativeBatteryInfo::remainingChargingTime(int battery) const
{
    return batteryInfo->remainingChargingTime(battery);
}

/*!
    \qmlproperty bool BatteryInfo::monitorVoltage

    This property holds whether or not monitor the voltage of batteries.

    \sa onVoltageChanged
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
    \qmlsignal BatteryInfo::onVoltageChanged(int battery, int voltage)

    This handler is called when voltage of \a battery has changed to \a voltage.
    Note that it won't be called unless monitorVoltage is set true.

    \sa voltage, monitorVoltage
 */

/*!
    \qmlmethod int BatteryInfo::voltage(int battery)

    Returns the voltage of the \a battery.

    \sa onVoltageChanged
*/
int QDeclarativeBatteryInfo::voltage(int battery) const
{
    return batteryInfo->voltage(battery);
}

/*!
    \qmlproperty bool BatteryInfo::monitorChargingState

    This property holds whether or not monitor the change of charging state.

    \sa onChargingStateChanged
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
            connect(batteryInfo, SIGNAL(chargingStateChanged(int,QBatteryInfo::ChargingState)),
                    this, SLOT(_q_chargingStateChanged(int,QBatteryInfo::ChargingState)));
        } else {
            disconnect(batteryInfo, SIGNAL(chargingStateChanged(int,QBatteryInfo::ChargingState)),
                       this, SLOT(_q_chargingStateChanged(int,QBatteryInfo::ChargingState)));
        }
        emit monitorChargingStateChanged();
    }
}

/*!
    \qmlmethod ChargingState BatteryInfo::chargingState(int battery)

    Returns the charging state of the given \a battery. Possible values are:
    \list
    \li BatteryInfo.UnknownChargingState  - The charging state is unknown or charging error occured.
    \li BatteryInfo.NotCharging           - The battery is not charging, i.e. too low charger power.
    \li BatteryInfo.Charging              - The battery is charging.
    \li BatteryInfo.Discharging           - The battery is discharging.
    \li BatteryInfo.Full                  - The battery is fully charged.
    \endlist

    \sa onChargingStateChanged
*/
int QDeclarativeBatteryInfo::chargingState(int battery) const
{
    return batteryInfo->chargingState(battery);
}

/*!
    \qmlsignal BatteryInfo::onChargingStateChanged(int battery, ChargingState state)

    This handler is called when charging state of \a battery has changed to \a state.
    Note that it won't be called unless monitorChargingState is set true.

    \sa chargingState, monitorChargingState
 */
void QDeclarativeBatteryInfo::_q_chargingStateChanged(int battery, QBatteryInfo::ChargingState state)
{
    emit chargingStateChanged(battery, static_cast<ChargingState>(state));
}

/*!
    \qmlproperty enumeration BatteryInfo::energyUnit

    This property holds the energy unit used. Possible values are:
    \list
    \li BatteryInfo.UnitUnknown            - Energy unit unknown.
    \li BatteryInfo.UnitmAh                - Energy described in milliamp-hour (mAh)
    \li BatteryInfo.UnitmWh                - Energy described in milliwatt-hour (mWh)
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

/*!
    \qmlproperty bool BatteryInfo::monitorBatteryStatus

    This property holds whether or not monitor the change of battery's status.

    \sa onBatteryStatusChanged
 */
bool QDeclarativeBatteryInfo::monitorBatteryStatus() const
{
    return isMonitorBatteryStatus;
}

void QDeclarativeBatteryInfo::setMonitorBatteryStatus(bool monitor)
{
    if (monitor != isMonitorBatteryStatus) {
        isMonitorBatteryStatus = monitor;
        if (monitor) {
            connect(batteryInfo, SIGNAL(batteryStatusChanged(int,QBatteryInfo::BatteryStatus)),
                    this, SLOT(_q_batteryStatusChanged(int,QBatteryInfo::BatteryStatus)));
        } else {
            disconnect(batteryInfo, SIGNAL(batteryStatusChanged(int,QBatteryInfo::BatteryStatus)),
                       this, SLOT(_q_batteryStatusChanged(int,QBatteryInfo::BatteryStatus)));
        }
        emit monitorBatteryStatusChanged();
    }
}

/*!
    \qmlmethod BatteryStatus BatteryInfo::batteryStatus(int battery)

    Returns the status of the given \a battery. Possible values are:
    \list
    \li BatteryInfo.BatteryStatusUnknown      - Battery level undetermined.
    \li BatteryInfo.BatteryEmpty              - Battery is considered be empty and device needs to shut down.
    \li BatteryInfo.BatteryLow                - Battery level is low and warnings need to be issued to the user.
    \li BatteryInfo.BatteryOk                 - Battery level is Ok. It is above "Low" but not "Full".
    \li BatteryInfo.BatteryFull               - Battery is fully charged.
    \endlist

    \sa onBatteryStatusChanged
*/
int QDeclarativeBatteryInfo::batteryStatus(int battery) const
{
    return batteryInfo->batteryStatus(battery);
}

/*!
    \qmlsignal BatteryInfo::onBatteryStatusChanged(int battery, BatteryStatus status)

    This handler is called when status of \a battery has changed to \a status.
    Note that it won't be called unless monitorBatteryStatus is set true.

    \sa batteryStatus, monitorBatteryStatus
 */
void QDeclarativeBatteryInfo::_q_batteryStatusChanged(int battery, QBatteryInfo::BatteryStatus status)
{
    emit batteryStatusChanged(battery, static_cast<BatteryStatus>(status));
}

QT_END_NAMESPACE
