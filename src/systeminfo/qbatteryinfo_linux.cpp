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

#include "qbatteryinfo_linux_p.h"

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qtimer.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(const QString, AC_ONLINE_SYSFS_PATH, (QStringLiteral("/sys/class/power_supply/AC/online")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, BATTERY_SYSFS_PATH, (QStringLiteral("/sys/class/power_supply/BAT%1/")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, POWER_SUPPLY_SYSFS_PATH, (QStringLiteral("/sys/class/power_supply/")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, USB_PRESENT_SYSFS_PATH, (QStringLiteral("/sys/class/power_supply/usb/present")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, USB_TYPE_SYSFS_PATH, (QStringLiteral("/sys/class/power_supply/usb/type")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, USB0_PRESENT_SYSFS_PATH, (QStringLiteral("/sys/class/power_supply/USB0/present")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, USB0_TYPE_SYSFS_PATH, (QStringLiteral("/sys/class/power_supply/USB0/type")))

QBatteryInfoPrivate::QBatteryInfoPrivate(QBatteryInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
    , watchBatteryCount(false)
    , watchChargerType(false)
    , watchChargingState(false)
    , watchCurrentFlow(false)
    , watchRemainingCapacity(false)
    , watchRemainingChargingTime(false)
    , watchVoltage(false)
    , batteryCounts(-1)
    , timer(0)
    , currentChargerType(QBatteryInfo::UnknownCharger)
{
}

QBatteryInfoPrivate::~QBatteryInfoPrivate()
{
    delete timer;
}

int QBatteryInfoPrivate::batteryCount()
{
    if (!watchBatteryCount)
        return getBatteryCount();

    return batteryCounts;
}

int QBatteryInfoPrivate::currentFlow(int battery)
{
    if (!watchCurrentFlow)
        return getCurrentFlow(battery);

    return currentFlows.value(battery);
}

int QBatteryInfoPrivate::maximumCapacity(int battery)
{
    if (maximumCapacities[battery] == 0) {
        QFile maximum(BATTERY_SYSFS_PATH()->arg(battery) + QStringLiteral("charge_full"));
        if (maximum.open(QIODevice::ReadOnly)) {
            bool ok = false;
            int capacity = maximum.readAll().simplified().toInt(&ok);
            if (ok)
                maximumCapacities[battery] = capacity / 1000;
            else
                maximumCapacities[battery] = -1;
        } else {
            maximumCapacities[battery] = -1;
        }
    }

    return maximumCapacities[battery];
}

int QBatteryInfoPrivate::remainingCapacity(int battery)
{
    if (!watchRemainingCapacity)
        return getRemainingCapacity(battery);

    return remainingCapacities.value(battery);
}

int QBatteryInfoPrivate::remainingChargingTime(int battery)
{
    if (!watchRemainingChargingTime)
        return getRemainingChargingTime(battery);

    return remainingCapacities.value(battery);
}

int QBatteryInfoPrivate::voltage(int battery)
{
    if (!watchVoltage)
        return getVoltage(battery);

    return voltages.value(battery);
}

QBatteryInfo::ChargerType QBatteryInfoPrivate::chargerType()
{
    if (!watchChargerType)
        return getChargerType();

    return currentChargerType;
}

QBatteryInfo::ChargingState QBatteryInfoPrivate::chargingState(int battery)
{
    if (!watchChargingState)
        return getChargingState(battery);

    return chargingStates.value(battery);
}

QBatteryInfo::EnergyUnit QBatteryInfoPrivate::energyUnit()
{
    return QBatteryInfo::UnitmAh;
}

void QBatteryInfoPrivate::connectNotify(const char *signal)
{
    if (timer == 0) {
        timer = new QTimer;
        timer->setInterval(2000);
        connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    }

    if (!timer->isActive())
        timer->start();

    if (strcmp(signal, SIGNAL(batteryCountChanged(int))) == 0) {
        watchBatteryCount = true;
        batteryCounts = getBatteryCount();
    } else if (strcmp(signal, SIGNAL(currentFlowChanged(int,int))) == 0) {
        watchCurrentFlow = true;
        int count = batteryCount();
        for (int i = 0; i < count; ++i)
            currentFlows[i] = getCurrentFlow(i);
    } else if (strcmp(signal, SIGNAL(voltageChanged(int,int))) == 0) {
        watchVoltage = true;
        int count = batteryCount();
        for (int i = 0; i < count; ++i)
            voltages[i] = getVoltage(i);
    } else if (strcmp(signal, SIGNAL(remainingCapacityChanged(int,int))) == 0) {
        watchRemainingCapacity = true;
        int count = batteryCount();
        for (int i = 0; i < count; ++i)
            remainingCapacities[i] = getRemainingCapacity(i);
    } else if (strcmp(signal, SIGNAL(remainingChargingTimeChanged(int,int))) == 0) {
        watchRemainingChargingTime = true;
        int count = batteryCount();
        for (int i = 0; i < count; ++i)
            remainingChargingTimes[i] = getRemainingChargingTime(i);
    } else if (strcmp(signal, SIGNAL(chargerTypeChanged(QBatteryInfo::ChargerType))) == 0) {
        watchChargerType = true;
        currentChargerType = getChargerType();
    } else if (strcmp(signal, SIGNAL(chargingStateChanged(int,QBatteryInfo::ChargingState))) == 0) {
        watchChargingState = true;
        int count = batteryCount();
        for (int i = 0; i < count; ++i)
            chargingStates[i] = getChargingState(i);
    }
}

void QBatteryInfoPrivate::disconnectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(batteryCountChanged(int))) == 0) {
        watchBatteryCount = false;
        batteryCounts = -1;
    } else if (strcmp(signal, SIGNAL(currentFlowChanged(int,int))) == 0) {
        watchCurrentFlow = false;
        currentFlows.clear();
    } else if (strcmp(signal, SIGNAL(voltageChanged(int,int))) == 0) {
        watchVoltage = false;
        voltages.clear();
    } else if (strcmp(signal, SIGNAL(remainingCapacityChanged(int,int))) == 0) {
        watchRemainingCapacity = false;
        remainingCapacities.clear();
    } else if (strcmp(signal, SIGNAL(remainingChargingTimeChanged(int,int))) == 0) {
        watchRemainingChargingTime = false;
        remainingChargingTimes.clear();
    } else if (strcmp(signal, SIGNAL(chargerTypeChanged(QBatteryInfo::ChargerType))) == 0) {
        watchChargerType = false;
        currentChargerType = QBatteryInfo::UnknownCharger;
    } else if (strcmp(signal, SIGNAL(chargingStateChanged(int,QBatteryInfo::ChargingState))) == 0) {
        watchChargingState = false;
        chargingStates.clear();
    }

    if (!watchBatteryCount && !watchChargerType && !watchChargingState
        && !watchCurrentFlow && !watchRemainingCapacity
        && !watchRemainingChargingTime && !watchVoltage) {
        timer->stop();
    }
}

void QBatteryInfoPrivate::onTimeout()
{
    int count = getBatteryCount();
    int value;
    if (watchBatteryCount) {
        value = getBatteryCount();
        if (batteryCounts != value) {
            batteryCounts = value;
            emit batteryCountChanged(value);
        }
    }

    for (int i = 0; i < count; ++i) {
        if (watchCurrentFlow) {
            value = getCurrentFlow(i);
            if (currentFlows.value(i) != value) {
                currentFlows[i] = value;
                emit currentFlowChanged(i, value);
            }
        }

        if (watchVoltage) {
            value = getVoltage(i);
            if (voltages.value(i) != value) {
                voltages[i] = value;
                emit voltageChanged(i, value);
            }
        }

        if (watchRemainingCapacity) {
            value = getRemainingCapacity(i);
            if (remainingCapacities.value(i) != value) {
                remainingCapacities[i] = value;
                emit remainingCapacityChanged(i, value);
            }
        }

        if (watchRemainingChargingTime) {
            value = getRemainingChargingTime(i);
            if (remainingChargingTimes.value(i) != value) {
                remainingChargingTimes[i] = value;
                emit remainingChargingTimeChanged(i, value);
            }
        }

        if (watchChargerType) {
            QBatteryInfo::ChargerType charger = getChargerType();
            if (currentChargerType != charger) {
                currentChargerType = charger;
                emit chargerTypeChanged(charger);
            }
        }

        if (watchChargingState) {
            QBatteryInfo::ChargingState state = getChargingState(i);
            if (chargingStates.value(i) != state) {
                chargingStates[i] = state;
                emit chargingStateChanged(i, state);
            }
        }
    }
}

int QBatteryInfoPrivate::getBatteryCount()
{
    return QDir(*POWER_SUPPLY_SYSFS_PATH()).entryList(QStringList() << QStringLiteral("BAT*")).size();
}

int QBatteryInfoPrivate::getCurrentFlow(int battery)
{
    QBatteryInfo::ChargingState state = chargingState(battery);
    if (state == QBatteryInfo::UnknownChargingState)
        return 0;

    QFile current(BATTERY_SYSFS_PATH()->arg(battery) + QStringLiteral("current_now"));
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

int QBatteryInfoPrivate::getRemainingCapacity(int battery)
{
    QFile remaining(BATTERY_SYSFS_PATH()->arg(battery) + QStringLiteral("charge_now"));
    if (!remaining.open(QIODevice::ReadOnly))
        return -1;

    bool ok = false;
    int capacity = remaining.readAll().simplified().toInt(&ok);
    if (ok)
        return capacity / 1000;
    return -1;
}

int QBatteryInfoPrivate::getRemainingChargingTime(int battery)
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

int QBatteryInfoPrivate::getVoltage(int battery)
{
    QFile current(BATTERY_SYSFS_PATH()->arg(battery) + QStringLiteral("voltage_now"));
    if (!current.open(QIODevice::ReadOnly))
        return -1;

    bool ok = false;
    int voltage = current.readAll().simplified().toInt(&ok);
    if (ok)
        return voltage / 1000;
    return -1;
}

QBatteryInfo::ChargerType QBatteryInfoPrivate::getChargerType()
{
    QFile charger(*AC_ONLINE_SYSFS_PATH());
    if (charger.open(QIODevice::ReadOnly)) {
        char online;
        if (charger.read(&online, 1) == 1 && online == '1')
            return QBatteryInfo::WallCharger;
        charger.close();
    }

    QMap<QString, QString> chargerMap;
    chargerMap.insert(*USB0_PRESENT_SYSFS_PATH(), *USB0_TYPE_SYSFS_PATH());
    chargerMap.insert(*USB_PRESENT_SYSFS_PATH(), *USB_TYPE_SYSFS_PATH());

    QList<QString> presentPaths = chargerMap.keys();
    foreach (const QString &presentPath, presentPaths) {
        charger.setFileName(presentPath);
        if (charger.open(QIODevice::ReadOnly)) {
            char present;
            if (charger.read(&present, 1) == 1 && present == '1') {
                charger.close();

                charger.setFileName(chargerMap.value(presentPath));
                if (charger.open(QIODevice::ReadOnly)) {
                    if (charger.readAll().simplified() == "USB_DCP")
                        return QBatteryInfo::WallCharger;
                    return QBatteryInfo::USBCharger;
                }
            }
            charger.close();
        }
    }

    return QBatteryInfo::UnknownCharger;
}

QBatteryInfo::ChargingState QBatteryInfoPrivate::getChargingState(int battery)
{
    QFile state(BATTERY_SYSFS_PATH()->arg(battery) + QStringLiteral("status"));
    if (!state.open(QIODevice::ReadOnly))
        return QBatteryInfo::UnknownChargingState;

    QByteArray status = state.readAll().simplified();
    if (status == "Charging")
        return QBatteryInfo::Charging;
    else if (status == "Not charging")
        return QBatteryInfo::NotCharging;
    else if (status == "Discharging")
        return QBatteryInfo::Discharging;
    else if (status == "Full")
        return QBatteryInfo::Full;

    return QBatteryInfo::UnknownChargingState;
}

QT_END_NAMESPACE
