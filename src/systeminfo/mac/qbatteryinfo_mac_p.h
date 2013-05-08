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

#ifndef QBATTERYINFO_MAC_P_H
#define QBATTERYINFO_MAC_P_H

#include <qbatteryinfo.h>

#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

class QBatteryInfoPrivate : public QObject
{
    Q_OBJECT

public:
    QBatteryInfoPrivate(QBatteryInfo *parent);
    ~QBatteryInfoPrivate();

    int batteryCount();
    int currentFlow(int battery);
    int maximumCapacity(int battery);
    int remainingCapacity(int battery);
    int remainingChargingTime(int battery);
    int voltage(int battery);

    QBatteryInfo::ChargerType chargerType();
    QBatteryInfo::ChargingState chargingState(int battery);
    QBatteryInfo::EnergyUnit energyUnit();
    QBatteryInfo::BatteryStatus batteryStatus(int battery);
    void getBatteryInfo();
    QBatteryInfo::ChargingState currentChargingState();

Q_SIGNALS:
    void batteryCountChanged(int count);
    void chargerTypeChanged(QBatteryInfo::ChargerType type);
    void chargingStateChanged(int battery, QBatteryInfo::ChargingState state);
    void currentFlowChanged(int battery, int flow);
    void remainingCapacityChanged(int battery, int capacity);
    void remainingChargingTimeChanged(int battery, int seconds);
    void voltageChanged(int battery, int voltage);
    void batteryStatusChanged(int battery, QBatteryInfo::BatteryStatus);

protected:
    void connectNotify(const QMetaMethod &signal);
    void disconnectNotify(const QMetaMethod &signal);

private Q_SLOTS:
private:
    QBatteryInfo * const q_ptr;
    Q_DECLARE_PUBLIC(QBatteryInfo)

    int batteryLevelCache;
    QBatteryInfo::ChargingState currentChargingStateCache;
    QBatteryInfo::BatteryStatus batteryStatusCache;


    QBatteryInfo::BatteryStatus currentBatStatus;
    QBatteryInfo::ChargingState curChargeState;
    QBatteryInfo::ChargerType curChargeType;

    int currentBatLevel;
    int currentVoltage;
    int dischargeRate;
    int capacity;
    int currentCapacity;
    int timeToFull;
    int remainingEnergy;
    int numberOfBatteries;

//    int batteryCounts;
    QMap<int, int> currentFlows; // <battery ID, current value> pair
    QMap<int, int> voltages;
    QMap<int, int> remainingCapacities;
    QMap<int, int> remainingChargingTimes;
    QMap<int, int> maximumCapacities;
    QMap<int, QBatteryInfo::ChargingState> chargingStates;
    QBatteryInfo::ChargerType currentChargerType;
    QMap<int, QBatteryInfo::BatteryStatus> batteryStatuses;

};

QT_END_NAMESPACE

#endif // QBATTERYINFO_MAC_P_H
