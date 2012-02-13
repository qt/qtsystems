/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsysteminfo_simulator_p.h"
#include "qsysteminfobackend_simulator_p.h"
#include "qsysteminfoconnection_simulator_p.h"

#if defined(Q_OS_LINUX) && !defined(QT_NO_JSONDB)
#  include "qdeviceinfo_linux_p.h"
#endif

#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
#  include "qnetworkinfo_linux_p.h"
#endif

QT_BEGIN_NAMESPACE

// QBatteryInfoSimulator

QBatteryInfoSimulator::QBatteryInfoSimulator(QBatteryInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
    , batteryInfoSimulatorBackend(QBatteryInfoSimulatorBackend::getSimulatorBackend())
{
    SystemInfoConnection::ensureSimulatorConnection();
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

QBatteryInfo::BatteryStatus QBatteryInfoSimulator::batteryStatus(int battery)
{
    if (batteryInfoSimulatorBackend)
        return batteryInfoSimulatorBackend->getBatteryStatus(battery);

    return QBatteryInfo::BatteryStatusUnknown;
}

void QBatteryInfoSimulator::connectNotify(const char *signal)
{
    if (batteryInfoSimulatorBackend && (strcmp(signal, SIGNAL(batteryCountChanged(int))) == 0
                                        || strcmp(signal, SIGNAL(currentFlowChanged(int,int))) == 0
                                        || strcmp(signal, SIGNAL(voltageChanged(int,int))) == 0
                                        || strcmp(signal, SIGNAL(remainingCapacityChanged(int,int))) == 0
                                        || strcmp(signal, SIGNAL(remainingChargingTimeChanged(int,int))) == 0
                                        || strcmp(signal, SIGNAL(chargerTypeChanged(QBatteryInfo::ChargerType))) == 0
                                        || strcmp(signal, SIGNAL(chargingStateChanged(int,QBatteryInfo::ChargingState))) == 0
                                        || strcmp(signal, SIGNAL(batteryStatusChanged(int,QBatteryInfo::BatteryStatus))) == 0)) {
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
                                        || strcmp(signal, SIGNAL(chargingStateChanged(int,QBatteryInfo::ChargingState))) == 0
                                        || strcmp(signal, SIGNAL(batteryStatusChanged(int,QBatteryInfo::BatteryStatus))) == 0)) {
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
    SystemInfoConnection::ensureSimulatorConnection();
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
    if (deviceInfoSimulatorBackend)
        return deviceInfoSimulatorBackend->getUniqueDeviceID();

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
    SystemInfoConnection::ensureSimulatorConnection();
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


// QNetworkInfoSimulator

QNetworkInfoSimulator::QNetworkInfoSimulator(QNetworkInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
    , networkInfoSimulatorBackend(QNetworkInfoSimulatorBackend::getSimulatorBackend())
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    , d_ptr(new QNetworkInfoPrivate())
#endif
{
    SystemInfoConnection::ensureSimulatorConnection();
}

QNetworkInfoSimulator::~QNetworkInfoSimulator()
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    delete d_ptr;
#endif
}

int QNetworkInfoSimulator::networkInterfaceCount(QNetworkInfo::NetworkMode mode)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    if (mode != QNetworkInfo::WlanMode) {
        if (d_ptr)
            return d_ptr->networkInterfaceCount(mode);
        else
            return -1;
    }
#endif
    if (networkInfoSimulatorBackend)
        return networkInfoSimulatorBackend->getNetworkInterfaceCount(mode);
    return -1;
}

int QNetworkInfoSimulator::networkSignalStrength(QNetworkInfo::NetworkMode mode, int interface)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    if (mode != QNetworkInfo::WlanMode) {
        if (d_ptr)
            return d_ptr->networkSignalStrength(mode, interface);
        else
            return -1;
    }
#endif
    if (networkInfoSimulatorBackend)
        return networkInfoSimulatorBackend->getNetworkSignalStrength(mode, interface);
    return -1;
}

QNetworkInfo::CellDataTechnology QNetworkInfoSimulator::currentCellDataTechnology(int interface)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
        if (d_ptr)
            return d_ptr->currentCellDataTechnology(interface);
        else
            return QNetworkInfo::UnknownDataTechnology;
#endif
    if (networkInfoSimulatorBackend)
        return networkInfoSimulatorBackend->getCurrentCellDataTechnology(interface);
    return QNetworkInfo::UnknownDataTechnology;
}

