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

#include "qsysteminfobackend_simulator_p.h"
#include "qsysteminfoconnection_simulator_p.h"

#include <QMutex>

QT_BEGIN_NAMESPACE

QBatteryInfoSimulatorBackend *QBatteryInfoSimulatorBackend::globalSimulatorBackend = 0;
QDeviceInfoSimulatorBackend *QDeviceInfoSimulatorBackend::globalSimulatorBackend = 0;
QStorageInfoSimulatorBackend *QStorageInfoSimulatorBackend::globalSimulatorBackend = 0;


// QBatteryInfoSimulatorBackend

QBatteryInfoSimulatorBackend::QBatteryInfoSimulatorBackend(QObject *parent)
    : QObject(parent)
{
    SystemInfoConnection::ensureSimulatorConnection();

    data.currentFlow = 0;
    data.maximumCapacity = -1;
    data.remainingCapacity = -1;
    data.remainingChargingTime = -1;
    data.voltage = -1;
    data.chargingState = QBatteryInfo::UnknownChargingState;
    data.chargerType = QBatteryInfo::UnknownCharger;
    data.energyMeasurementUnit = QBatteryInfo::UnitUnknown;
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

int QBatteryInfoSimulatorBackend::getCurrentFlow(int battery)
{
    if (battery == 0)
        return data.currentFlow;
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

QBatteryInfo::EnergyUnit QBatteryInfoSimulatorBackend::getEnergyUnit()
{
    return data.energyMeasurementUnit;
}

void QBatteryInfoSimulatorBackend::setCurrentFlow(int flow)
{
    if (data.currentFlow != flow) {
        data.currentFlow = flow;
        emit currentFlowChanged(0, flow);
    }
}

void QBatteryInfoSimulatorBackend::setMaximumCapacity(int capacity)
{
    if (data.maximumCapacity != capacity)
        data.maximumCapacity = capacity;
}

void QBatteryInfoSimulatorBackend::setRemainingCapacity(int capacity)
{
    if (data.remainingCapacity != capacity) {
        data.remainingCapacity = capacity;
        emit remainingCapacityChanged(0, capacity);
    }
}

void QBatteryInfoSimulatorBackend::setVoltage(int vol)
{
    if (data.voltage != vol) {
        data.voltage = vol;
        emit voltageChanged(0, vol);
    }
}

void QBatteryInfoSimulatorBackend::setRemainingChargingTime(int time)
{
    if (data.remainingChargingTime != time) {
        data.remainingChargingTime = time;
        emit remainingChargingTimeChanged(0, time);
    }
}

void QBatteryInfoSimulatorBackend::setChargingState(QBatteryInfo::ChargingState state)
{
    if (data.chargingState != state) {
        data.chargingState = state;
        emit chargingStateChanged(0, state);
    }
}

void QBatteryInfoSimulatorBackend::setChargerType(QBatteryInfo::ChargerType type)
{
    if (data.chargerType != type) {
        data.chargerType = type;
        emit chargerTypeChanged(type);
    }
}

void QBatteryInfoSimulatorBackend::setEnergyUnit(QBatteryInfo::EnergyUnit unit)
{
    if (data.energyMeasurementUnit != unit)
        data.energyMeasurementUnit = unit;
}


// QDeviceInfoSimulatorBackend

QDeviceInfoSimulatorBackend::QDeviceInfoSimulatorBackend(QObject *parent)
    : QObject(parent)
{
    SystemInfoConnection::ensureSimulatorConnection();

    initFeatureList();
    initImeiList();
    initVersionList();
}

QDeviceInfoSimulatorBackend::~QDeviceInfoSimulatorBackend()
{
}

QDeviceInfoSimulatorBackend *QDeviceInfoSimulatorBackend::getSimulatorBackend()
{
    static QMutex mutex;

    mutex.lock();
    if (!globalSimulatorBackend)
        globalSimulatorBackend = new QDeviceInfoSimulatorBackend();
    mutex.unlock();

    return globalSimulatorBackend;
}

void QDeviceInfoSimulatorBackend::initFeatureList()
{
    data.featureList.insert(QDeviceInfo::Bluetooth, false);
    data.featureList.insert(QDeviceInfo::Camera, false);
    data.featureList.insert(QDeviceInfo::FmRadio, false);
    data.featureList.insert(QDeviceInfo::FmTransmitter, false);
    data.featureList.insert(QDeviceInfo::Infrared, false);
    data.featureList.insert(QDeviceInfo::Led, false);
    data.featureList.insert(QDeviceInfo::MemoryCard, false);
    data.featureList.insert(QDeviceInfo::Usb, false);
    data.featureList.insert(QDeviceInfo::Vibration, false);
    data.featureList.insert(QDeviceInfo::Wlan, false);
    data.featureList.insert(QDeviceInfo::Sim, false);
    data.featureList.insert(QDeviceInfo::Positioning, false);
    data.featureList.insert(QDeviceInfo::VideoOut, false);
    data.featureList.insert(QDeviceInfo::Haptics, false);
    data.featureList.insert(QDeviceInfo::Nfc, false);
}

void QDeviceInfoSimulatorBackend::initImeiList()
{
    data.imeiList.insert(0, QStringLiteral("IMEI 0"));
    data.imeiList.insert(0, QStringLiteral("IMEI 1"));
}

void QDeviceInfoSimulatorBackend::initVersionList()
{
    data.versionList[QDeviceInfo::Os] = QStringLiteral("1.0");
    data.versionList[QDeviceInfo::Firmware] = QStringLiteral("1.0");
}

bool QDeviceInfoSimulatorBackend::hasFeature(QDeviceInfo::Feature feature)
{
    QHash<QDeviceInfo::Feature, bool>::const_iterator i = data.featureList.find(feature);
    if (i != data.featureList.end())
        return i.value();

    return false;
}

int QDeviceInfoSimulatorBackend::getImeiCount()
{
    return data.imeiList.count();
}

QDeviceInfo::LockTypeFlags QDeviceInfoSimulatorBackend::getActivatedLocks()
{
    return data.activatedLocks;
}

QDeviceInfo::LockTypeFlags QDeviceInfoSimulatorBackend::getEnabledLocks()
{
    return data.enabledLocks;
}

QDeviceInfo::ThermalState QDeviceInfoSimulatorBackend::getThermalState()
{
    return data.currentThermalState;
}

QString QDeviceInfoSimulatorBackend::getImei(int num)
{
    if (num >= 0 && num < data.imeiList.count())
        return data.imeiList.at(num);
    return QString();
}

QString QDeviceInfoSimulatorBackend::getManufacturer()
{
    return data.manufacturer;
}

QString QDeviceInfoSimulatorBackend::getModel()
{
    return data.model;
}

QString QDeviceInfoSimulatorBackend::getProductName()
{
    return data.productName;
}

QString QDeviceInfoSimulatorBackend::getUniqueDeviceID()
{
    return data.uniqueDeviceID;
}

QString QDeviceInfoSimulatorBackend::getVersion(QDeviceInfo::Version version)
{
    QMap<QDeviceInfo::Version, QString>::const_iterator i = data.versionList.find(version);
    if (i != data.versionList.end())
        return i.value();

    return QString();
}

void QDeviceInfoSimulatorBackend::setFeature(QDeviceInfo::Feature feature, bool enable)
{
    QHash<QDeviceInfo::Feature, bool>::iterator i = data.featureList.find(feature);
    if (i != data.featureList.end() && i.value() != enable)
        data.featureList[feature] = enable;
}

void QDeviceInfoSimulatorBackend::setActivatedLocks(QDeviceInfo::LockTypeFlags flag)
{
    if (data.activatedLocks != flag) {
        data.activatedLocks = flag;
        emit activatedLocksChanged(flag);
    }
}

void QDeviceInfoSimulatorBackend::setEnabledLocks(QDeviceInfo::LockTypeFlags flag)
{
    if (data.enabledLocks != flag) {
        data.enabledLocks = flag;
        emit enabledLocksChanged(flag);
    }
}

void QDeviceInfoSimulatorBackend::setThermalState(QDeviceInfo::ThermalState state)
{
    if (data.currentThermalState != state) {
        data.currentThermalState = state;
        emit thermalStateChanged(state);
    }

}

void QDeviceInfoSimulatorBackend::setImei(int num, QString imei)
{
    if (num >= 0 && num < data.imeiList.count()) {
        if (data.imeiList[num] != imei)
            data.imeiList[num] = imei;
    }
}

void QDeviceInfoSimulatorBackend::setManufacturer(QString manufacturer)
{
    if (data.manufacturer != manufacturer)
        data.manufacturer = manufacturer;
}

void QDeviceInfoSimulatorBackend::setModel(QString model)
{
    if (data.model != model)
        data.model = model;
}

void QDeviceInfoSimulatorBackend::setProductName(QString name)
{
    if (data.productName != name)
        data.productName = name;
}

void QDeviceInfoSimulatorBackend::setUniqueDeviceID(QString id)
{
    if (data.uniqueDeviceID != id)
        data.uniqueDeviceID = id;
}

void QDeviceInfoSimulatorBackend::setVersion(QDeviceInfo::Version version, QString versionString)
{
    QMap<QDeviceInfo::Version, QString>::iterator i = data.versionList.find(version);
    if (i != data.versionList.end() && i.value() != versionString)
        data.versionList[version] = versionString;
}


// QStorageInfoSimulatorBackend

QStorageInfoSimulatorBackend::QStorageInfoSimulatorBackend(QObject *parent)
    : QObject(parent)
{
    SystemInfoConnection::ensureSimulatorConnection();
}

QStorageInfoSimulatorBackend::~QStorageInfoSimulatorBackend()
{
}

QStorageInfoSimulatorBackend *QStorageInfoSimulatorBackend::getSimulatorBackend()
{
    static QMutex mutex;

    mutex.lock();
    if (!globalSimulatorBackend)
        globalSimulatorBackend = new QStorageInfoSimulatorBackend();
    mutex.unlock();

    return globalSimulatorBackend;
}

qlonglong QStorageInfoSimulatorBackend::getAvailableDiskSpace(const QString &drive)
{
    return getDriveInfo(drive).availableSpace;
}

qlonglong QStorageInfoSimulatorBackend::getTotalDiskSpace(const QString &drive)
{
    return getDriveInfo(drive).totalSpace;
}

QString QStorageInfoSimulatorBackend::getUriForDrive(const QString &drive)
{
    return getDriveInfo(drive).uri;
}

QStringList QStorageInfoSimulatorBackend::getAllLogicalDrives()
{
    QStringList driveList;
    driveList << data.drives.keys();

    return driveList;
}

QStorageInfo::DriveType QStorageInfoSimulatorBackend::getDriveType(const QString &drive)
{
    return getDriveInfo(drive).type;
}

void QStorageInfoSimulatorBackend::setAvailableDiskSpace(const QString &drive, qlonglong space)
{
    if (hasDriveInfo(drive) &&  getDriveInfo(drive).availableSpace != space)
        data.drives[drive].availableSpace = space;
}

void QStorageInfoSimulatorBackend::setTotalDiskSpace(const QString &drive, qlonglong space)
{
    if (hasDriveInfo(drive) &&  getDriveInfo(drive).totalSpace != space)
        data.drives[drive].totalSpace = space;
}

void QStorageInfoSimulatorBackend::setUriForDrive(const QString &drive, QString uri)
{
    if (hasDriveInfo(drive) &&  getDriveInfo(drive).uri != uri)
        data.drives[drive].uri = uri;
}

void QStorageInfoSimulatorBackend::setDriveType(const QString &drive, QStorageInfo::DriveType type)
{
    if (hasDriveInfo(drive) &&  getDriveInfo(drive).type != type)
        data.drives[drive].type = type;
}

bool QStorageInfoSimulatorBackend::addDrive(const QString &drive)
{
    return addDrive(drive, QStorageInfo::UnknownDrive, 0, 0, QString());
}

bool QStorageInfoSimulatorBackend::addDrive(const QString &drive, QStorageInfo::DriveType type,
                                            qint64 totalSpace, qint64 availableSpace,
                                            const QString &uri)
{
    QHash<QString, QStorageInfoData::DriveInfo>::const_iterator it = data.drives.find(drive);
    if (it != data.drives.end())
        return false;

    QStorageInfoData::DriveInfo d;
    d.type = static_cast<QStorageInfo::DriveType>(type);
    d.totalSpace = totalSpace;
    d.availableSpace = availableSpace;
    d.uri = uri;

    data.drives[drive] = d;
    emit logicalDriveChanged(drive, true);

    return true;
}

bool QStorageInfoSimulatorBackend::removeDrive(const QString &drive)
{
    if (data.drives.remove(drive) > 0) {
        emit logicalDriveChanged(drive, false);
        return true;
    }
    return false;
}

QStorageInfoData::DriveInfo QStorageInfoSimulatorBackend::getDriveInfo(const QString &drive)
{
    QHash<QString, QStorageInfoData::DriveInfo>::const_iterator i = data.drives.find(drive);
    if (i != data.drives.end())
        return i.value();
    else {
        struct QStorageInfoData::DriveInfo info = {-1, -1, QString(), QStorageInfo::UnknownDrive};
        return info;
    }
}

bool QStorageInfoSimulatorBackend::hasDriveInfo(const QString &drive)
{
    QHash<QString, QStorageInfoData::DriveInfo>::const_iterator i = data.drives.find(drive);
    if (i != data.drives.end())
        return true;

    return false;
}

QT_END_NAMESPACE
