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

#ifndef QSYSTEMINFO_SIMULATOR_P_H
#define QSYSTEMINFO_SIMULATOR_P_H


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

#include <QStringList>

#include <QNetworkInfo>
#include <QDeviceInfo>
#include <QStorageInfo>
#include <QBatteryInfo>
#include <QDeviceInfo>
#include <QDisplayInfo>
#include <QStorageInfo>
#include <QScreenSaver>
#include <QInputDeviceInfo>
#include "qsysteminfodata_simulator_p.h"

QT_BEGIN_NAMESPACE

class QNetworkInfoPrivate
{
public:
    QNetworkInfoPrivate(QNetworkInfo *) {}

    int networkInterfaceCount(QNetworkInfo::NetworkMode) { return -1; }
    int networkSignalStrength(QNetworkInfo::NetworkMode, int) { return -1; }
    QNetworkInfo::CellDataTechnology currentCellDataTechnology(int) { return QNetworkInfo::UnknownDataTechnology; }
    QNetworkInfo::NetworkMode currentNetworkMode() { return QNetworkInfo::UnknownMode; }
    QNetworkInfo::NetworkStatus networkStatus(QNetworkInfo::NetworkMode, int) { return QNetworkInfo::UnknownStatus; }
    QNetworkInterface interfaceForMode(QNetworkInfo::NetworkMode, int) { return QNetworkInterface(); }
    QString cellId(int) { return QString(); }
    QString currentMobileCountryCode(int) { return QString(); }
    QString currentMobileNetworkCode(int) { return QString(); }
    QString homeMobileCountryCode(int) { return QString(); }
    QString homeMobileNetworkCode(int) { return QString(); }
    QString imsi(int) { return QString(); }
    QString locationAreaCode(int) { return QString(); }
    QString macAddress(QNetworkInfo::NetworkMode, int) { return QString(); }
    QString networkName(QNetworkInfo::NetworkMode, int) { return QString(); }
};
QNetworkInfoPrivate *getNetworkInfoPrivate();

class QDisplayInfoPrivate
{
public:
    QDisplayInfoPrivate(QDisplayInfo *) {}

    int brightness(int) { return -1; }
    int contrast(int) const { return -1; }
    QDisplayInfo::BacklightState backlightState(int) { return QDisplayInfo::BacklightUnknown; }
};
QDisplayInfoPrivate *getDisplayInfoPrivate();

class QDeviceInfoPrivate
{
public:
    QDeviceInfoPrivate(QDeviceInfo *) {}

    bool hasFeature(QDeviceInfo::Feature) { return false; }
    int imeiCount() { return -1; }
    QDeviceInfo::LockTypeFlags activatedLocks() { return QDeviceInfo::NoLock; }
    QDeviceInfo::LockTypeFlags enabledLocks() { return QDeviceInfo::NoLock; }
    QDeviceInfo::ThermalState thermalState() { return QDeviceInfo::UnknownThermal; }
    QString imei(int) { return QString(); }
    QString manufacturer() { return QString(); }
    QString model() { return QString(); }
    QString productName() { return QString(); }
    QString uniqueDeviceID() { return QString(); }
    QString version(QDeviceInfo::Version) { return QString(); }
};
QDeviceInfoPrivate *getDeviceInfoPrivate();

class QStorageInfoPrivate
{
public:
    QStorageInfoPrivate(QStorageInfo *) {}

    qlonglong availableDiskSpace(const QString &) { return -1; }
    qlonglong totalDiskSpace(const QString &) { return -1; }
    QString uriForDrive(const QString &) { return QString(); }
    QStringList allLogicalDrives() { return QStringList(); }
    QStorageInfo::DriveType driveType(const QString &) { return QStorageInfo::UnknownDrive; }
};
QStorageInfoPrivate *getStorageInfoPrivate();

class QScreenSaverPrivate
{
public:
    QScreenSaverPrivate(QScreenSaver *) {}

    bool screenSaverEnabled() { return false; }
    void setScreenSaverEnabled(bool) {}
};
QScreenSaverPrivate *getQScreenSaverPrivate();

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
};
QBatteryInfoPrivate *getQBatteryInfoPrivate();

class QInputDeviceInfoPrivate
{
public:
    QInputDeviceInfoPrivate(QInputDeviceInfo *) {}

    bool isKeyboardFlippedOpen() { return false; }
    bool isKeyboardLightOn() { return false; }
    bool isWirelessKeyboardConnected() { return false; }
    QInputDeviceInfo::InputDeviceTypes availableInputDevices() { return QInputDeviceInfo::UnknownInputDevice; }
    QInputDeviceInfo::KeyboardTypes availableKeyboards() { return QInputDeviceInfo::UnknownKeyboard; }
    QInputDeviceInfo::TouchDeviceTypes availableTouchDevices() { return QInputDeviceInfo::UnknownTouchDevice; }
};
QInputDeviceInfoPrivate *getInputDeviceInfoPrivate();

class QDeviceProfilePrivate
{
public:
    QDeviceProfilePrivate(QDeviceProfile *) {}

    bool isVibrationActivated() { return false; }
    int messageRingtoneVolume() { return -1; }
    int voiceRingtoneVolume() { return -1; }
    QDeviceProfile::ProfileType profileType() { return QDeviceProfile::UnknownProfile; }
};
QDeviceProfilePrivate *getDeviceProfilePrivate();

QT_END_NAMESPACE

#endif /*QSYSTEMINFO_SIMULATOR_P_H*/