QNetworkInfo::NetworkMode QNetworkInfoSimulator::currentNetworkMode()
{
    QNetworkInfo::NetworkMode mode = QNetworkInfo::UnknownMode;
    if (networkInfoSimulatorBackend)
        mode = networkInfoSimulatorBackend->getCurrentNetworkMode();
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    QNetworkInfo::NetworkMode mode2 = QNetworkInfo::UnknownMode;
        if (d_ptr)
            mode2 = d_ptr->currentNetworkMode();
        switch (mode2) {
        case QNetworkInfo::WlanMode:
            if (mode != QNetworkInfo::EthernetMode)
                mode = mode2;
        default:
            break;
        }

#endif
    return mode;
}

QNetworkInfo::NetworkStatus QNetworkInfoSimulator::networkStatus(QNetworkInfo::NetworkMode mode, int interface)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    if (mode != QNetworkInfo::WlanMode) {
        if (d_ptr)
            return d_ptr->networkStatus(mode, interface);
        else
            return QNetworkInfo::UnknownStatus;
    }
#endif
    if (networkInfoSimulatorBackend)
        return networkInfoSimulatorBackend->getNetworkStatus(mode, interface);
    return QNetworkInfo::UnknownStatus;
}

QNetworkInterface QNetworkInfoSimulator::interfaceForMode(QNetworkInfo::NetworkMode mode, int interface)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    if (mode != QNetworkInfo::WlanMode) {
        if (d_ptr)
            return d_ptr->interfaceForMode(mode, interface);
        else
            return QNetworkInterface();
    }
#endif
    if (networkInfoSimulatorBackend)
        return networkInfoSimulatorBackend->getInterfaceForMode(mode, interface);
    return QNetworkInterface();
}

QString QNetworkInfoSimulator::cellId(int interface)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    if (d_ptr)
        return d_ptr->cellId(interface);
    else
        return QString();
#endif
    if (networkInfoSimulatorBackend)
        return networkInfoSimulatorBackend->getCellId(interface);
    return QString();
}

QString QNetworkInfoSimulator::currentMobileCountryCode(int interface)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    if (d_ptr)
        return d_ptr->currentMobileCountryCode(interface);
    else
        return QString();
#endif
    if (networkInfoSimulatorBackend)
        return networkInfoSimulatorBackend->getCurrentMobileCountryCode(interface);
    return QString();
}

QString QNetworkInfoSimulator::currentMobileNetworkCode(int interface)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    if (d_ptr)
        return d_ptr->currentMobileNetworkCode(interface);
    else
        return QString();
#endif
    if (networkInfoSimulatorBackend)
        return networkInfoSimulatorBackend->getCurrentMobileNetworkCode(interface);
    return QString();
}

QString QNetworkInfoSimulator::homeMobileCountryCode(int interface)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    if (d_ptr)
        return d_ptr->homeMobileCountryCode(interface);
    else
        return QString();
#endif
    if (networkInfoSimulatorBackend)
        return networkInfoSimulatorBackend->getHomeMobileCountryCode(interface);
    return QString();
}

QString QNetworkInfoSimulator::homeMobileNetworkCode(int interface)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    if (d_ptr)
        return d_ptr->homeMobileNetworkCode(interface);
    else
        return QString();
#endif
    if (networkInfoSimulatorBackend)
        return networkInfoSimulatorBackend->getHomeMobileNetworkCode(interface);
    return QString();
}

QString QNetworkInfoSimulator::imsi(int interface)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    if (d_ptr)
        return d_ptr->imsi(interface);
    else
        return QString();
#endif
    if (networkInfoSimulatorBackend)
        return networkInfoSimulatorBackend->getImsi(interface);
    return QString();
}

QString QNetworkInfoSimulator::locationAreaCode(int interface)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    if (d_ptr)
        return d_ptr->locationAreaCode(interface);
    else
        return QString();
#endif
    if (networkInfoSimulatorBackend)
        return networkInfoSimulatorBackend->getLocationAreaCode(interface);
    return QString();
}

QString QNetworkInfoSimulator::macAddress(QNetworkInfo::NetworkMode mode, int interface)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    if (mode != QNetworkInfo::WlanMode) {
        if (d_ptr)
            return d_ptr->macAddress(mode, interface);
        else
            return QString();
    }
#endif
    if (networkInfoSimulatorBackend)
        return networkInfoSimulatorBackend->getMacAddress(mode, interface);
    return QString();
}

