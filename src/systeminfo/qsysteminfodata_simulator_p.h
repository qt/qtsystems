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

#ifndef QSYSTEMINFODATA_SIMULATOR_P_H
#define QSYSTEMINFODATA_SIMULATOR_P_H


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

#include <QNetworkInfo>
#include <QDeviceInfo>
#include <QBatteryInfo>
#include <QDisplayInfo>
#include <QStorageInfo>
#include <QScreenSaver>
#include <QInputDeviceInfo>
#include <QDeviceProfile>

#include <QHash>
#include <QVector>
#include <QString>
#include <QMetaType>

QT_BEGIN_NAMESPACE

void qt_registerSystemInfoTypes();

struct QNetworkInfoData
{
    struct BasicNetworkInfo {
        QString name;
        int signalStrength;
        QNetworkInfo::NetworkMode mode;
        QNetworkInfo::NetworkStatus status;
    };

    struct EthernetInfo
    {
        BasicNetworkInfo basicNetworkInfo;
        QString macAddress;
        QNetworkInterface interface;
    };

    struct WLanInfo
    {
        BasicNetworkInfo basicNetworkInfo;
        QString macAddress;
        QNetworkInterface interface;
    };

    struct CellularInfo {
        BasicNetworkInfo basicNetworkInfo;

        QString cellId;
        QString locationAreaCode;

        QString currentMobileCountryCode;
        QString currentMobileNetworkCode;
        QString homeMobileCountryCode;
        QString homeMobileNetworkCode;
        QNetworkInfo::CellDataTechnology cellData;
    };

    struct BluetoothInfo {
        BasicNetworkInfo basicNetworkInfo;
        QString btAddress;
    };

    QVector<EthernetInfo> ethernetInfo;
    QVector<WLanInfo> wLanInfo;
    QVector<CellularInfo> cellularInfo;
    QVector<BluetoothInfo> bluetoothInfo;
};

struct QBatteryInfoData
{
    int nominalCapacity;
    int remainingCapacityPercent;
    int remainingCapacity;

    int voltage;
    int remainingChargingTime;

    int currentFlow;
    int cumulativeCurrentFlow;
    int remainingCapacityBars;
    int maxBars;

    QBatteryInfo::ChargingState chargingState;
    QBatteryInfo::ChargerType chargerType;
    QBatteryInfo::EnergyUnit energyMeasurementUnit;
};

struct QDeviceInfoData
{
    QString imei;
    QString manufacturer;
    QString model;
    QString productName;
    QString uniqueDeviceID;
    int     imeiCount;

    QDeviceInfo::Feature feature;
    QDeviceInfo::LockTypeFlags lockType;
    QDeviceInfo::ThermalState currentThermalState;
    QDeviceInfo::Version version;
};

struct QDisplayInfoData
{
    int brightness;
    int contrast;
    QDisplayInfo::BacklightState backlightStatus;
};

struct QStorageInfoData
{
    struct DriveInfo
    {
      qint64 totalSpace;
      qint64 availableSpace;
      QString uri;
      QStorageInfo::DriveType type;
    };
    QHash<QString, DriveInfo> drives;
};

struct QScreenSaverData
{
    bool screenSaverEnabled;
};

struct QInputDeviceInfoData
{
    QInputDeviceInfo::InputDeviceTypes availableInputDevices;
    QInputDeviceInfo::KeyboardTypes availableKeyboards;
    QInputDeviceInfo::TouchDeviceTypes availableTouchDevices;
    bool isKeyboardFlippedOpen;
    bool isKeyboardLightOn;
    bool isWirelessKeyboardConnected;
};

struct QDeviceProfileData
{
    int messageRingtoneVolume;
    int voiceRingtoneVolume;
    QDeviceProfile::ProfileType profileType;
    bool isVibrationActivated;
};

Q_DECLARE_METATYPE(QNetworkInfoData)
Q_DECLARE_METATYPE(QDeviceInfoData)
Q_DECLARE_METATYPE(QBatteryInfoData)
Q_DECLARE_METATYPE(QDisplayInfoData)
Q_DECLARE_METATYPE(QStorageInfoData)
Q_DECLARE_METATYPE(QScreenSaverData)
Q_DECLARE_METATYPE(QDeviceProfileData)
Q_DECLARE_METATYPE(QInputDeviceInfoData)

QT_END_NAMESPACE

#endif // QSYSTEMINFODATA_SIMULATOR_P_H
