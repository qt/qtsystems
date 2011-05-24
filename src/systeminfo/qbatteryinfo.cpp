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

#include "qbatteryinfo.h"

#if defined(Q_OS_LINUX)
#  include "qbatteryinfo_linux_p.h"
#else
QT_BEGIN_NAMESPACE
class QBatteryInfoPrivate
{
public:
    QBatteryInfoPrivate(QBatteryInfo *) {}

    int currentFlow(int) { return 0; }
    int maximumCapacity(int) { return -1; }
    int remainingCapacity(int) { return -1; }
    int remainingChargingTime(int) { return -1; }
    int voltage(int) { return -1; }
    QBatteryInfo::ChargerType chargerType() { return QBatteryInfo::UnknownCharger; }
    QBatteryInfo::ChargingState chargingState(int) { return QBatteryInfo::UnknownChargingState; }
    QBatteryInfo::EnergyUnit energyUnit() { return QBatteryInfo::UnitUnknown; }
};
QT_END_NAMESPACE
#endif

QT_BEGIN_NAMESPACE

/*!
    \class QBatteryInfo
    \inmodule QtSystemKit
    \brief The QBatteryInfo class provides various information of the battery.
*/

/*!
    \enum QBatteryInfo::ChargerType
    This enum describes the type of charger used.

    \value UnknownCharger           The charger type is unknown, or no charger.
    \value WallCharger              Using wall (mains) charger.
    \value USBCharger               Using USB charger when the system cannot differentiate the current.
    \value USB_500mACharger         Using USB charger at 500 mA.
    \value USB_100mACharger         Using USB charger at 100 mA.
    \value VariableCurrentCharger   Using variable current charger such as bicycle or solar.
*/

/*!
    \enum QBatteryInfo::ChargingState
    This enum describes the charging state:

    \value UnknownChargingState  The charging state is unknown.
    \value NotCharging           The battery is not charging, i.e. full.
    \value Charging              The battery is charging.
    \value Discharging           The battery is discharging.
*/

/*!
    \enum QBatteryInfo::EnergyUnit
    This enum describes the energy unit used by the system.

    \value UnitUnknown            Energy unit unknown.
    \value UnitmAh                Energy described in milliamp-hour (mAh)
    \value UnitmWh                Energy described in milliwatt-hour (mWh)
*/

/*!
    \fn void QBatteryInfo::chargerTypeChanged(QBatteryInfo::ChargerType type);

    This signal is emitted when the charger has changed to \a type.
*/

/*!
    \fn void QBatteryInfo::chargingStateChanged(int battery, QBatteryInfo::ChargingState state);

    This signal is emitted when the charging state of the \a battery has changed to \a state.
*/

/*!
    \fn void QBatteryInfo::currentFlowChanged(int battery, int flow);

    This signal is emitted when the current flow of the \a battery has changed to \a flow, measured
    in milliapmeres (mA). Note that listening to this signal could lead to an intensive CPU usage.
*/

/*!
    \fn void QBatteryInfo::remainingCapacityChanged(int battery, int capacity);

    This signal is emitted when the remaining capacity of the \a battery has changed to \a capacity,
    which is measured in QBatteryInfo::EnergyUnit.
*/

/*!
    \fn void QBatteryInfo::remainingChargingTimeChanged(int battery, int seconds);

    This signal is emitted when the remaining charging time of the \a battery has changed to \a seconds.
*/

/*!
    \fn void QBatteryInfo::voltageChanged(int battery, int voltage);

    This signal is emitted when the current voltage of the \a battery has changed to \a voltage,
    measured in millivolts (mV).
*/

/*!
    Constructs a QBatteryInfo object with the given \a parent.
*/
QBatteryInfo::QBatteryInfo(QObject *parent)
    : QObject(parent)
    , d_ptr(new QBatteryInfoPrivate(this))
{
}

/*!
    Destroys the object
*/
QBatteryInfo::~QBatteryInfo()
{
    delete d_ptr;
}

/*!
    Returns the current flow of the given \a battery, measured in milliapmeres (mA). A positive
    returned value means discharging, and a negative value means charging. In case of error, or
    the information if not available, 0 is returned.
*/
int QBatteryInfo::currentFlow(int battery) const
{
    return d_ptr->currentFlow(battery);
}

/*!
    Returns the maximum capacity of the given \a battery, measured in QBatteryInfo::EnergyUnit.
    If the battery is not found, or the information is not available, -1 is returned.
*/
int QBatteryInfo::maximumCapacity(int battery) const
{
    return d_ptr->maximumCapacity(battery);
}

/*!
    Returns the remaining level of the given \a battery, measured in QBatteryInfo::EnergyUnit. If
    the battery is not found, or the information is not available, -1 is returned.
*/
int QBatteryInfo::remainingCapacity(int battery) const
{
    return d_ptr->remainingCapacity(battery);
}

/*!
    Returns the remaining charging time needed for \a battery, measured in seconds. If the battery
    is full or not charging, 0 is returned. If the battery is not found or the information is not
    available, -1 is returned.
*/
int QBatteryInfo::remainingChargingTime(int battery) const
{
    return d_ptr->remainingChargingTime(battery);
}

/*!
    Returns the voltage of the given \a battery, measured in millivolts (mV). If the battery is not
    found, or the information is not available, -1 is returned.
*/
int QBatteryInfo::voltage(int battery) const
{
    return d_ptr->voltage(battery);
}

/*!
    \property QBatteryInfo::chargerType
    \brief The type of the charger.

    Returns the type of the charger currently used.
*/
QBatteryInfo::ChargerType QBatteryInfo::chargerType() const
{
    return d_ptr->chargerType();
}

/*!
    Returns the charging state of the given \a battery.
*/
QBatteryInfo::ChargingState QBatteryInfo::chargingState(int battery) const
{
    return d_ptr->chargingState(battery);
}

/*!
    Returns the energy unit that the system uses.
*/
QBatteryInfo::EnergyUnit QBatteryInfo::energyUnit() const
{
    return d_ptr->energyUnit();
}

QT_END_NAMESPACE