QString QNetworkInfoSimulator::networkName(QNetworkInfo::NetworkMode mode, int interface)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    if (mode != QNetworkInfo::WlanMode) {
        if (d_ptr)
            return d_ptr->networkName(mode, interface);
        else
            return QString();
    }
#endif
    if (networkInfoSimulatorBackend)
        return networkInfoSimulatorBackend->getNetworkName(mode, interface);
    return QString();
}

void QNetworkInfoSimulator::connectNotify(const char *signal)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    if (strcmp(signal, SIGNAL(networkInterfaceCountChanged(QNetworkInfo::NetworkMode,int))) == 0) {
        if (networkInfoSimulatorBackend)
            connect(networkInfoSimulatorBackend, signal, this, SLOT(onNetworkInterfaceCountChanged(QNetworkInfo::NetworkMode,int)), Qt::UniqueConnection);
        if (d_ptr)
            connect(d_ptr, signal, this, SLOT(onNetworkInterfaceCountChanged(QNetworkInfo::NetworkMode,int)), Qt::UniqueConnection);
        return;
    } else if (strcmp(signal, SIGNAL(currentNetworkModeChanged(QNetworkInfo::NetworkMode)))  == 0) {
        if (networkInfoSimulatorBackend)
            connect(networkInfoSimulatorBackend, signal, this, SLOT(onCurrentNetworkModeChanged(QNetworkInfo::NetworkMode)), Qt::UniqueConnection);
        if (d_ptr)
            connect(d_ptr, signal, this, SLOT(onCurrentNetworkModeChanged(QNetworkInfo::NetworkMode)), Qt::UniqueConnection);
        return;
    } else if (strcmp(signal, SIGNAL(networkNameChanged(QNetworkInfo::NetworkMode,int,QString)))  == 0) {
        if (networkInfoSimulatorBackend)
            connect(networkInfoSimulatorBackend, signal, this, SLOT(onNetworkNameChanged(QNetworkInfo::NetworkMode,int,QString)), Qt::UniqueConnection);
        if (d_ptr)
            connect(d_ptr, signal, this, SLOT(onNetworkNameChanged(QNetworkInfo::NetworkMode,int,QString)), Qt::UniqueConnection);
        return;
    } else if (strcmp(signal, SIGNAL(networkSignalStrengthChanged(QNetworkInfo::NetworkMode,int,int)))  == 0) {
        if (networkInfoSimulatorBackend)
            connect(networkInfoSimulatorBackend, signal, this, SLOT(onNetworkSignalStrengthChanged(QNetworkInfo::NetworkMode,int,int)), Qt::UniqueConnection);
        if (d_ptr)
            connect(d_ptr, signal, this, SLOT(onNetworkSignalStrengthChanged(QNetworkInfo::NetworkMode,int,int)), Qt::UniqueConnection);
        return;
    } else if (strcmp(signal, SIGNAL(networkStatusChanged(QNetworkInfo::NetworkMode,int,QNetworkInfo::NetworkStatus)))  == 0) {
        if (networkInfoSimulatorBackend)
            connect(networkInfoSimulatorBackend, signal, this, SLOT(onNetworkStatusChanged(QNetworkInfo::NetworkMode,int,QNetworkInfo::NetworkStatus)), Qt::UniqueConnection);
        if (d_ptr)
            connect(d_ptr, signal, this, SLOT(onNetworkStatusChanged(QNetworkInfo::NetworkMode,int,QNetworkInfo::NetworkStatus)), Qt::UniqueConnection);
        return;
    }
#endif
    if (networkInfoSimulatorBackend)
        connect(networkInfoSimulatorBackend, signal, this, signal, Qt::UniqueConnection);
}

