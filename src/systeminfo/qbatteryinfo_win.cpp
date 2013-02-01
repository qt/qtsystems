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

#include "qbatteryinfo_win_p.h"

#include <qt_windows.h>

#include <powrprof.h>
#include <setupapi.h>

#if !defined (Q_CC_MINGW) || defined(__MINGW64_VERSION_MAJOR)
#  include <BatClass.h>
#endif

#include <QtCore/qmetaobject.h>
#include <QtCore/QTimer>
#include <QtCore/QUuid>

#ifdef Q_CC_MSVC
#  pragma comment (lib, "Setupapi.lib")
#endif

QT_BEGIN_NAMESPACE

QBatteryInfoPrivate::QBatteryInfoPrivate(QBatteryInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
    , timeToFull(0)
    , numberOfBatteries(0)
{
    QTimer *timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(getBatteryStatus()));
    timer->start(1000);
    getBatteryStatus();
}

QBatteryInfoPrivate::~QBatteryInfoPrivate()
{
}

int QBatteryInfoPrivate::batteryCount()
{
    return numberOfBatteries;
}

int QBatteryInfoPrivate::currentFlow(int battery)
{
    return currentFlows[battery];
}

int QBatteryInfoPrivate::maximumCapacity(int battery)
{
    return maximumCapacities[battery];
}

int QBatteryInfoPrivate::remainingCapacity(int battery)
{
    return remainingCapacities[battery];
}

int QBatteryInfoPrivate::remainingChargingTime(int battery)
{
    Q_UNUSED(battery)
    SYSTEM_BATTERY_STATE systemBatteryState;
    CallNtPowerInformation(SystemBatteryState,NULL,0,&systemBatteryState,sizeof(systemBatteryState));

   int cTime = systemBatteryState.EstimatedTime;
    if (cTime != timeToFull) {
        timeToFull = cTime;
        emit remainingChargingTimeChanged(1,timeToFull);
    }

    return timeToFull;
}

int QBatteryInfoPrivate::voltage(int battery)
{
    return voltages[battery];
}

QBatteryInfo::ChargerType QBatteryInfoPrivate::chargerType()
{
    return currentChargerType;
}

QBatteryInfo::ChargingState QBatteryInfoPrivate::chargingState(int battery)
{
    return chargingStates[battery];
}

QBatteryInfo::EnergyUnit QBatteryInfoPrivate::energyUnit()
{
    return QBatteryInfo::UnitmWh;
}

QBatteryInfo::BatteryStatus QBatteryInfoPrivate::batteryStatus(int battery)
{
    return batteryStatuses[battery];
}

