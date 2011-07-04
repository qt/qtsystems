/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystems module of the Qt Toolkit.
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

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qtimer.h>

QT_BEGIN_NAMESPACE

static const QString BATTERY_SYSFS_PATH(QString::fromAscii("/sys/class/power_supply/BAT%1/"));

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
        QFile maximum(BATTERY_SYSFS_PATH.arg(battery) + QString::fromAscii("charge_full"));
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

    return currentRemainingCapacities.value(battery);
}

int QBatteryInfoPrivate::remainingChargingTime(int battery)
{
    if (!watchRemainingChargingTime)
        return getRemainingChargingTime(battery);

    return currentRemainingCapacities.value(battery);
}

int QBatteryInfoPrivate::voltage(int battery)
{
    if (!watchVoltage)
        return getVoltage(battery);

    return currentVoltages.value(battery);
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

    return currentChargingStates.value(battery);
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
            currentVoltages[i] = getVoltage(i);
    } else if (strcmp(signal, SIGNAL(remainingCapacityChanged(int,int))) == 0) {
        watchRemainingCapacity = true;
        int count = batteryCount();
        for (int i = 0; i < count; ++i)
            currentRemainingCapacities[i] = getRemainingCapacity(i);
    } else if (strcmp(signal, SIGNAL(remainingChargingTimeChanged(int,int))) == 0) {
        watchRemainingChargingTime = true;
        int count = batteryCount();
        for (int i = 0; i < count; ++i)
            currentRemainingChargingTimes[i] = getRemainingChargingTime(i);
    } else if (strcmp(signal, SIGNAL(chargerTypeChanged(QBatteryInfo::ChargerType))) == 0) {
        watchChargerType = true;
        currentChargerType = getChargerType();
    } else if (strcmp(signal, SIGNAL(chargingStateChanged(int,QBatteryInfo::ChargingState))) == 0) {
        watchChargingState = true;
        int count = batteryCount();
        for (int i = 0; i < count; ++i)
            currentChargingStates[i] = getChargingState(i);
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
        currentVoltages.clear();
    } else if (strcmp(signal, SIGNAL(remainingCapacityChanged(int,int))) == 0) {
        watchRemainingCapacity = false;
        currentRemainingCapacities.clear();
    } else if (strcmp(signal, SIGNAL(remainingChargingTimeChanged(int,int))) == 0) {
        watchRemainingChargingTime = false;
        currentRemainingChargingTimes.clear();
    } else if (strcmp(signal, SIGNAL(chargerTypeChanged(QBatteryInfo::ChargerType))) == 0) {
        watchChargerType = false;
        currentChargerType = QBatteryInfo::UnknownCharger;
    } else if (strcmp(signal, SIGNAL(chargingStateChanged(int,QBatteryInfo::ChargingState))) == 0) {
        watchChargingState = false;
        currentChargingStates.clear();
    }

    if (!watchCurrentFlow && !watchVoltage)
        timer->stop();
}

void QBatteryInfoPrivate::onTimeout()
{
    int count = QDir(QString::fromAscii("/sys/class/power_supply/")).entryList(QStringList() << QString::fromAscii("BAT*")).size();
    int value;
    if (watchBatteryCount) {
        value = getBatteryCount();
        if (batteryCounts != value) {
            batteryCounts = value;
            Q_EMIT batteryCountChanged(value);
        }
    }

    for (int i = 0; i < count; ++i) {
        if (watchCurrentFlow) {
            value = getCurrentFlow(i);
            if (currentFlows.value(i) != value) {
                currentFlows[i] = value;
                Q_EMIT currentFlowChanged(i, value);
            }
        }

        if (watchVoltage) {
            value = getVoltage(i);
            if (currentVoltages.value(i) != value) {
                currentVoltages[i] = value;
                Q_EMIT voltageChanged(i, value);
            }
        }

        if (watchRemainingCapacity) {
            value = getRemainingCapacity(i);
            if (currentRemainingCapacities.value(i) != value) {
                currentRemainingCapacities[i] = value;
                Q_EMIT remainingCapacityChanged(i, value);
            }
        }

        if (watchRemainingChargingTime) {
            value = getRemainingChargingTime(i);
            if (currentRemainingChargingTimes.value(i) != value) {
                currentRemainingChargingTimes[i] = value;
                Q_EMIT remainingChargingTimeChanged(i, value);
            }
        }

        if (watchChargerType) {
            QBatteryInfo::ChargerType charger = getChargerType();
            if (currentChargerType != charger) {
                currentChargerType = charger;
                Q_EMIT chargerTypeChanged(charger);
            }
        }

        if (watchChargingState) {
            QBatteryInfo::ChargingState state = getChargingState(i);
            if (currentChargingStates.value(i) != state) {
                currentChargingStates[i] = state;
                Q_EMIT chargingStateChanged(i, state);
            }
        }
    }
}

int QBatteryInfoPrivate::getBatteryCount()
{
    return QDir(QString::fromAscii("/sys/class/power_supply/")).entryList(QStringList() << QString::fromAscii("BAT*")).size();
}

int QBatteryInfoPrivate::getCurrentFlow(int battery)
{
    QBatteryInfo::ChargingState state = chargingState(battery);
    if (state == QBatteryInfo::UnknownChargingState)
        return 0;

    QFile current(BATTERY_SYSFS_PATH.arg(battery) + QString::fromAscii("current_now"));
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
    QFile remaining(BATTERY_SYSFS_PATH.arg(battery) + QString::fromAscii("charge_now"));
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
    QFile current(BATTERY_SYSFS_PATH.arg(battery) + QString::fromAscii("voltage_now"));
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
    QFile charger(QString::fromAscii("/sys/class/power_supply/AC/online"));
    if (charger.open(QIODevice::ReadOnly)) {
        char online;
        if (charger.read(&online, 1) == 1 && online == '1')
            return QBatteryInfo::WallCharger;
        charger.close();
    }

    charger.setFileName(QString::fromAscii("/sys/class/power_supply/usb/present"));
    if (charger.open(QIODevice::ReadOnly)) {
        char present;
        if (charger.read(&present, 1) == 1 && present == '1') {
            charger.close();
            charger.setFileName(QString::fromAscii("/sys/class/power_supply/usb/type"));
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

QBatteryInfo::ChargingState QBatteryInfoPrivate::getChargingState(int battery)
{
    QFile state(BATTERY_SYSFS_PATH.arg(battery) + QString::fromAscii("status"));
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

QT_END_NAMESPACE
