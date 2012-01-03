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

#include "qsysteminfo_simulator_p.h"
#include "qsysteminfobackend_simulator_p.h"

#if defined(Q_OS_LINUX) && !defined(QT_NO_JSONDB)
#  include "qdeviceinfo_linux_p.h"
#endif

QT_BEGIN_NAMESPACE


// QBatteryInfoSimulator

QBatteryInfoSimulator::QBatteryInfoSimulator(QBatteryInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
    , batteryInfoSimulatorBackend(QBatteryInfoSimulatorBackend::getSimulatorBackend())
{
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

int QBatteryInfoSimulator::currentFlow(int battery)
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getCurrentFlow(battery);

    return 0;
}

int QBatteryInfoSimulator::maximumCapacity(int battery)
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getMaximumCapacity(battery);

    return -1;
}

int QBatteryInfoSimulator::remainingCapacity(int battery)
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getRemainingCapacity(battery);

    return -1;
}

int QBatteryInfoSimulator::remainingChargingTime(int battery)
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getRemainingChargingTime(battery);

    return -1;
}

int QBatteryInfoSimulator::voltage(int battery)
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getVoltage(battery);

    return -1;
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

QBatteryInfo::EnergyUnit QBatteryInfoSimulator::energyUnit()
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getEnergyUnit();

    return QBatteryInfo::UnitUnknown;
}

void QBatteryInfoSimulator::connectNotify(const char *signal)
{
    if (batteryInfoSimulatorBackend && (strcmp(signal, SIGNAL(batteryCountChanged(int))) == 0
                                        || strcmp(signal, SIGNAL(currentFlowChanged(int,int))) == 0
                                        || strcmp(signal, SIGNAL(voltageChanged(int,int))) == 0
                                        || strcmp(signal, SIGNAL(remainingCapacityChanged(int,int))) == 0
                                        || strcmp(signal, SIGNAL(remainingChargingTimeChanged(int,int))) == 0
                                        || strcmp(signal, SIGNAL(chargerTypeChanged(QBatteryInfo::ChargerType))) == 0
                                        || strcmp(signal, SIGNAL(chargingStateChanged(int,QBatteryInfo::ChargingState))) == 0)) {
        connect(batteryInfoSimulatorBackend, signal, this, signal);
    }
}

void QBatteryInfoSimulator::disconnectNotify(const char *signal)
{
    if (batteryInfoSimulatorBackend && (strcmp(signal, SIGNAL(batteryCountChanged(int))) == 0
                                        || strcmp(signal, SIGNAL(currentFlowChanged(int,int))) == 0
                                        || strcmp(signal, SIGNAL(voltageChanged(int,int))) == 0
                                        || strcmp(signal, SIGNAL(remainingCapacityChanged(int,int))) == 0
                                        || strcmp(signal, SIGNAL(remainingChargingTimeChanged(int,int))) == 0
                                        || strcmp(signal, SIGNAL(chargerTypeChanged(QBatteryInfo::ChargerType))) == 0
                                        || strcmp(signal, SIGNAL(chargingStateChanged(int,QBatteryInfo::ChargingState))) == 0)) {
        disconnect(batteryInfoSimulatorBackend, signal, this, signal);
    }
}


// QDeviceInfoSimulator