void QBatteryInfoPrivate::getBatteryStatus()
{
#if !defined (Q_CC_MINGW) || defined(__MINGW64_VERSION_MAJOR)
    SYSTEM_BATTERY_STATE systemBatteryState;
    CallNtPowerInformation(SystemBatteryState,NULL,0,&systemBatteryState,sizeof(systemBatteryState));

    int cTime = systemBatteryState.EstimatedTime;
    if (cTime != timeToFull) {
        timeToFull = cTime;
        emit remainingChargingTimeChanged(1,timeToFull);
    }

    int batteryNumber = 0;

    QUuid guidDeviceBattery(0x72631e54L,0x78A4,0x11d0,0xbc,0xf7,0x00,0xaa,0x00,0xb7,0xb3,0x2a);
    GUID GUID_DEVICE_BATTERY = static_cast<GUID>(guidDeviceBattery);

    HDEVINFO hdevInfo = SetupDiGetClassDevs(&GUID_DEVICE_BATTERY,0,0,DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (INVALID_HANDLE_VALUE != hdevInfo) {
        for (int i = 0; i < 100; i++) {
            SP_DEVICE_INTERFACE_DATA deviceInterfaceData = {0};
            deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

            if (SetupDiEnumDeviceInterfaces(hdevInfo,0,&GUID_DEVICE_BATTERY,i,&deviceInterfaceData)){
                DWORD cbRequired = 0;

                SetupDiGetDeviceInterfaceDetail(hdevInfo, &deviceInterfaceData,0, 0, &cbRequired, 0);
                if (ERROR_INSUFFICIENT_BUFFER == GetLastError()) {
                    PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetail =
                            (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, cbRequired);
                    if (deviceInterfaceDetail){
                        deviceInterfaceDetail->cbSize = sizeof(*deviceInterfaceDetail);
                        if (SetupDiGetDeviceInterfaceDetail(hdevInfo, &deviceInterfaceData, deviceInterfaceDetail, cbRequired, &cbRequired, 0)) {

                            batteryNumber++; //new battery
                            HANDLE batteryHandle = CreateFile(deviceInterfaceDetail->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                         NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                            if (INVALID_HANDLE_VALUE != batteryHandle){

                                BATTERY_QUERY_INFORMATION batteryQueryInfo = {0};

                                DWORD inBuf = 0;
                                DWORD dwOut;

                                if (DeviceIoControl(batteryHandle,IOCTL_BATTERY_QUERY_TAG, &inBuf, sizeof(inBuf), &batteryQueryInfo.BatteryTag, sizeof(batteryQueryInfo.BatteryTag), &dwOut, NULL)
                                        && batteryQueryInfo.BatteryTag) {

                                    BATTERY_INFORMATION batteryInfo = {0};
                                    batteryQueryInfo.InformationLevel = BatteryInformation;

                                    if (DeviceIoControl(batteryHandle, IOCTL_BATTERY_QUERY_INFORMATION,
                                                        &batteryQueryInfo, sizeof(batteryQueryInfo),
                                                        &batteryInfo, sizeof(batteryInfo), &dwOut, NULL)) {

                                        maximumCapacities.insert(batteryNumber, batteryInfo.FullChargedCapacity);

                                        if (batteryInfo.Capabilities & BATTERY_SYSTEM_BATTERY) {
                                            if (!(batteryInfo.Capabilities & BATTERY_IS_SHORT_TERM)) {
 //                                                dwResult |= HASBATTERY;

                                                BATTERY_WAIT_STATUS batteryWaitStatus = {0};
                                                batteryWaitStatus.BatteryTag = batteryQueryInfo.BatteryTag;

                                                BATTERY_STATUS batteryStatus;
                                                if (DeviceIoControl(batteryHandle, IOCTL_BATTERY_QUERY_STATUS,
                                                                    &batteryWaitStatus, sizeof(batteryWaitStatus),
                                                                    &batteryStatus, sizeof(batteryStatus), &dwOut, NULL)) {

                                                    QBatteryInfo::ChargerType chargerType = QBatteryInfo::UnknownCharger;

                                                    if (batteryStatus.PowerState & BATTERY_POWER_ON_LINE) {
                                                        chargerType = QBatteryInfo::WallCharger;
                                                    }
                                                    if (currentChargerType != chargerType) {
                                                        currentChargerType = chargerType;
                                                        Q_EMIT chargerTypeChanged(chargerType);
                                                    }

                                                    QBatteryInfo::ChargingState chargingState;
                                                    if (batteryStatus.PowerState & BATTERY_CHARGING)
                                                        chargingState = QBatteryInfo::Charging;
                                                    if (batteryStatus.PowerState & BATTERY_DISCHARGING)
                                                         chargingState = QBatteryInfo::Discharging;

                                                    if (chargingStates[batteryNumber] != chargingState) {
                                                        chargingStates.insert(batteryNumber, chargingState);
                                                        emit chargingStateChanged(batteryNumber,chargingState);
                                                    }

                                                    if (voltages[batteryNumber] !=  batteryStatus.Voltage) {
                                                        voltages.insert(batteryNumber, batteryStatus.Voltage);
                                                        Q_EMIT voltageChanged(batteryNumber, batteryStatus.Voltage);
                                                    }
                                                    if (currentFlows[batteryNumber] != batteryStatus.Rate) {
                                                        currentFlows.insert(batteryNumber,batteryStatus.Rate);
                                                        Q_EMIT currentFlowChanged(batteryNumber, batteryStatus.Rate);
                                                    }
                                                    if (remainingCapacities[batteryNumber] != batteryStatus.Capacity) {
                                                        remainingCapacities.insert(batteryNumber, batteryStatus.Capacity);
                                                        Q_EMIT remainingCapacityChanged(batteryNumber, batteryStatus.Capacity);
                                                    }
                                                    ///
                                                    int level = batteryInfo.FullChargedCapacity / batteryStatus.Capacity;
                                                    QBatteryInfo::BatteryStatus batStatus = QBatteryInfo::BatteryStatusUnknown;

                                                    if (batteryStatus.PowerState & BATTERY_CRITICAL) {
                                                        batStatus =QBatteryInfo::BatteryEmpty ;
                                                    } else if (level < 67) {
                                                        batStatus = QBatteryInfo::BatteryLow;
                                                    } else if (level > 66) {
                                                       batStatus = QBatteryInfo::BatteryOk;
                                                    } else if (level == 100) {
                                                        batStatus = QBatteryInfo::BatteryFull;
                                                    }

                                                    if (batteryStatuses[batteryNumber] != batStatus) {
                                                        batteryStatuses.insert(batteryNumber,batStatus);
                                                        Q_EMIT batteryStatusChanged(batteryNumber, batStatus);
                                                    }
                                                }
                                            }
                                        }
                                    } //end IOCTL_BATTERY_QUERY_INFORMATION
                                } // end BATTERY_INFORMATION

                                CloseHandle(batteryHandle);
                            }
                        }
                        LocalFree(deviceInterfaceDetail);
                    }
                }
            }
            else  if (ERROR_NO_MORE_ITEMS == GetLastError()) {
                break;
            }
        }
        SetupDiDestroyDeviceInfoList(hdevInfo);
    }
    numberOfBatteries = batteryNumber;
#else // !defined (Q_CC_MINGW) || defined(__MINGW64_VERSION_MAJOR)
    numberOfBatteries = 0;
    Q_UNIMPLEMENTED();
#endif
}

QT_END_NAMESPACE
