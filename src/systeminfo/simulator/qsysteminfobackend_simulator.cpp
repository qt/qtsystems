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

#include "qsysteminfobackend_simulator_p.h"

#include <QMutex>

QT_BEGIN_NAMESPACE

QBatteryInfoSimulatorBackend *QBatteryInfoSimulatorBackend::globalSimulatorBackend = 0;
QDeviceInfoSimulatorBackend *QDeviceInfoSimulatorBackend::globalSimulatorBackend = 0;
QNetworkInfoSimulatorBackend *QNetworkInfoSimulatorBackend::globalSimulatorBackend = 0;


// QBatteryInfoSimulatorBackend

QBatteryInfoSimulatorBackend::QBatteryInfoSimulatorBackend(QObject *parent)
    : QObject(parent)
{
    data.currentFlow = 0;
    data.maximumCapacity = -1;
    data.remainingCapacity = -1;
    data.remainingChargingTime = -1;
    data.voltage = -1;
    data.chargingState = QBatteryInfo::UnknownChargingState;
    data.chargerType = QBatteryInfo::UnknownCharger;
    data.energyMeasurementUnit = QBatteryInfo::UnitUnknown;
    data.batteryStatus = QBatteryInfo::BatteryStatusUnknown;
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

QBatteryInfo::BatteryStatus QBatteryInfoSimulatorBackend::getBatteryStatus(int battery)
{
    if (battery == 0)
        return data.batteryStatus;

    return QBatteryInfo::BatteryStatusUnknown;
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

void QBatteryInfoSimulatorBackend::setBatteryStatus(QBatteryInfo::BatteryStatus status)
{
    if (data.batteryStatus != status) {
        data.batteryStatus = status;
        emit batteryStatusChanged(0, status);
    }
}


// QDeviceInfoSimulatorBackend

QDeviceInfoSimulatorBackend::QDeviceInfoSimulatorBackend(QObject *parent)
    : QObject(parent)
{
    initFeatureList();
    initImeiList();
    initVersionList();
    data.enabledLocks = QDeviceInfo::NoLock;
    data.activatedLocks = QDeviceInfo::NoLock;
    data.currentThermalState = QDeviceInfo::UnknownThermal;
    data.manufacturer = QStringLiteral("Simulator Manufacturer");
    data.model = QStringLiteral("Simulator Model");
    data.productName =  QStringLiteral("Simulator Product Name");
    data.uniqueDeviceID =  QStringLiteral("");
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
    data.featureList.insert(QDeviceInfo::BluetoothFeature, false);
    data.featureList.insert(QDeviceInfo::CameraFeature, false);
    data.featureList.insert(QDeviceInfo::FmRadioFeature, false);
    data.featureList.insert(QDeviceInfo::FmTransmitterFeature, false);
    data.featureList.insert(QDeviceInfo::InfraredFeature, false);
    data.featureList.insert(QDeviceInfo::LedFeature, false);
    data.featureList.insert(QDeviceInfo::MemoryCardFeature, false);
    data.featureList.insert(QDeviceInfo::UsbFeature, false);
    data.featureList.insert(QDeviceInfo::VibrationFeature, false);
    data.featureList.insert(QDeviceInfo::WlanFeature, false);
    data.featureList.insert(QDeviceInfo::SimFeature, false);
    data.featureList.insert(QDeviceInfo::PositioningFeature, false);
    data.featureList.insert(QDeviceInfo::VideoOutFeature, false);
    data.featureList.insert(QDeviceInfo::HapticsFeature, false);
    data.featureList.insert(QDeviceInfo::NfcFeature, false);
}

void QDeviceInfoSimulatorBackend::initImeiList()
{
    data.imeiList.insert(0, QStringLiteral("000000000000000"));
    data.imeiList.insert(0, QStringLiteral("111111111111111"));
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

// QNetworkInfoSimulatorBackend

QNetworkInfoSimulatorBackend::QNetworkInfoSimulatorBackend(QNetworkInfo *parent)
    : QObject(parent)
{
}

QNetworkInfoSimulatorBackend::~QNetworkInfoSimulatorBackend()
{
}

QNetworkInfoSimulatorBackend *QNetworkInfoSimulatorBackend::getSimulatorBackend()
{
    static QMutex mutex;

    mutex.lock();
    if (!globalSimulatorBackend)
        globalSimulatorBackend = new QNetworkInfoSimulatorBackend();
    mutex.unlock();

    return globalSimulatorBackend;
}

QNetworkInfoData::BasicNetworkInfo *QNetworkInfoSimulatorBackend::basicNetworkInfo(QNetworkInfo::NetworkMode mode, int interface)
{
    QNetworkInfoData::BasicNetworkInfo *basic = 0;
    if (interface >= 0) {
        switch (mode) {
        case QNetworkInfo::GsmMode:
        case QNetworkInfo::CdmaMode:
        case QNetworkInfo::WcdmaMode:
        case QNetworkInfo::LteMode:
        case QNetworkInfo::TdscdmaMode:
            if (interface < data.cellularInfo.count())
                basic = &(data.cellularInfo[interface].basicNetworkInfo);
            break;
        case QNetworkInfo::WlanMode:
            if (interface < data.wLanInfo.count()) {
                basic = &(data.wLanInfo[interface].basicNetworkInfo);
            }
            break;
        case QNetworkInfo::EthernetMode:
            if (interface <= data.ethernetInfo.count())
                basic = &(data.ethernetInfo[interface].basicNetworkInfo);
            break;
        case QNetworkInfo::BluetoothMode:
            if (interface <= data.bluetoothInfo.count())
                basic = &(data.bluetoothInfo[interface].basicNetworkInfo);
            break;
        default:
            break;
        }
    }
    return basic;
}

bool QNetworkInfoSimulatorBackend::isValidInterface(QNetworkInfo::NetworkMode mode, int interface)
{
    switch (mode) {
    case QNetworkInfo::GsmMode:
    case QNetworkInfo::CdmaMode:
    case QNetworkInfo::WcdmaMode:
    case QNetworkInfo::LteMode:
    case QNetworkInfo::TdscdmaMode:
        if (interface >= 0 && interface < data.cellularInfo.count())
            return true;
        else
            return false;
    case QNetworkInfo::WlanMode:
        if (interface >= 0 && interface < data.wLanInfo.count())
            return true;
        else
            return false;
    case QNetworkInfo::EthernetMode:
        if (interface >= 0 && interface < data.ethernetInfo.count())
            return true;
        else
            return false;
    case QNetworkInfo::BluetoothMode:
        if (interface >= 0 && interface < data.bluetoothInfo.count())
            return true;
        else
            return false;
    default:
        break;
    }
    return false;
}

QNetworkInfo::NetworkMode QNetworkInfoSimulatorBackend::getCurrentNetworkMode()
{
    if (getNetworkStatus(QNetworkInfo::EthernetMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::EthernetMode;
    else if (getNetworkStatus(QNetworkInfo::WlanMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::WlanMode;
    else if (getNetworkStatus(QNetworkInfo::BluetoothMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::BluetoothMode;
    else if (getNetworkStatus(QNetworkInfo::WimaxMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::WimaxMode;
    else if (getNetworkStatus(QNetworkInfo::LteMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::LteMode;
    else if (getNetworkStatus(QNetworkInfo::WcdmaMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::WcdmaMode;
    else if (getNetworkStatus(QNetworkInfo::CdmaMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::CdmaMode;
    else if (getNetworkStatus(QNetworkInfo::GsmMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::GsmMode;
    else if (getNetworkStatus(QNetworkInfo::TdscdmaMode, 0) == QNetworkInfo::HomeNetwork)
        return QNetworkInfo::TdscdmaMode;
    else if (getNetworkStatus(QNetworkInfo::WimaxMode, 0) == QNetworkInfo::Roaming)
        return QNetworkInfo::WimaxMode;
    else if (getNetworkStatus(QNetworkInfo::LteMode, 0) == QNetworkInfo::Roaming)
        return QNetworkInfo::LteMode;
    else if (getNetworkStatus(QNetworkInfo::WcdmaMode, 0) == QNetworkInfo::Roaming)
        return QNetworkInfo::WcdmaMode;
    else if (getNetworkStatus(QNetworkInfo::CdmaMode, 0) == QNetworkInfo::Roaming)
        return QNetworkInfo::CdmaMode;
    else if (getNetworkStatus(QNetworkInfo::GsmMode, 0) == QNetworkInfo::Roaming)
        return QNetworkInfo::GsmMode;
    else if (getNetworkStatus(QNetworkInfo::TdscdmaMode, 0) == QNetworkInfo::Roaming)
        return QNetworkInfo::TdscdmaMode;
    else
        return QNetworkInfo::UnknownMode;
}

QString QNetworkInfoSimulatorBackend::getNetworkName(QNetworkInfo::NetworkMode mode, int interface)
{
    if (isValidInterface(mode, interface)) {
        QNetworkInfoData::BasicNetworkInfo *basic = basicNetworkInfo(mode, interface);
        return basic->name;
    }
    return QString();
}

int QNetworkInfoSimulatorBackend::getNetworkSignalStrength(QNetworkInfo::NetworkMode mode, int interface)
{
    if (isValidInterface(mode, interface)) {
        QNetworkInfoData::BasicNetworkInfo *basic = basicNetworkInfo(mode, interface);
        return basic->signalStrength;
    }
    return -1;
}

QNetworkInfo::NetworkStatus QNetworkInfoSimulatorBackend::getNetworkStatus(QNetworkInfo::NetworkMode mode, int interface)
{
    if (isValidInterface(mode, interface)) {
        QNetworkInfoData::BasicNetworkInfo *basic = basicNetworkInfo(mode, interface);
        return basic->status;
    }
    return QNetworkInfo::UnknownStatus;
}

QNetworkInfo::NetworkMode QNetworkInfoSimulatorBackend::getMode(QNetworkInfo::NetworkMode mode, int interface)
{
    switch (mode) {
    case QNetworkInfo::GsmMode:
    case QNetworkInfo::CdmaMode:
    case QNetworkInfo::WcdmaMode:
    case QNetworkInfo::LteMode:
    case QNetworkInfo::TdscdmaMode:
        if (isValidInterface(mode, interface)) {
            QNetworkInfoData::BasicNetworkInfo *basic = basicNetworkInfo(mode, interface);
            return basic->mode;
        }
    case QNetworkInfo::WlanMode:
        if (isValidInterface(mode, interface))
            return QNetworkInfo::WlanMode;
    case QNetworkInfo::EthernetMode:
        if (isValidInterface(mode, interface))
            return QNetworkInfo::EthernetMode;
    case QNetworkInfo::BluetoothMode:
        if (isValidInterface(mode, interface))
            return QNetworkInfo::BluetoothMode;
    default:
        break;
    }
    return QNetworkInfo::UnknownMode;
}

QString QNetworkInfoSimulatorBackend::getMacAddress(QNetworkInfo::NetworkMode mode, int interface)
{
    if (interface >= 0) {
        switch (mode) {
        case QNetworkInfo::WlanMode:
            if (isValidInterface(mode, interface))
                return data.wLanInfo[interface].macAddress;
            break;
        case QNetworkInfo::EthernetMode:
            if (isValidInterface(mode, interface))
                return data.ethernetInfo[interface].macAddress;
            break;
        case QNetworkInfo::BluetoothMode:
            if (isValidInterface(mode, interface))
                return data.bluetoothInfo[interface].btAddress;
            break;
        default:
            break;
        }
    }
    return QString();
}

#ifndef QT_NO_NETWORKINTERFACE
QNetworkInterface QNetworkInfoSimulatorBackend::getInterfaceForMode(QNetworkInfo::NetworkMode mode, int interface)
{
    Q_UNUSED(mode)
    Q_UNUSED(interface)
    return QNetworkInterface();
}
#endif // QT_NO_NETWORKINTERFACE

QString QNetworkInfoSimulatorBackend::getImsi(int interface)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface))
        return data.cellularInfo[interface].imsi;
    else
        return QString();
}

QString QNetworkInfoSimulatorBackend::getCellId(int interface)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface))
        return data.cellularInfo[interface].cellId;
    else
        return QString();
}

QString QNetworkInfoSimulatorBackend::getCurrentMobileCountryCode(int interface)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface))
        return data.cellularInfo[interface].currentMobileCountryCode;
    else
        return QString();
}

QString QNetworkInfoSimulatorBackend::getCurrentMobileNetworkCode(int interface)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface))
        return data.cellularInfo[interface].currentMobileNetworkCode;
    else
        return QString();
}

QString QNetworkInfoSimulatorBackend::getHomeMobileCountryCode(int interface)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface))
        return data.cellularInfo[interface].homeMobileCountryCode;
    else
        return QString();
}

QString QNetworkInfoSimulatorBackend::getHomeMobileNetworkCode(int interface)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface))
        return data.cellularInfo[interface].homeMobileNetworkCode;
    else
        return QString();
}

QString QNetworkInfoSimulatorBackend::getLocationAreaCode(int interface)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface))
        return data.cellularInfo[interface].locationAreaCode;
    else
        return QString();
}

