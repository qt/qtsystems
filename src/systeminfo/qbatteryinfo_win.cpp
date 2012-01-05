/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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

#include "qbatteryinfo_win_p.h"

#include <windows.h>
#include <PowrProf.h>

QT_BEGIN_NAMESPACE

QBatteryInfoPrivate::QBatteryInfoPrivate(QBatteryInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
{
}

QBatteryInfoPrivate::~QBatteryInfoPrivate()
{
}

int QBatteryInfoPrivate::batteryCount()
{
    SYSTEM_BATTERY_STATE state;
    if (CallNtPowerInformation(SystemBatteryState, 0, 0, &state, sizeof(state)) == 0) {
        if (state.BatteryPresent)
            return 1;
        else
            return 0;
    }

    return -1;
}

int QBatteryInfoPrivate::currentFlow(int battery)
{
    Q_UNUSED(battery)
    return 0;
}

int QBatteryInfoPrivate::maximumCapacity(int battery)
{
    Q_UNUSED(battery)

    SYSTEM_BATTERY_STATE state;
    if (CallNtPowerInformation(SystemBatteryState, 0, 0, &state, sizeof(state)) == 0)
        return state.MaxCapacity;

    return -1;
}

int QBatteryInfoPrivate::remainingCapacity(int battery)
{
    Q_UNUSED(battery)

    SYSTEM_BATTERY_STATE state;
    if (CallNtPowerInformation(SystemBatteryState, 0, 0, &state, sizeof(state)) == 0)
        return state.RemainingCapacity;

    return -1;
}

int QBatteryInfoPrivate::remainingChargingTime(int battery)
{
    Q_UNUSED(battery)
    return -1;
}

int QBatteryInfoPrivate::voltage(int battery)
{
    Q_UNUSED(battery)
    return -1;
}

QBatteryInfo::ChargerType QBatteryInfoPrivate::chargerType()
{
    SYSTEM_POWER_STATUS status;
    if (GetSystemPowerStatus(&status)) {
        if (status.ACLineStatus == 1)
            return QBatteryInfo::WallCharger;
    }

    return QBatteryInfo::UnknownCharger;
}

QBatteryInfo::ChargingState QBatteryInfoPrivate::chargingState(int battery)
{
    Q_UNUSED(battery)

    SYSTEM_POWER_STATUS status;
    if (GetSystemPowerStatus(&status)) {
        if (status.BatteryFlag & 8)
            return QBatteryInfo::Charging;
        else if (status.BatteryFlag & 128)
            return QBatteryInfo::NotCharging;
        else if (status.BatteryFlag & 255)
            return QBatteryInfo::UnknownChargingState;
        else if (status.BatteryFlag & 1 || status.BatteryFlag & 2 || status.BatteryFlag & 4)
            return QBatteryInfo::Discharging;
    }

    return QBatteryInfo::UnknownChargingState;
}

QBatteryInfo::EnergyUnit QBatteryInfoPrivate::energyUnit()
{
    return QBatteryInfo::UnitmWh;
}

void QBatteryInfoPrivate::connectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(batteryCountChanged(int))) == 0) {
    } else if (strcmp(signal, SIGNAL(currentFlowChanged(int,int))) == 0) {
    } else if (strcmp(signal, SIGNAL(voltageChanged(int,int))) == 0) {
    } else if (strcmp(signal, SIGNAL(remainingCapacityChanged(int,int))) == 0) {
    } else if (strcmp(signal, SIGNAL(remainingChargingTimeChanged(int,int))) == 0) {
    } else if (strcmp(signal, SIGNAL(chargerTypeChanged(QBatteryInfo::ChargerType))) == 0) {
    } else if (strcmp(signal, SIGNAL(chargingStateChanged(int,QBatteryInfo::ChargingState))) == 0) {
    }
}

void QBatteryInfoPrivate::disconnectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(batteryCountChanged(int))) == 0) {
    } else if (strcmp(signal, SIGNAL(currentFlowChanged(int,int))) == 0) {
    } else if (strcmp(signal, SIGNAL(voltageChanged(int,int))) == 0) {
    } else if (strcmp(signal, SIGNAL(remainingCapacityChanged(int,int))) == 0) {
    } else if (strcmp(signal, SIGNAL(remainingChargingTimeChanged(int,int))) == 0) {
    } else if (strcmp(signal, SIGNAL(chargerTypeChanged(QBatteryInfo::ChargerType))) == 0) {
    } else if (strcmp(signal, SIGNAL(chargingStateChanged(int,QBatteryInfo::ChargingState))) == 0) {
    }
}

QT_END_NAMESPACE
