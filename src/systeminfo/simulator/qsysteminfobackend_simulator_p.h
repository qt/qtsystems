/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd and/or its subsidiary(-ies).
** Copyright (C) 2018 BlackBerry Limited. All rights reserved.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSYSTEMINFOBACKEND_SIMULATOR_P_H
#define QSYSTEMINFOBACKEND_SIMULATOR_P_H


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

#include "qsysteminfodata_simulator_p.h"

QT_BEGIN_NAMESPACE

// QBatteryInfoSimulatorBackend

class Q_SYSTEMINFO_EXPORT QBatteryInfoSimulatorBackend : public QObject
{
    Q_OBJECT

private:
    QBatteryInfoSimulatorBackend(QObject *parent = 0);
    Q_DISABLE_COPY(QBatteryInfoSimulatorBackend)

public:
    ~QBatteryInfoSimulatorBackend();
    static QBatteryInfoSimulatorBackend *getSimulatorBackend();

    int getBatteryCount();
    int getBatteryIndex() const;
    int getLevel(int battery);
    int getCurrentFlow(int battery);
    int getCycleCount(int battery);
    int getMaximumCapacity(int battery);
    int getRemainingCapacity(int battery);
    int getRemainingChargingTime(int battery);
    int getVoltage(int battery);
    QBatteryInfo::ChargingState getChargingState(int battery);
    QBatteryInfo::ChargerType getChargerType();
    QBatteryInfo::LevelStatus getLevelStatus(int battery);
    QBatteryInfo::Health getHealth(int battery);
    float getTemperature(int battery);

    void setBatteryIndex(int batteryIndex);
    void setCurrentFlow(int flow);
    void setCycleCount(int cycleCount);
    void setMaximumCapacity(int capacity);
    void setRemainingCapacity(int capacity);
    void setRemainingChargingTime(int time);
    void setVoltage(int vol);
    void setChargingState(QBatteryInfo::ChargingState state);
    void setChargerType(QBatteryInfo::ChargerType type);
    void setLevelStatus(QBatteryInfo::LevelStatus levelStatus);
    void setHealth(QBatteryInfo::Health health);
    void setTemperature(float temperature);

Q_SIGNALS:
    void batteryCountChanged(int count);
    void batteryIndexChanged(int batteryIndex);
    void levelChanged(int level);
    void currentFlowChanged(int flow);
    void cycleCountChanged(int cycleCount);
    void remainingCapacityChanged(int capacity);
    void remainingChargingTimeChanged(int seconds);
    void voltageChanged(int voltage);
    void chargingStateChanged(QBatteryInfo::ChargingState state);
    void chargerTypeChanged(QBatteryInfo::ChargerType type);
    void levelStatusChanged(QBatteryInfo::LevelStatus levelStatus);
    void healthChanged(QBatteryInfo::Health health);
    void temperatureChanged(float temperature);

private:
    static QBatteryInfoSimulatorBackend *globalSimulatorBackend;
    QBatteryInfoData data;
};

QT_END_NAMESPACE

#endif // QSYSTEMINFOBACKEND_SIMULATOR_P_H
