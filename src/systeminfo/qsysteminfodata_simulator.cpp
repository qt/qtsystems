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

#include "qsysteminfodata_simulator_p.h"
#include <QtCore/QDataStream>

QT_BEGIN_NAMESPACE

void qt_registerSystemInfoTypes()
{
    qRegisterMetaTypeStreamOperators<QNetworkInfoData>("QNetworkInfoData");
    qRegisterMetaTypeStreamOperators<QNetworkInfoData::EthernetInfo>("QNetworkInfoData::EthernetInfo");
    qRegisterMetaTypeStreamOperators<QNetworkInfoData::WLanInfo>("QNetworkInfoData::WLanInfo");
    qRegisterMetaTypeStreamOperators<QNetworkInfoData::CellularInfo>("QNetworkInfoData::CellularInfo");
    qRegisterMetaTypeStreamOperators<QNetworkInfoData::BluetoothInfo>("QNetworkInfoData::BluetoothInfo");
    qRegisterMetaTypeStreamOperators<QNetworkInfoData::BasicNetworkInfo>("QNetworkInfoData::BasicNetworkInfo");
    qRegisterMetaTypeStreamOperators<QDeviceInfoData>("QDeviceInfoData");
    qRegisterMetaTypeStreamOperators<QBatteryInfoData>("QBatteryInfoData");
    qRegisterMetaTypeStreamOperators<QDisplayInfoData>("QDisplayInfoData");
    qRegisterMetaTypeStreamOperators<QStorageInfoData>("QStorageInfoData");
    qRegisterMetaTypeStreamOperators<QStorageInfoData::DriveInfo>("QStorageInfoData::DriveInfo");
    qRegisterMetaTypeStreamOperators<QScreenSaverData>("QScreenSaverData");
    qRegisterMetaTypeStreamOperators<QInputDeviceInfoData>("QInputDeviceInfoData");
    qRegisterMetaTypeStreamOperators<QDeviceProfileData>("QDeviceProfileData");
}

QDataStream &operator<<(QDataStream &out, const QNetworkInfoData::BasicNetworkInfo &s)
{
    out << s.name;
    out << static_cast<qint32>(s.signalStrength);
    out << static_cast<qint32>(s.mode);
    out << static_cast<qint32>(s.status);
    return out;
}

QDataStream &operator>>(QDataStream &in, QNetworkInfoData::BasicNetworkInfo &s)
{
    in >> s.name;
    qint32 signalStrength;
    in >> signalStrength;
    s.signalStrength = signalStrength;
    qint32 mode, status;
    in >> status;
    in >> mode;
    s.mode = static_cast<QNetworkInfo::NetworkMode>(mode);
    s.status = static_cast<QNetworkInfo::NetworkStatus>(status);
    return in;
}

QDataStream &operator<<(QDataStream &out, const QNetworkInfoData::EthernetInfo &s)
{
    out << s.basicNetworkInfo;
    out << s.macAddress;

    return out;
}

QDataStream &operator>>(QDataStream &in, QNetworkInfoData::EthernetInfo &s)
{
    in >> s.basicNetworkInfo;
    in >> s.macAddress;

    return in;
}

QDataStream &operator<<(QDataStream &out, const QNetworkInfoData::WLanInfo &s)
{
    out << s.basicNetworkInfo;
    out << s.macAddress;
    return out;
}

QDataStream &operator>>(QDataStream &in, QNetworkInfoData::WLanInfo &s)
{
    in >> s.basicNetworkInfo;
    in >> s.macAddress;
    return in;
}

QDataStream &operator<<(QDataStream &out, const QNetworkInfoData::BluetoothInfo &s)
{
    out << s.basicNetworkInfo;
    out << s.btAddress;
    return out;
}

QDataStream &operator>>(QDataStream &in, QNetworkInfoData::BluetoothInfo &s)
{
    in >> s.basicNetworkInfo;
    in >> s.btAddress;
    return in;
}

QDataStream &operator<<(QDataStream &out, const QNetworkInfoData::CellularInfo &s)
{
    out << s.basicNetworkInfo;
    out << static_cast<qint32>(s.cellId) << static_cast<qint32>(s.locationAreaCode);
    out << s.currentMobileCountryCode << s.currentMobileNetworkCode;
    out << s.homeMobileCountryCode << s.homeMobileNetworkCode;
    out << static_cast<qint32>(s.cellData);

    return out;
}

