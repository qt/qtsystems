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

#include "qbatteryinfo.h"

#if defined(QT_SIMULATOR)
#  include "simulator/qsysteminfo_simulator_p.h"
#elif defined(Q_OS_LINUX)
#if defined(QT_NO_UPOWER)
#include "linux/qbatteryinfo_linux_p.h"
#else
#include "linux/qbatteryinfo_upower_p.h"
#endif
#elif defined(Q_OS_WIN)
#  include "windows/qbatteryinfo_win_p.h"
#elif defined(Q_OS_MAC)
#  include "mac/qbatteryinfo_mac_p.h"
#else
QT_BEGIN_NAMESPACE
class QBatteryInfoPrivate
{
public:
    QBatteryInfoPrivate(QBatteryInfo *) {}

    int batteryCount() { return -1; }
    int currentFlow(int) { return 0; }
    int maximumCapacity(int) { return -1; }
    int remainingCapacity(int) { return -1; }
    int remainingChargingTime(int) { return -1; }
    int voltage(int) { return -1; }
    QBatteryInfo::ChargerType chargerType() { return QBatteryInfo::UnknownCharger; }
    QBatteryInfo::ChargingState chargingState(int) { return QBatteryInfo::UnknownChargingState; }
    QBatteryInfo::EnergyUnit energyUnit() { return QBatteryInfo::UnitUnknown; }
    QBatteryInfo::BatteryStatus batteryStatus(int) { return QBatteryInfo::BatteryStatusUnknown; }
};
QT_END_NAMESPACE
#endif

#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

/*!
    \class QBatteryInfo
    \inmodule QtSystemInfo
    \brief The QBatteryInfo class provides various information about the battery.
    \ingroup systeminfo

    Note that on some platforms, listening to the signals could lead to a heavy CPU usage. Therefore,
    you are strongly suggested to disconnect the signals when no longer needed in your application.

    Battery index starts at 0, which indicates the first battery.
*/

/*!
    \enum QBatteryInfo::ChargerType
    This enum describes the type of charger used.

    \value UnknownCharger           The charger type is unknown, or no charger.
    \value WallCharger              Using wall (mains) charger.
    \value USBCharger               Using USB charger when the system cannot differentiate the current.
    \value VariableCurrentCharger   Using variable current charger such as bicycle or solar.
*/

/*!
    \enum QBatteryInfo::ChargingState
    This enum describes the charging state:

    \value UnknownChargingState  The charging state is unknown or charging error occured.
    \value NotCharging           The battery is not charging, i.e. too low charger power.
    \value Charging              The battery is charging.
    \value Discharging           The battery is discharging.
    \value Full                  The battery is fully charged.
*/

/*!
    \enum QBatteryInfo::EnergyUnit
    This enum describes the energy unit used by the system.

    \value UnitUnknown            Energy unit unknown.
    \value UnitmAh                Energy described in milliamp-hour (mAh)
    \value UnitmWh                Energy described in milliwatt-hour (mWh)
*/

/*!
    \enum QBatteryInfo::BatteryStatus
    This enum describes the status of the battery.

    \value BatteryStatusUnknown      Battery level undetermined.
    \value BatteryEmpty              Battery is considered be empty and device needs to shut down.
    \value BatteryLow                Battery level is low and warnings need to be issued to the user.
    \value BatteryOk                 Battery level is Ok. It is above "Low" but not "Full".
    \value BatteryFull               Battery is fully charged.
*/

/*!
    \fn void QBatteryInfo::batteryCountChanged(int count);

    This signal is emitted when the number of batteries available has changed to \a count.
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
    in milliamperes (mA).
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
    \fn void QBatteryInfo::batteryStatusChanged(int battery, QBatteryInfo::BatteryStatus status);

    This signal is emitted when the battery status of the \a battery has changed to \a status.
*/

/*!
    Constructs a QBatteryInfo object with the given \a parent.
*/
QBatteryInfo::QBatteryInfo(QObject *parent)
    : QObject(parent)
#if !defined(QT_SIMULATOR)
    , d_ptr(new QBatteryInfoPrivate(this))
#else
    , d_ptr(new QBatteryInfoSimulator(this))
#endif // QT_SIMULATOR

{
}

/*!
    Destroys the object
*/
QBatteryInfo::~QBatteryInfo()
{
}

/*!
    \property QBatteryInfo::batteryCount
    \brief The number of the batteries available.

    Returns the number of batteries available, or -1 on error or the information is not available.
*/
int QBatteryInfo::batteryCount() const
{
    return d_ptr->batteryCount();
}

/*!
    Returns the current flow of the given \a battery, measured in milliamperes (mA). A positive
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

    To calculate capacity in percentage,
    (remainingCapacity(0) / maximumCapacity(0)) * 100
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
    \property QBatteryInfo::energyUnit
    \brief The used energy unit.

    Returns the energy unit that the system uses.
*/
QBatteryInfo::EnergyUnit QBatteryInfo::energyUnit() const
{
    return d_ptr->energyUnit();
}

/*!
    Returns the battery status of the given \a battery.
*/
QBatteryInfo::BatteryStatus QBatteryInfo::batteryStatus(int battery) const
{
    return d_ptr->batteryStatus(battery);
}

/*!
    \internal

    Returns the signal that corresponds to \a proxySignal in the
    meta-object of the \a sourceObject.
*/
QMetaMethod proxyToSourceSignal(const QMetaMethod &proxySignal, QObject *sourceObject)
{
    if (!proxySignal.isValid())
        return proxySignal;
    Q_ASSERT(proxySignal.methodType() == QMetaMethod::Signal);
    Q_ASSERT(sourceObject != 0);
    const QMetaObject *sourceMeta = sourceObject->metaObject();
    int sourceIndex = sourceMeta->indexOfSignal(proxySignal.methodSignature());
    Q_ASSERT(sourceIndex != -1);
    return sourceMeta->method(sourceIndex);
}

/*!
    \internal
*/
void QBatteryInfo::connectNotify(const QMetaMethod &signal)
{
#if defined(Q_OS_LINUX) || defined(Q_OS_WIN) || defined(QT_SIMULATOR) || defined(Q_OS_MAC)
    QMetaMethod sourceSignal = proxyToSourceSignal(signal, d_ptr);
    connect(d_ptr, sourceSignal, this, signal, Qt::UniqueConnection);
#else
    Q_UNUSED(signal)
#endif
}

/*!
    \internal
*/
void QBatteryInfo::disconnectNotify(const QMetaMethod &signal)
{
#if defined(Q_OS_LINUX) || defined(Q_OS_WIN) || defined(QT_SIMULATOR) || defined(Q_OS_MAC)
    // We can only disconnect with the private implementation, when there is no receivers for the signal.
    if (isSignalConnected(signal))
        return;

    QMetaMethod sourceSignal = proxyToSourceSignal(signal, d_ptr);
    disconnect(d_ptr, sourceSignal, this, signal);
#else
    Q_UNUSED(signal)
#endif
}

QT_END_NAMESPACE