QNetworkInfo::CellDataTechnology QNetworkInfoSimulatorBackend::getCurrentCellDataTechnology(int interface)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface))
        return data.cellularInfo[interface].cellData;
    else
        return QNetworkInfo::UnknownDataTechnology;
}

int QNetworkInfoSimulatorBackend::getNetworkInterfaceCount(QNetworkInfo::NetworkMode mode)
{
    switch (mode) {
    case QNetworkInfo::GsmMode:
    case QNetworkInfo::CdmaMode:
    case QNetworkInfo::WcdmaMode:
    case QNetworkInfo::LteMode:
    case QNetworkInfo::TdscdmaMode:
        return data.cellularInfo.count();
    case QNetworkInfo::WlanMode:
        return data.wLanInfo.count();
    case QNetworkInfo::EthernetMode:
        return data.ethernetInfo.count();
    case QNetworkInfo::BluetoothMode:
        return data.bluetoothInfo.count();
    default:
        break;
    }
    return -1;
}

void QNetworkInfoSimulatorBackend::setImsi(int interface, const QString &id)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface) && (data.cellularInfo[interface].imsi != id))
        data.cellularInfo[interface].imsi = id;
}

void QNetworkInfoSimulatorBackend::setCellId(int interface, const QString &id)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface) && (data.cellularInfo[interface].cellId != id)) {
        data.cellularInfo[interface].cellId = id;
        emit cellIdChanged(interface, id);
    }
}