QDeviceInfoSimulator::QDeviceInfoSimulator(QDeviceInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
    , deviceInfoSimulatorBackend(QDeviceInfoSimulatorBackend::getSimulatorBackend())
#if defined(Q_OS_LINUX) && !defined(QT_NO_JSONDB)
    , d_ptr(new QDeviceInfoPrivate())
#endif
{
}

QDeviceInfoSimulator::~QDeviceInfoSimulator()
{
#if defined(Q_OS_LINUX) && !defined(QT_NO_JSONDB)
    delete d_ptr;
#endif
}

bool QDeviceInfoSimulator::hasFeature(QDeviceInfo::Feature feature)
{
    if (deviceInfoSimulatorBackend)
        return deviceInfoSimulatorBackend->hasFeature(feature);

    return false;
}

QDeviceInfo::LockTypeFlags QDeviceInfoSimulator::activatedLocks()
{
#if defined(Q_OS_LINUX) && !defined(QT_NO_JSONDB)
    if (d_ptr)
        return d_ptr->activatedLocks();
#else

    if (deviceInfoSimulatorBackend)
        return deviceInfoSimulatorBackend->getActivatedLocks();
#endif

    return QDeviceInfo::NoLock;
}

QDeviceInfo::LockTypeFlags QDeviceInfoSimulator::enabledLocks()
{
#if defined(Q_OS_LINUX) && !defined(QT_NO_JSONDB)
    if (d_ptr)
        return d_ptr->enabledLocks();
#else

    if (deviceInfoSimulatorBackend)
        return deviceInfoSimulatorBackend->getEnabledLocks();
#endif

    return QDeviceInfo::NoLock;
}

QDeviceInfo::ThermalState QDeviceInfoSimulator::thermalState()
{
    if (deviceInfoSimulatorBackend)
        return deviceInfoSimulatorBackend->getThermalState();

    return QDeviceInfo::UnknownThermal;
}

int QDeviceInfoSimulator::imeiCount()
{
    if (deviceInfoSimulatorBackend)
        return deviceInfoSimulatorBackend->getImeiCount();

    return -1;
}

QString QDeviceInfoSimulator::imei(int interface)
{
    if (deviceInfoSimulatorBackend)
        return deviceInfoSimulatorBackend->getImei(interface);

    return QString();
}

QString QDeviceInfoSimulator::manufacturer()
{
    if (deviceInfoSimulatorBackend)
        return deviceInfoSimulatorBackend->getManufacturer();

    return QString();
}

QString QDeviceInfoSimulator::model()
{
    if (deviceInfoSimulatorBackend)
        return deviceInfoSimulatorBackend->getModel();

    return QString();
}

QString QDeviceInfoSimulator::productName()
{
    if (deviceInfoSimulatorBackend)
        return deviceInfoSimulatorBackend->getProductName();

    return QString();
}

QString QDeviceInfoSimulator::uniqueDeviceID()
{
#if defined(Q_OS_LINUX) && !defined(QT_NO_JSONDB)
    if (d_ptr)
        return d_ptr->uniqueDeviceID();
#else

    if (deviceInfoSimulatorBackend)
        return deviceInfoSimulatorBackend->getUniqueDeviceID();
#endif

    return QString();
}

QString QDeviceInfoSimulator::version(QDeviceInfo::Version type)
{
#if defined(Q_OS_LINUX) && !defined(QT_NO_JSONDB)
    if (d_ptr)
        return d_ptr->version(type);
#else

    if (deviceInfoSimulatorBackend)
        return deviceInfoSimulatorBackend->getVersion(type);
#endif

    return QString();
}

void QDeviceInfoSimulator::connectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(activatedLocksChanged(QDeviceInfo::LockTypeFlags))) == 0
            || strcmp(signal, SIGNAL(enabledLocksChanged(QDeviceInfo::LockTypeFlags))) == 0) {
#if defined(Q_OS_LINUX) && !defined(QT_NO_JSONDB)
        if (d_ptr)
            connect(d_ptr, signal, this, signal, Qt::UniqueConnection);
        return;
#else
        if (deviceInfoSimulatorBackend)
            connect(deviceInfoSimulatorBackend, signal, this, signal);
        return;
#endif
    }

    if (deviceInfoSimulatorBackend && strcmp(signal, SIGNAL(thermalStateChanged(QDeviceInfo::ThermalState state))) == 0)
        connect(deviceInfoSimulatorBackend, signal, this, signal);
}

void QDeviceInfoSimulator::disconnectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(activatedLocksChanged(QDeviceInfo::LockTypeFlags))) == 0
            || strcmp(signal, SIGNAL(enabledLocksChanged(QDeviceInfo::LockTypeFlags))) == 0) {
#if defined(Q_OS_LINUX) && !defined(QT_NO_JSONDB)
        if (d_ptr)
            disconnect(d_ptr, signal, this, signal);
        return;
#else
        if (deviceInfoSimulatorBackend)
            disconnect(deviceInfoSimulatorBackend, signal, this, signal);
        return;
#endif
    }

    if (deviceInfoSimulatorBackend && strcmp(signal, SIGNAL(thermalStateChanged(QDeviceInfo::ThermalState state))) == 0)
        disconnect(deviceInfoSimulatorBackend, signal, this, signal);
}


// QStorageInfoSimulator

QStorageInfoSimulator::QStorageInfoSimulator(QStorageInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
    , storageInfoSimulatorBackend(QStorageInfoSimulatorBackend::getSimulatorBackend())
{
}

QStorageInfoSimulator::~QStorageInfoSimulator()
{
}

qlonglong QStorageInfoSimulator::availableDiskSpace(const QString &drive)
{
    if (storageInfoSimulatorBackend)
        return storageInfoSimulatorBackend->getAvailableDiskSpace(drive);

    return -1;
}

qlonglong QStorageInfoSimulator::totalDiskSpace(const QString &drive)
{
    if (storageInfoSimulatorBackend)
        return storageInfoSimulatorBackend->getTotalDiskSpace(drive);

    return -1;
}

QString QStorageInfoSimulator::uriForDrive(const QString &drive)
{
    if (storageInfoSimulatorBackend)
        return storageInfoSimulatorBackend->getUriForDrive(drive);

    return QString();
}

QStringList QStorageInfoSimulator::allLogicalDrives()
{
    if (storageInfoSimulatorBackend)
        return storageInfoSimulatorBackend->getAllLogicalDrives();

    return QStringList();
}

QStorageInfo::DriveType QStorageInfoSimulator::driveType(const QString &drive)
{
    if (storageInfoSimulatorBackend)
        return storageInfoSimulatorBackend->getDriveType(drive);

    return QStorageInfo::UnknownDrive;
}
void QStorageInfoSimulator::connectNotify(const char *signal)
{
    if (storageInfoSimulatorBackend && strcmp(signal, SIGNAL(logicalDriveChanged(const QString, bool))) == 0)
        connect(storageInfoSimulatorBackend, signal, this, signal);
}

void QStorageInfoSimulator::disconnectNotify(const char *signal)
{
    if (storageInfoSimulatorBackend && strcmp(signal, SIGNAL(logicalDriveChanged(const QString, bool))) == 0)
        disconnect(storageInfoSimulatorBackend, signal, this, signal);
}

QT_END_NAMESPACE
