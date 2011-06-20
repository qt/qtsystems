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

#ifndef QBATTERYINFO_H
#define QBATTERYINFO_H

#include "qsysteminfo_p.h"
#include <QtCore/qobject.h>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QBatteryInfoPrivate;

class Q_SYSTEMINFO_EXPORT QBatteryInfo : public QObject
{
    Q_OBJECT

    Q_ENUMS(ChargerType)
    Q_ENUMS(ChargingState)
    Q_ENUMS(EnergyUnit)

    Q_PROPERTY(ChargerType chargerType READ chargerType NOTIFY chargerTypeChanged)
    Q_PROPERTY(EnergyUnit energyUnit READ energyUnit)

public:
    enum ChargerType {
        UnknownCharger = 0,
        WallCharger,
        USBCharger,
        USB_500mACharger, // Is it really necessary to have these different USB charger types?
        USB_100mACharger,
        VariableCurrentCharger
    };

    enum ChargingState {
        UnknownChargingState = 0,
        NotCharging,
        Charging,
        Discharging
    };

    enum EnergyUnit {
            UnitUnknown = -1,
            UnitmAh,
            UnitmWh
        };

    QBatteryInfo(QObject *parent = 0);
    virtual ~QBatteryInfo();

    Q_INVOKABLE int currentFlow(int battery) const;
    Q_INVOKABLE int maximumCapacity(int battery) const;
    Q_INVOKABLE int remainingCapacity(int battery) const;
    Q_INVOKABLE int remainingChargingTime(int battery) const;
    Q_INVOKABLE int voltage(int battery) const;
    Q_INVOKABLE QBatteryInfo::ChargingState chargingState(int battery) const;

    QBatteryInfo::ChargerType chargerType() const;
    QBatteryInfo::EnergyUnit energyUnit() const;

Q_SIGNALS:
    void chargerTypeChanged(QBatteryInfo::ChargerType type);
    void chargingStateChanged(int battery, QBatteryInfo::ChargingState state);
    void currentFlowChanged(int battery, int flow);
    void remainingCapacityChanged(int battery, int capacity);
    void remainingChargingTimeChanged(int battery, int seconds);
    void voltageChanged(int battery, int voltage);

protected:
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);

private:
    Q_DISABLE_COPY(QBatteryInfo)
    QBatteryInfoPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(QBatteryInfo)
};

QT_END_NAMESPACE
QT_END_HEADER

#endif // QBATTERYINFO_H
