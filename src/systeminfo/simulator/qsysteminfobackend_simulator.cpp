/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Copyright (C) 2014 BlackBerry Limited. All rights reserved.
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

#include "qsysteminfobackend_simulator_p.h"

#include <QMutex>

QT_BEGIN_NAMESPACE

QBatteryInfoSimulatorBackend *QBatteryInfoSimulatorBackend::globalSimulatorBackend = 0;

// QBatteryInfoSimulatorBackend

QBatteryInfoSimulatorBackend::QBatteryInfoSimulatorBackend(QObject *parent)
    : QObject(parent)
{
    data.index = 0;
    data.currentFlow = 0;
    data.cycleCount = -1;
    data.maximumCapacity = -1;
    data.remainingCapacity = -1;
    data.remainingChargingTime = -1;
    data.voltage = -1;
    data.chargingState = QBatteryInfo::UnknownChargingState;
    data.chargerType = QBatteryInfo::UnknownCharger;
    data.levelStatus = QBatteryInfo::LevelUnknown;
    data.health = QBatteryInfo::HealthUnknown;
}

QBatteryInfoSimulatorBackend::~QBatteryInfoSimulatorBackend()
{
}

QBatteryInfoSimulatorBackend *QBatteryInfoSimulatorBackend::getSimulatorBackend()
{
    static QMutex mutex;

    mutex.lock();
    if (!globalSimulatorBackend)
        globalSimulatorBackend = new QBatteryInfoSimulatorBackend();
    mutex.unlock();

    return globalSimulatorBackend;
}


int QBatteryInfoSimulatorBackend::getBatteryCount()
{
    return 1;
}

int QBatteryInfoSimulatorBackend::getBatteryIndex() const
{
    return data.index;
}

int QBatteryInfoSimulatorBackend::getLevel(int battery)
{
    if (battery == 0) {
        int maxCapacity = getMaximumCapacity(battery);
        int remCapacity = getRemainingCapacity(battery);

        if (maxCapacity == 0)
            return -1;

        return remCapacity * 100 / maxCapacity;
    }

    return -1;
}

int QBatteryInfoSimulatorBackend::getCurrentFlow(int battery)
{
    if (battery == 0)
        return data.currentFlow;
    return -1;
}

int QBatteryInfoSimulatorBackend::getCycleCount(int battery)
{
    if (battery == 0)
        return data.cycleCount;

    return -1;
}

int QBatteryInfoSimulatorBackend::getMaximumCapacity(int battery)
{
    if (battery == 0)
        return data.maximumCapacity;
    return -1;
}

int QBatteryInfoSimulatorBackend::getRemainingCapacity(int battery)
{
    if (battery == 0)
        return data.remainingCapacity;
    return -1;
}

int QBatteryInfoSimulatorBackend::getRemainingChargingTime(int battery)
{
    if (battery == 0)
        return data.remainingChargingTime;
    return -1;
}

int QBatteryInfoSimulatorBackend::getVoltage(int battery)
{
    if (battery == 0)
        return data.voltage;
    return -1;
}

QBatteryInfo::ChargingState QBatteryInfoSimulatorBackend::getChargingState(int battery)
{
    if (battery == 0)
        return data.chargingState;

    return QBatteryInfo::UnknownChargingState;
}

QBatteryInfo::ChargerType QBatteryInfoSimulatorBackend::getChargerType()
{
    return data.chargerType;
}

QBatteryInfo::LevelStatus QBatteryInfoSimulatorBackend::getLevelStatus(int battery)
{
    if (battery == 0)
        return data.levelStatus;

    return QBatteryInfo::LevelUnknown;
}

QBatteryInfo::Health QBatteryInfoSimulatorBackend::getHealth(int battery)
{
    if (battery == 0)
        return data.health;

    return QBatteryInfo::HealthUnknown;
}

float QBatteryInfoSimulatorBackend::getTemperature(int battery)
{
    if (battery == 0)
        return data.temperature;

    return -1.0f;
}

void QBatteryInfoSimulatorBackend::setBatteryIndex(int batteryIndex)
{
    if (data.index != batteryIndex) {
        data.index = batteryIndex;
        emit batteryIndexChanged(data.index);
    }
}

void QBatteryInfoSimulatorBackend::setCurrentFlow(int flow)
{
    if (data.currentFlow != flow) {
        data.currentFlow = flow;
        emit currentFlowChanged(flow);
    }
}

void QBatteryInfoSimulatorBackend::setCycleCount(int cycleCount)
{
    if (data.cycleCount != cycleCount) {
        data.cycleCount = cycleCount;
        emit cycleCountChanged(cycleCount);
    }
}

void QBatteryInfoSimulatorBackend::setMaximumCapacity(int capacity)
{
    if (data.maximumCapacity != capacity) {
        int levelBefore = getLevel(0);
        data.maximumCapacity = capacity;
        int levelNow = getLevel(0);
        if (levelBefore != levelNow) {
            emit levelChanged(levelNow);
        }
    }
}

void QBatteryInfoSimulatorBackend::setRemainingCapacity(int capacity)
{
    if (data.remainingCapacity != capacity) {
        int levelBefore = getLevel(0);
        data.remainingCapacity = capacity;
        emit remainingCapacityChanged(capacity);
        int levelNow = getLevel(0);
        if (levelBefore != levelNow) {
            emit levelChanged(levelNow);
        }
    }
}

void QBatteryInfoSimulatorBackend::setVoltage(int vol)
{
    if (data.voltage != vol) {
        data.voltage = vol;
        emit voltageChanged(vol);
    }
}

void QBatteryInfoSimulatorBackend::setRemainingChargingTime(int time)
{
    if (data.remainingChargingTime != time) {
        data.remainingChargingTime = time;
        emit remainingChargingTimeChanged(time);
    }
}

void QBatteryInfoSimulatorBackend::setChargingState(QBatteryInfo::ChargingState state)
{
    if (data.chargingState != state) {
        data.chargingState = state;
        emit chargingStateChanged(state);
    }
}

void QBatteryInfoSimulatorBackend::setChargerType(QBatteryInfo::ChargerType type)
{
    if (data.chargerType != type) {
        data.chargerType = type;
        emit chargerTypeChanged(type);
    }
}

void QBatteryInfoSimulatorBackend::setLevelStatus(QBatteryInfo::LevelStatus levelStatus)
{
    if (data.levelStatus != levelStatus) {
        data.levelStatus = levelStatus;
        emit levelStatusChanged(levelStatus);
    }
}

void QBatteryInfoSimulatorBackend::setHealth(QBatteryInfo::Health health)
{
    if (data.health != health) {
        data.health = health;
        emit healthChanged(health);
    }
}

void QBatteryInfoSimulatorBackend::setTemperature(float temperature)
{
    if (!qFuzzyCompare(data.temperature, temperature)) {
        data.temperature = temperature;
        emit temperatureChanged(temperature);
    }
}

QT_END_NAMESPACE