void QNetworkInfoSimulatorBackend::setLocationAreaCode(int interface, const QString &code)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface) && (data.cellularInfo[interface].locationAreaCode != code)) {
        data.cellularInfo[interface].locationAreaCode = code;
        emit locationAreaCodeChanged(interface, code);
    }
}

void QNetworkInfoSimulatorBackend::setCurrentMobileCountryCode(int interface, const QString &code)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface) && (data.cellularInfo[interface].currentMobileCountryCode != code)) {
        data.cellularInfo[interface].currentMobileCountryCode = code;
        emit currentMobileCountryCodeChanged(interface, code);
    }
}

void QNetworkInfoSimulatorBackend::setCurrentMobileNetworkCode(int interface, const QString &code)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface) && (data.cellularInfo[interface].currentMobileNetworkCode != code)) {
        data.cellularInfo[interface].currentMobileNetworkCode = code;
        emit currentMobileNetworkCodeChanged(interface, code);
    }
}

void QNetworkInfoSimulatorBackend::setHomeMobileCountryCode(int interface, const QString &code)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface) && (data.cellularInfo[interface].homeMobileCountryCode != code))
        data.cellularInfo[interface].homeMobileCountryCode = code;
}

void QNetworkInfoSimulatorBackend::setHomeMobileNetworkCode(int interface, const QString &code)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface) && (data.cellularInfo[interface].homeMobileNetworkCode != code))
        data.cellularInfo[interface].homeMobileNetworkCode = code;
}