void QNetworkInfoSimulator::disconnectNotify(const char *signal)
{
#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)
    if (strcmp(signal, SIGNAL(networkInterfaceCountChanged(QNetworkInfo::NetworkMode,int))) == 0) {
        if (networkInfoSimulatorBackend)
            disconnect(networkInfoSimulatorBackend, signal, this, SLOT(onNetworkInterfaceCountChanged(QNetworkInfo::NetworkMode,int)));
        if (d_ptr)
            disconnect(d_ptr, signal, this, SLOT(onNetworkInterfaceCountChanged(QNetworkInfo::NetworkMode,int)));
        return;
    } else if (strcmp(signal, SIGNAL(currentNetworkModeChanged(QNetworkInfo::NetworkMode)))  == 0) {
        if (networkInfoSimulatorBackend)
            disconnect(networkInfoSimulatorBackend, signal, this, SLOT(onCurrentNetworkModeChanged(QNetworkInfo::NetworkMode)));
        if (d_ptr)
            disconnect(d_ptr, signal, this, SLOT(onCurrentNetworkModeChanged(QNetworkInfo::NetworkMode)));
        return;
    } else if (strcmp(signal, SIGNAL(networkNameChanged(QNetworkInfo::NetworkMode,int,QString)))  == 0) {
        if (networkInfoSimulatorBackend)
            disconnect(networkInfoSimulatorBackend, signal, this, SLOT(onNetworkNameChanged(QNetworkInfo::NetworkMode,int,QString)));
        if (d_ptr)
            disconnect(d_ptr, signal, this, SLOT(onNetworkNameChanged(QNetworkInfo::NetworkMode,int,QString)));
        return;
    } else if (strcmp(signal, SIGNAL(networkSignalStrengthChanged(QNetworkInfo::NetworkMode,int,int)))  == 0) {
        if (networkInfoSimulatorBackend)
            disconnect(networkInfoSimulatorBackend, signal, this, SLOT(onNetworkSignalStrengthChanged(QNetworkInfo::NetworkMode,int,int)));
        if (d_ptr)
            disconnect(d_ptr, signal, this, SLOT(onNetworkSignalStrengthChanged(QNetworkInfo::NetworkMode,int,int)));
        return;
    } else if (strcmp(signal, SIGNAL(networkStatusChanged(QNetworkInfo::NetworkMode,int,QNetworkInfo::NetworkStatus)))  == 0) {
        if (networkInfoSimulatorBackend)
            disconnect(networkInfoSimulatorBackend, signal, this, SLOT(onNetworkStatusChanged(QNetworkInfo::NetworkMode,int,QNetworkInfo::NetworkStatus)));
        if (d_ptr)
            disconnect(d_ptr, signal, this, SLOT(onNetworkStatusChanged(QNetworkInfo::NetworkMode,int,QNetworkInfo::NetworkStatus)));
        return;
    }
#endif
    if (networkInfoSimulatorBackend)
        disconnect(networkInfoSimulatorBackend, signal, this, signal);
}

#if !defined(QT_NO_SFW_NETREG) || !defined(QT_NO_OFONO)

void QNetworkInfoSimulator::onCurrentNetworkModeChanged(QNetworkInfo::NetworkMode mode)
{
    Q_UNUSED(mode)
    emit currentNetworkModeChanged(currentNetworkMode());
}

void QNetworkInfoSimulator::onNetworkInterfaceCountChanged(QNetworkInfo::NetworkMode mode, int count)
{
    if (strcmp(sender()->metaObject()->className(), "QNetworkInfoSimulatorBackend") == 0) {
        if (mode == QNetworkInfo::WlanMode)
            emit networkInterfaceCountChanged(mode, count);
    } else {
        if (mode != QNetworkInfo::WlanMode)
            emit networkInterfaceCountChanged(mode, count);
    }
}

void QNetworkInfoSimulator::onNetworkNameChanged(QNetworkInfo::NetworkMode mode, int interface, const QString &name)
{
    if (strcmp(sender()->metaObject()->className(), "QNetworkInfoSimulatorBackend") == 0) {
        if (mode == QNetworkInfo::WlanMode)
            emit networkNameChanged(mode, interface, name);
    } else {
        if (mode != QNetworkInfo::WlanMode)
            emit networkNameChanged(mode, interface, name);
    }
}

void QNetworkInfoSimulator::onNetworkSignalStrengthChanged(QNetworkInfo::NetworkMode mode, int interface, int strength)
{
    if (strcmp(sender()->metaObject()->className(), "QNetworkInfoSimulatorBackend") == 0) {
        if (mode == QNetworkInfo::WlanMode)
            emit networkSignalStrengthChanged(mode, interface, strength);
    } else {
        if (mode != QNetworkInfo::WlanMode)
            emit networkSignalStrengthChanged(mode, interface, strength);
    }
}

void QNetworkInfoSimulator::onNetworkStatusChanged(QNetworkInfo::NetworkMode mode, int interface, QNetworkInfo::NetworkStatus status)
{
    if (strcmp(sender()->metaObject()->className(), "QNetworkInfoSimulatorBackend") == 0) {
        if (mode == QNetworkInfo::WlanMode)
            emit networkStatusChanged(mode, interface, status);
    } else {
        if (mode != QNetworkInfo::WlanMode)
            emit networkStatusChanged(mode, interface, status);
    }
}

#endif

QT_END_NAMESPACE