QDataStream &operator>>(QDataStream &in, QNetworkInfoData::CellularInfo &s)
{
    in >> s.basicNetworkInfo;
    qint32 cellid, lac, cellData;
    in >> cellid >> lac;
    s.cellId = cellid;
    s.locationAreaCode = lac;
    in >> s.currentMobileCountryCode >> s.currentMobileNetworkCode;
    in >> s.homeMobileCountryCode >> s.homeMobileNetworkCode;
    in >> cellData;
    s.cellData = static_cast<QNetworkInfo::CellDataTechnology>(cellData);

    return in;
}

QDataStream &operator<<(QDataStream &out, const QNetworkInfoData &s)
{
    out << s.ethernetInfo;
    out << s.wLanInfo;
    out << s.cellularInfo;
    out << s.bluetoothInfo;

    return out;
}

QDataStream &operator>>(QDataStream &in, QNetworkInfoData &s)
{
    in >> s.ethernetInfo;
    in >> s.wLanInfo;
    in >> s.cellularInfo;
    in >> s.bluetoothInfo;

    return in;
}

QDataStream &operator<<(QDataStream &out, const QDeviceInfoData &s)
{
    out << s.imei << s.manufacturer << s.model << s.productName << s.uniqueDeviceID;
    out << s.imeiCount;
    out << static_cast<qint32>(s.feature) << static_cast<qint32>(s.lockType)
        << static_cast<qint32>(s.currentThermalState) << static_cast<qint32>(s.version);

    return out;
}

QDataStream &operator>>(QDataStream &in, QDeviceInfoData &s)
{
    in >> s.imei >> s.manufacturer >> s.model >> s.productName >> s.uniqueDeviceID;
    in >> s.imeiCount;
    qint32 feature, lockType, currentThermalState, version;
    in >> feature >> lockType >> currentThermalState >> version;
    s.feature = static_cast<QDeviceInfo::Feature>(feature);
    s.lockType = static_cast<QDeviceInfo::LockTypeFlags>(lockType);
    s.currentThermalState = static_cast<QDeviceInfo::ThermalState>(currentThermalState);
    s.version = static_cast<QDeviceInfo::Version>(version);

    return in;
}

QDataStream &operator<<(QDataStream &out, const QBatteryInfoData &s)
{
    out << static_cast<qint32>(s.chargingState) << static_cast<qint32>(s.chargerType)
        << static_cast<qint32>(s.energyMeasurementUnit);

    out << static_cast<qint32>(s.nominalCapacity) << static_cast<qint32>(s.remainingCapacityPercent)
        << static_cast<qint32>(s.remainingCapacity) << static_cast<qint32>(s.voltage)
        << static_cast<qint32>(s.remainingChargingTime) << static_cast<qint32>(s.currentFlow)
        << static_cast<qint32>(s.cumulativeCurrentFlow) << static_cast<qint32>(s.remainingCapacityBars)
        << static_cast<qint32>(s.maxBars);
    return out;
}

QDataStream &operator>>(QDataStream &in, QBatteryInfoData &s)
{
    qint32 chargingState, chargerType, energyMeasurementUnit;
    in >> chargingState >> chargerType >> energyMeasurementUnit;
    s.chargingState = static_cast<QBatteryInfo::ChargingState>(chargingState);
    s.chargerType = static_cast<QBatteryInfo::ChargerType>(chargerType);
    s.energyMeasurementUnit = static_cast<QBatteryInfo::EnergyUnit>(energyMeasurementUnit);

    qint32 nominalCapacity, remainingCapacityPercent, remainingCapacity, voltage,
           remainingChargingTime, currentFlow, cumulativeCurrentFlow, remainingCapacityBars,
           maxBars;
    in >> nominalCapacity >> remainingCapacityPercent >> remainingCapacity >> voltage
       >> remainingChargingTime >> currentFlow >> cumulativeCurrentFlow >> remainingCapacityBars
       >> maxBars;

    s.nominalCapacity = nominalCapacity;
    s.remainingCapacityPercent = remainingCapacityPercent;
    s.remainingCapacity = remainingCapacity;
    s.voltage = voltage;
    s.remainingChargingTime = remainingChargingTime;
    s.currentFlow = currentFlow;
    s.cumulativeCurrentFlow = cumulativeCurrentFlow;
    s.remainingCapacityBars = remainingCapacityBars;
    s.maxBars = maxBars;

    return in;
}