void QNetworkInfoSimulatorBackend::setCellDataTechnology(int interface, QNetworkInfo::CellDataTechnology cellData)
{
    if (isValidInterface(QNetworkInfo::GsmMode, interface) && (data.cellularInfo[interface].cellData != cellData)) {
        data.cellularInfo[interface].cellData = cellData;
        emit currentCellDataTechnologyChanged(interface, cellData);
    }
}

void QNetworkInfoSimulatorBackend::setMode(int interface, QNetworkInfo::NetworkMode mode)
{
    switch (mode) {
    case QNetworkInfo::GsmMode:
    case QNetworkInfo::CdmaMode:
    case QNetworkInfo::WcdmaMode:
    case QNetworkInfo::LteMode:
    case QNetworkInfo::TdscdmaMode:
        if (isValidInterface(mode, interface) && (data.cellularInfo[interface].basicNetworkInfo.mode != mode))
            data.cellularInfo[interface].basicNetworkInfo.mode = mode;
    default:
        break;
    }
}

void QNetworkInfoSimulatorBackend::setNetworkName(QNetworkInfo::NetworkMode mode, int interface, const QString &name)
{
    if (isValidInterface(mode, interface)) {
        QNetworkInfoData::BasicNetworkInfo *basic = basicNetworkInfo(mode, interface);
        if (basic != 0 && basic->name != name) {
            basic->name = name;
            emit networkNameChanged(mode, interface, name);
        }
    }
}

void QNetworkInfoSimulatorBackend::setNetworkMacAddress(QNetworkInfo::NetworkMode mode, int interface, const QString &mac)
{
    if (interface >= 0) {
        switch (mode) {
        case QNetworkInfo::WlanMode:
            if (isValidInterface(mode, interface))
                data.wLanInfo[interface].macAddress = mac;
            break;
        case QNetworkInfo::EthernetMode:
            if (isValidInterface(mode, interface))
                data.ethernetInfo[interface].macAddress = mac;
            break;
        case QNetworkInfo::BluetoothMode:
            if (isValidInterface(mode, interface))
                data.bluetoothInfo[interface].btAddress = mac;
            break;
        default:
            break;
        }
    }
}

