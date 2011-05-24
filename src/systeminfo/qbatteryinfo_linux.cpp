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

#include "qbatteryinfo_linux_p.h"

#include <QtCore/qfile.h>

QT_BEGIN_NAMESPACE

static const QString BATTERY_SYSFS_PATH("/sys/class/power_supply/BAT%1/");

QBatteryInfoPrivate::QBatteryInfoPrivate(QBatteryInfo *parent)
    : q_ptr(parent)
{
}

int QBatteryInfoPrivate::currentFlow(int battery)
{
    QBatteryInfo::ChargingState state = chargingState(battery);
    if (state == QBatteryInfo::UnknownChargingState)
        return 0;

    QFile current(BATTERY_SYSFS_PATH.arg(battery) + "current_now");
    if (!current.open(QIODevice::ReadOnly))
        return 0;

    bool ok = false;
    int flow = current.readAll().simplified().toInt(&ok);
    if (ok) {
        if (state == QBatteryInfo::Charging)
            return flow / -1000;
        else if (state == QBatteryInfo::Discharging)
            return flow / 1000;
    }

    return 0;
}

int QBatteryInfoPrivate::maximumCapacity(int battery)
{
    QFile maximum(BATTERY_SYSFS_PATH.arg(battery) + "charge_full");
    if (!maximum.open(QIODevice::ReadOnly))
        return -1;

    bool ok = false;
    int capacity = maximum.readAll().simplified().toInt(&ok);
    if (ok)
        return capacity / 1000;
    return -1;
}

int QBatteryInfoPrivate::remainingCapacity(int battery)
{
    QFile remaining(BATTERY_SYSFS_PATH.arg(battery) + "charge_now");
    if (!remaining.open(QIODevice::ReadOnly))
        return -1;

    bool ok = false;
    int capacity = remaining.readAll().simplified().toInt(&ok);
    if (ok)
        return capacity / 1000;
    return -1;
}

int QBatteryInfoPrivate::remainingChargingTime(int battery)
{
    QBatteryInfo::ChargingState state = chargingState(battery);
    if (state == QBatteryInfo::UnknownChargingState)
        return -1;
    else if (state == QBatteryInfo::NotCharging || state == QBatteryInfo::Discharging)
        return 0;

    int max = 0;
    int remaining = 0;
    int current = 0;
    if ((max = maximumCapacity(battery)) == -1
        || (remaining = remainingCapacity(battery)) == -1
        || (current = currentFlow(battery)) == 0) {
        return -1;
    }
    return (max - remaining) * -3600 / current;
}

int QBatteryInfoPrivate::voltage(int battery)
{
    QFile current(BATTERY_SYSFS_PATH.arg(battery) + "voltage_now");
    if (!current.open(QIODevice::ReadOnly))
        return -1;

    bool ok = false;
    int voltage = current.readAll().simplified().toInt(&ok);
    if (ok)
        return voltage / 1000;
    return -1;
}

QBatteryInfo::ChargerType QBatteryInfoPrivate::chargerType()
{
    QFile charger("/sys/class/power_supply/AC/online");
    if (charger.open(QIODevice::ReadOnly)) {
        char online;
        if (charger.read(&online, 1) == 1 && online == '1')
            return QBatteryInfo::WallCharger;
        charger.close();
    }

    charger.setFileName("/sys/class/power_supply/usb/present");
    if (charger.open(QIODevice::ReadOnly)) {
        char present;
        if (charger.read(&present, 1) == 1 && present == '1') {
            charger.close();
            charger.setFileName("/sys/class/power_supply/usb/type");
            if (charger.open(QIODevice::ReadOnly)) {
                if (charger.readAll().simplified() == "USB_DCP")
                    return QBatteryInfo::WallCharger;
                return QBatteryInfo::USBCharger;
            }
        }
        charger.close();
    }

    return QBatteryInfo::UnknownCharger;
}

QBatteryInfo::ChargingState QBatteryInfoPrivate::chargingState(int battery)
{
    QFile state(BATTERY_SYSFS_PATH.arg(battery) + "status");
    if (!state.open(QIODevice::ReadOnly))
        return QBatteryInfo::UnknownChargingState;

    QByteArray status = state.readAll().simplified();
    if (status == "Charging")
        return QBatteryInfo::Charging;
    else if (status == "Discharging")
        return QBatteryInfo::Discharging;
    else if (status == "Full")
        return QBatteryInfo::NotCharging;

    return QBatteryInfo::UnknownChargingState;
}

QBatteryInfo::EnergyUnit QBatteryInfoPrivate::energyUnit()
{
    return QBatteryInfo::UnitmAh;
}

QT_END_NAMESPACE