QDataStream &operator<<(QDataStream &out, const QDisplayInfoData &s)
{
    out << static_cast<qint32>(s.brightness) << static_cast<qint32>(s.contrast);
    out << static_cast<qint32>(s.backlightStatus);

    return out;
}

QDataStream &operator>>(QDataStream &in, QDisplayInfoData &s)
{
    qint32 brightness, contrast;
    in >> brightness >> contrast;

    s.brightness = brightness;
    s.contrast = contrast;
    qint32 backlightStatus;
    in >> backlightStatus;
    s.backlightStatus = static_cast<QDisplayInfo::BacklightState>(backlightStatus);

    return in;
}

QDataStream &operator<<(QDataStream &out, const QStorageInfoData &s)
{
    out << s.drives;
    return out;
}

QDataStream &operator>>(QDataStream &in, QStorageInfoData &s)
{
    in >> s.drives;
    return in;
}

QDataStream &operator<<(QDataStream &out, const QStorageInfoData::DriveInfo &s)
{
    out << s.totalSpace << s.availableSpace;
    out << s.uri;
    out << static_cast<qint32>(s.type);

    return out;
}

QDataStream &operator>>(QDataStream &in, QStorageInfoData::DriveInfo &s)
{
    in >> s.totalSpace >> s.availableSpace;
    in >> s.uri;
    qint32 type;
    in >> type;
    s.type = static_cast<QStorageInfo::DriveType>(type);

    return in;
}

QDataStream &operator<<(QDataStream &out, const QScreenSaverData &s)
{
    out << s.screenSaverEnabled;
    return out;
}

QDataStream &operator>>(QDataStream &in, QScreenSaverData &s)
{
    in >> s.screenSaverEnabled;
    return in;
}

QDataStream &operator<<(QDataStream &out, const QInputDeviceInfoData &s)
{
    out << static_cast<qint32>(s.availableInputDevices)
        << static_cast<qint32>(s.availableKeyboards)
        << static_cast<qint32>(s.availableTouchDevices);
    out << s.isKeyboardFlippedOpen << s.isKeyboardLightOn << s.isWirelessKeyboardConnected;

    return out;
}

QDataStream &operator>>(QDataStream &in, QInputDeviceInfoData &s)
{
    qint32 availableInputDevices, availableKeyboards, availableTouchDevices;
    in >> availableInputDevices >> availableKeyboards >> availableTouchDevices;
    s.availableInputDevices = static_cast<QInputDeviceInfo::InputDeviceTypes>(availableInputDevices);
    s.availableKeyboards = static_cast<QInputDeviceInfo::KeyboardTypes>(availableKeyboards);
    s.availableTouchDevices = static_cast<QInputDeviceInfo::TouchDeviceTypes>(availableTouchDevices);

    in >> s.isKeyboardFlippedOpen >> s.isKeyboardLightOn >> s.isWirelessKeyboardConnected;

    return in;
}

QDataStream &operator<<(QDataStream &out, const QDeviceProfileData &s)
{
    out << static_cast<qint32>(s.messageRingtoneVolume)
        << static_cast<qint32>(s.voiceRingtoneVolume)
        << static_cast<qint32>(s.profileType);
    out << s.isVibrationActivated;

    return out;
}

QDataStream &operator>>(QDataStream &in, QDeviceProfileData &s)
{
    qint32 messageRingtoneVolume, voiceRingtoneVolume, profileType;
    in >> messageRingtoneVolume >> voiceRingtoneVolume >> profileType;
    s.messageRingtoneVolume = messageRingtoneVolume;
    s.voiceRingtoneVolume = voiceRingtoneVolume;
    s.profileType = static_cast<QDeviceProfile::ProfileType>(profileType);
    in >> s.isVibrationActivated;

    return in;
}

QT_END_NAMESPACE


