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

#ifndef QBATTERYINFO_H
#define QBATTERYINFO_H

#include <qsysteminfoglobal.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_SIMULATOR)
class QBatteryInfoPrivate;
#else
class QBatteryInfoSimulator;
#endif // QT_SIMULATOR

class Q_SYSTEMINFO_EXPORT QBatteryInfo : public QObject
{
    Q_OBJECT

    Q_ENUMS(ChargerType)
    Q_ENUMS(ChargingState)
    Q_ENUMS(EnergyUnit)
    Q_ENUMS(BatteryStatus)

    Q_PROPERTY(int batteryCount READ batteryCount NOTIFY batteryCountChanged)
    Q_PROPERTY(ChargerType chargerType READ chargerType NOTIFY chargerTypeChanged)
    Q_PROPERTY(EnergyUnit energyUnit READ energyUnit)

public:
    enum ChargerType {
        UnknownCharger = 0,
        WallCharger,
        USBCharger,
        VariableCurrentCharger
    };

    enum ChargingState {
        UnknownChargingState = 0,
        NotCharging,
        Charging,
        Discharging,
        Full
    };

    enum EnergyUnit {
        UnitUnknown = 0,
        UnitmAh,
        UnitmWh
    };

    enum BatteryStatus {
        BatteryStatusUnknown = 0,
        BatteryEmpty,
        BatteryLow,
        BatteryOk,
        BatteryFull
    };

    QBatteryInfo(QObject *parent = 0);
    virtual ~QBatteryInfo();

    int batteryCount() const;
    int currentFlow(int battery) const;
    int maximumCapacity(int battery) const;
    int remainingCapacity(int battery) const;
    int remainingChargingTime(int battery) const;
    int voltage(int battery) const;
    QBatteryInfo::ChargingState chargingState(int battery) const;
    QBatteryInfo::ChargerType chargerType() const;
    QBatteryInfo::EnergyUnit energyUnit() const;
    QBatteryInfo::BatteryStatus batteryStatus(int battery) const;

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

private:
    Q_DISABLE_COPY(QBatteryInfo)
#if !defined(QT_SIMULATOR)
    QBatteryInfoPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(QBatteryInfo)
#else
    QBatteryInfoSimulator * const d_ptr;
#endif // QT_SIMULATOR
};

QT_END_NAMESPACE

#endif // QBATTERYINFO_H