void QNetworkInfoSimulatorBackend::setNetworkSignalStrength(QNetworkInfo::NetworkMode mode, int interface, int strength)
{
    if (isValidInterface(mode, interface)) {
        QNetworkInfoData::BasicNetworkInfo* basic = basicNetworkInfo(mode, interface);
        if (basic != 0 && basic->signalStrength != strength) {
            basic->signalStrength = strength;
            emit networkSignalStrengthChanged(mode, interface, strength);
        }
    }
}

void QNetworkInfoSimulatorBackend::setNetworkStatus(QNetworkInfo::NetworkMode mode, int interface, QNetworkInfo::NetworkStatus status)
{
    if (isValidInterface(mode, interface)) {
        QNetworkInfoData::BasicNetworkInfo* basic = basicNetworkInfo(mode, interface);
        if (basic != 0 && basic->status != status) {
            basic->status = status;
            emit networkStatusChanged(mode, interface, status);
        }
    }
}

void QNetworkInfoSimulatorBackend::addEthernetInterface(QNetworkInfoData::EthernetInfo info)
{
    data.ethernetInfo.append(info);
    emit networkInterfaceCountChanged(info.basicNetworkInfo.mode, getNetworkInterfaceCount(info.basicNetworkInfo.mode));
}

void QNetworkInfoSimulatorBackend::addWlanInterface(QNetworkInfoData::WLanInfo info)
{
    data.wLanInfo.append(info);
    emit networkInterfaceCountChanged(info.basicNetworkInfo.mode, getNetworkInterfaceCount(info.basicNetworkInfo.mode));
}

void QNetworkInfoSimulatorBackend::addCellularInterface(QNetworkInfoData::CellularInfo info)
{
    data.cellularInfo.append(info);
    emit networkInterfaceCountChanged(info.basicNetworkInfo.mode, getNetworkInterfaceCount(info.basicNetworkInfo.mode));
}

void QNetworkInfoSimulatorBackend::addBluetoothInterface(QNetworkInfoData::BluetoothInfo info)
{
    data.bluetoothInfo.append(info);
    emit networkInterfaceCountChanged(info.basicNetworkInfo.mode, getNetworkInterfaceCount(info.basicNetworkInfo.mode));
}

void QNetworkInfoSimulatorBackend::removeInterface(QNetworkInfo::NetworkMode mode, int interface)
{
    clearOrRemoveInterface(mode, interface, false);
}

void QNetworkInfoSimulatorBackend::clearInterface(QNetworkInfo::NetworkMode mode)
{
    clearOrRemoveInterface(mode, 0, true);
}

void QNetworkInfoSimulatorBackend::clearOrRemoveInterface(QNetworkInfo::NetworkMode mode, int interface, bool clear)
{
    switch (mode) {
    case QNetworkInfo::GsmMode:
    case QNetworkInfo::CdmaMode:
    case QNetworkInfo::WcdmaMode:
    case QNetworkInfo::LteMode:
    case QNetworkInfo::TdscdmaMode:
        if (isValidInterface(mode, interface)) {
            clear ? data.cellularInfo.clear() : data.cellularInfo.remove(interface);
            emit networkInterfaceCountChanged(mode, getNetworkInterfaceCount(mode));
        }
        break;
    case QNetworkInfo::WlanMode:
        if (isValidInterface(mode, interface)) {
            clear ? data.wLanInfo.clear() : data.wLanInfo.remove(interface);
            emit networkInterfaceCountChanged(mode, getNetworkInterfaceCount(mode));
        }
        break;
    case QNetworkInfo::EthernetMode:
        if (isValidInterface(mode, interface)) {
            clear ? data.ethernetInfo.clear() : data.ethernetInfo.remove(interface);
            emit networkInterfaceCountChanged(mode, getNetworkInterfaceCount(mode));
        }
        break;
    case QNetworkInfo::BluetoothMode:
        if (isValidInterface(mode, interface)) {
            clear ? data.bluetoothInfo.clear() : data.bluetoothInfo.remove(interface);
            emit networkInterfaceCountChanged(mode, getNetworkInterfaceCount(mode));
        }
        break;
    default:
        break;
    }
}

QT_END_NAMESPACE
