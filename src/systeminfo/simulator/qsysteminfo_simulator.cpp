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

#include "qsysteminfo_simulator_p.h"
#include "qsysteminfobackend_simulator_p.h"
#include "qsysteminfoconnection_simulator_p.h"

#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

// QBatteryInfoSimulator

QBatteryInfoSimulator::QBatteryInfoSimulator(QBatteryInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
    , batteryInfoSimulatorBackend(QBatteryInfoSimulatorBackend::getSimulatorBackend())
{
    SystemInfoConnection::ensureSimulatorConnection();
}

QBatteryInfoSimulator::QBatteryInfoSimulator(int batteryIndex, QBatteryInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
    , batteryInfoSimulatorBackend(QBatteryInfoSimulatorBackend::getSimulatorBackend())
{
    SystemInfoConnection::ensureSimulatorConnection();
    setBatteryIndex(batteryIndex);
}

QBatteryInfoSimulator::~QBatteryInfoSimulator()
{
}

int QBatteryInfoSimulator::batteryCount()
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getBatteryCount();

    return -1;
}

int QBatteryInfoSimulator::batteryIndex() const
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getBatteryIndex();

    return -1;
}

bool QBatteryInfoSimulator::isValid()
{
    // valid if the index < total count.
    return (batteryIndex() >= 0) && (batteryIndex() < batteryCount());
}

void QBatteryInfoSimulator::setBatteryIndex(int batteryIndex)
{
    if (batteryInfoSimulatorBackend)
        batteryInfoSimulatorBackend->setBatteryIndex(batteryIndex);
}

int QBatteryInfoSimulator::level()
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getLevel(batteryInfoSimulatorBackend->getBatteryIndex());

    return -1;
}

int QBatteryInfoSimulator::currentFlow(int battery)
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getCurrentFlow(battery);

    return 0;
}

int QBatteryInfoSimulator::currentFlow()
{
    return currentFlow(batteryInfoSimulatorBackend->getBatteryIndex());
}

int QBatteryInfoSimulator::cycleCount()
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getCycleCount(batteryInfoSimulatorBackend->getBatteryIndex());

    return -1;
}

int QBatteryInfoSimulator::maximumCapacity(int battery)
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getMaximumCapacity(battery);

    return -1;
}

int QBatteryInfoSimulator::maximumCapacity()
{
    return maximumCapacity(batteryInfoSimulatorBackend->getBatteryIndex());
}

int QBatteryInfoSimulator::remainingCapacity(int battery)
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getRemainingCapacity(battery);

    return -1;
}

int QBatteryInfoSimulator::remainingCapacity()
{
    return remainingCapacity(batteryInfoSimulatorBackend->getBatteryIndex());
}

int QBatteryInfoSimulator::remainingChargingTime(int battery)
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getRemainingChargingTime(battery);

    return -1;
}

int QBatteryInfoSimulator::remainingChargingTime()
{
    return remainingChargingTime(batteryInfoSimulatorBackend->getBatteryIndex());
}

int QBatteryInfoSimulator::voltage(int battery)
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getVoltage(battery);

    return -1;
}

int QBatteryInfoSimulator::voltage()
{
    return voltage(batteryInfoSimulatorBackend->getBatteryIndex());
}

QBatteryInfo::ChargerType QBatteryInfoSimulator::chargerType()
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getChargerType();

    return QBatteryInfo::UnknownCharger;
}

QBatteryInfo::ChargingState QBatteryInfoSimulator::chargingState(int battery)
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getChargingState(battery);

    return QBatteryInfo::UnknownChargingState;
}

QBatteryInfo::ChargingState QBatteryInfoSimulator::chargingState()
{
    return chargingState(batteryInfoSimulatorBackend->getBatteryIndex());
}

QBatteryInfo::LevelStatus QBatteryInfoSimulator::levelStatus(int battery)
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getLevelStatus(battery);

    return QBatteryInfo::LevelUnknown;
}

QBatteryInfo::LevelStatus QBatteryInfoSimulator::levelStatus()
{
    return levelStatus(batteryInfoSimulatorBackend->getBatteryIndex());
}

QBatteryInfo::Health QBatteryInfoSimulator::health()
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getHealth(batteryInfoSimulatorBackend->getBatteryIndex());

    return QBatteryInfo::HealthUnknown;
}

float QBatteryInfoSimulator::temperature()
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getTemperature(batteryInfoSimulatorBackend->getBatteryIndex());

    return -1.0f;
}

extern QMetaMethod proxyToSourceSignal(const QMetaMethod &, QObject *);

void QBatteryInfoSimulator::connectNotify(const QMetaMethod &signal)
{
    static const QMetaMethod batteryCountChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::batteryCountChanged);
    static const QMetaMethod chargerTypeChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::chargerTypeChanged);
    static const QMetaMethod chargingStateChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::chargingStateChanged);
    static const QMetaMethod currentFlowChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::currentFlowChanged);
    static const QMetaMethod remainingCapacityChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::remainingCapacityChanged);
    static const QMetaMethod remainingChargingTimeChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::remainingChargingTimeChanged);
    static const QMetaMethod voltageChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::voltageChanged);
    static const QMetaMethod levelStatusChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::levelStatusChanged);
    static const QMetaMethod healthChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::healthChanged);
    static const QMetaMethod temperatureChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::temperatureChanged);

    if (batteryInfoSimulatorBackend && (signal == batteryCountChangedSignal
                                        || signal == currentFlowChangedSignal
                                        || signal == voltageChangedSignal
                                        || signal == remainingCapacityChangedSignal
                                        || signal == remainingChargingTimeChangedSignal
                                        || signal == chargerTypeChangedSignal
                                        || signal == chargingStateChangedSignal
                                        || signal == levelStatusChangedSignal
                                        || signal == healthChangedSignal
                                        || signal == temperatureChangedSignal)) {
        QMetaMethod sourceSignal = proxyToSourceSignal(signal, batteryInfoSimulatorBackend);
        connect(batteryInfoSimulatorBackend, sourceSignal, this, signal);
    }
}

void QBatteryInfoSimulator::disconnectNotify(const QMetaMethod &signal)
{
    static const QMetaMethod batteryCountChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::batteryCountChanged);
    static const QMetaMethod chargerTypeChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::chargerTypeChanged);
    static const QMetaMethod chargingStateChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::chargingStateChanged);
    static const QMetaMethod currentFlowChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::currentFlowChanged);
    static const QMetaMethod remainingCapacityChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::remainingCapacityChanged);
    static const QMetaMethod remainingChargingTimeChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::remainingChargingTimeChanged);
    static const QMetaMethod voltageChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::voltageChanged);
    static const QMetaMethod levelStatusChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::levelStatusChanged);
    static const QMetaMethod healthChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::healthChanged);
    static const QMetaMethod temperatureChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoSimulator::temperatureChanged);

    if (batteryInfoSimulatorBackend && (signal == batteryCountChangedSignal
                                        || signal == currentFlowChangedSignal
                                        || signal == voltageChangedSignal
                                        || signal == remainingCapacityChangedSignal
                                        || signal == remainingChargingTimeChangedSignal
                                        || signal == chargerTypeChangedSignal
                                        || signal == chargingStateChangedSignal
                                        || signal == levelStatusChangedSignal
                                        || signal == healthChangedSignal
                                        || signal == temperatureChangedSignal)) {
        QMetaMethod sourceSignal = proxyToSourceSignal(signal, batteryInfoSimulatorBackend);
        disconnect(batteryInfoSimulatorBackend, sourceSignal, this, signal);
    }
}

QT_END_NAMESPACE
