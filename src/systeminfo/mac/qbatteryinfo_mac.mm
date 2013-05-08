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

#include "qbatteryinfo_mac_p.h"

#include <QtCore/qmetaobject.h>
#include <QDebug>

#include <Foundation/NSAutoreleasePool.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSString.h>
#include <Foundation/NSKeyValueCoding.h>
#include <IOKit/IOKitLib.h>

QT_BEGIN_NAMESPACE

void powerInfoChanged(void* context)
{
    QBatteryInfoPrivate *sys = reinterpret_cast<QBatteryInfoPrivate *>(context);
    if (sys) {
        sys->currentChargingState();
    }
}

void batteryInfoChanged(void* context)
{
    QBatteryInfoPrivate *bat = reinterpret_cast<QBatteryInfoPrivate *>(context);
    if (bat) {
        bat->getBatteryInfo();
    }
}

QBatteryInfoPrivate::QBatteryInfoPrivate(QBatteryInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
    , currentBatLevel(0)
    , currentVoltage(-1)
    , dischargeRate(0)
    , capacity(-1)
    , timeToFull(-1)
    , remainingEnergy(-1)
    , numberOfBatteries(0)
{
    getBatteryInfo();
    NSAutoreleasePool *autoreleasepool = [[NSAutoreleasePool alloc] init];

    CFRunLoopSourceRef runLoopSource = (CFRunLoopSourceRef)IOPSNotificationCreateRunLoopSource(batteryInfoChanged, this);
    if (runLoopSource) {
        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);
        CFRelease(runLoopSource);
    }
    [autoreleasepool release];
    currentChargingState();
    getBatteryInfo();
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
    if (battery < 0)
        battery = 0;
    return currentFlows.value(battery);
}

int QBatteryInfoPrivate::maximumCapacity(int battery)
{
    if (battery < 0)
        battery = 0;
    return maximumCapacities.value(battery);
}

int QBatteryInfoPrivate::remainingCapacity(int battery)
{
    if (battery < 0)
        battery = 0;
    return remainingCapacities.value(battery);
}

int QBatteryInfoPrivate::remainingChargingTime(int battery)
{
    if (battery < 0)
        battery = 0;
    return remainingChargingTimes.value(battery);
}

int QBatteryInfoPrivate::voltage(int battery)
{
    if (battery < 0)
        battery = 0;
    return voltages.value(battery);
}

QBatteryInfo::ChargerType QBatteryInfoPrivate::chargerType()
{
    return curChargeType;
}

QBatteryInfo::ChargingState QBatteryInfoPrivate::chargingState(int battery)
{
    if (battery < 0)
        battery = 0;
    return chargingStates.value(battery);
}

QBatteryInfo::EnergyUnit QBatteryInfoPrivate::energyUnit()
{
    return QBatteryInfo::UnitmAh;
}

QBatteryInfo::BatteryStatus QBatteryInfoPrivate::batteryStatus(int battery)
{
    if (battery < 0)
        battery = 0;
    return batteryStatuses.value(battery);
}

void QBatteryInfoPrivate::connectNotify(const QMetaMethod &signal)
{
   // static const QMetaMethod batteryCountChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::batteryCountChanged);
    static const QMetaMethod chargingStateChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::chargingStateChanged);
    static const QMetaMethod currentFlowChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::currentFlowChanged);
    static const QMetaMethod batteryStatusChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::batteryStatusChanged);
//    static const QMetaMethod chargerTypeChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::chargerTypeChanged);
//    static const QMetaMethod remainingCapacityChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::remainingCapacityChanged);
//    static const QMetaMethod remainingChargingTimeChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::remainingChargingTimeChanged);
//    static const QMetaMethod voltageChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::voltageChanged);

    if (signal == chargingStateChangedSignal
               || signal == currentFlowChangedSignal
               || signal == batteryStatusChangedSignal) {

           NSAutoreleasePool *autoreleasepool = [[NSAutoreleasePool alloc] init];

           CFRunLoopSourceRef runLoopSource = (CFRunLoopSourceRef)IOPSNotificationCreateRunLoopSource(powerInfoChanged, this);
           if (runLoopSource) {
               CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);
           }
           [autoreleasepool release];
           CFRelease(runLoopSource);
       }
}

void QBatteryInfoPrivate::disconnectNotify(const QMetaMethod &signal)
{
    Q_UNUSED(signal)
}

QBatteryInfo::ChargingState QBatteryInfoPrivate::currentChargingState()
{
    NSAutoreleasePool *autoreleasepool = [[NSAutoreleasePool alloc] init];
    QBatteryInfo::ChargingState state = QBatteryInfo::UnknownChargingState;
    QBatteryInfo::ChargerType cType = QBatteryInfo::UnknownCharger;

    CFDictionaryRef powerSourceDict = NULL;
    CFStringRef powerStateString;

    CFTypeRef powerSourcesInfo = IOPSCopyPowerSourcesInfo();
    CFArrayRef powerSourcesList = IOPSCopyPowerSourcesList(powerSourcesInfo);
    int batteries = CFArrayGetCount(powerSourcesList);

    if (batteries != numberOfBatteries) {
        numberOfBatteries = batteries;
        Q_EMIT batteryCountChanged(numberOfBatteries);
    }

    int i;
    for (i = 0; i < numberOfBatteries; i++) {
        powerSourceDict = IOPSGetPowerSourceDescription(powerSourcesInfo, CFArrayGetValueAtIndex(powerSourcesList, i));
        if (!powerSourceDict) {
            state = QBatteryInfo::UnknownChargingState;
            cType = QBatteryInfo::UnknownCharger;
            qDebug() << "unknown state";
            break;
        }

        powerStateString = (CFStringRef)CFDictionaryGetValue(powerSourceDict, CFSTR(kIOPSPowerSourceStateKey));
        if (CFStringCompare(powerStateString,CFSTR(kIOPSBatteryPowerValue),0)==kCFCompareEqualTo) {
            //has battery
            state = QBatteryInfo::Discharging;
            cType = QBatteryInfo::UnknownCharger;
        } else {

            NSDictionary *powerSourceInfo = nil;
            powerSourceInfo = [NSDictionary dictionaryWithDictionary:(NSDictionary*)powerSourceDict];
            bool charging = (bool)[[powerSourceInfo valueForKey:[NSString stringWithUTF8String:kIOPSIsChargingKey]] boolValue];
            if (charging ) {
                state = QBatteryInfo::Charging;
                cType = QBatteryInfo::WallCharger;
            } else {
                cType = QBatteryInfo::UnknownCharger;
                state = QBatteryInfo::NotCharging;
            }
        }

        if (cType != curChargeType) {
            curChargeType = cType;
            Q_EMIT chargerTypeChanged(curChargeType);
        }

        if ( chargingStates.value(i) != state) {
            chargingStates[i] = state;
            Q_EMIT chargingStateChanged(i,state);
        }
    }
    CFRelease(powerSourcesInfo);
    CFRelease(powerSourcesList);

    [autoreleasepool release];
    return state;
}

void QBatteryInfoPrivate::getBatteryInfo()
{
    int cEnergy = 0;
    int cVoltage = 0;
    int cTime = 0;
    int rEnergy = 0;

    CFTypeRef info;
    CFArrayRef list;
    CFDictionaryRef battery;

    info = IOPSCopyPowerSourcesInfo();
    if (info == NULL) {
        qDebug() << "IOPSCopyPowerSourcesInfo error";
        return;
    }
    list = IOPSCopyPowerSourcesList(info);
    if (list == NULL) {
        CFRelease(info);
        qDebug() << "IOPSCopyPowerSourcesList error";
        return;
    }

    CFMutableDictionaryRef matching = NULL;
    CFMutableDictionaryRef batDoctionary = NULL;
    io_registry_entry_t entry = 0;
    matching = IOServiceMatching("IOPMPowerSource");
    entry = IOServiceGetMatchingService(kIOMasterPortDefault,matching);
    IORegistryEntryCreateCFProperties(entry, &batDoctionary,NULL,0);

    int batteries = CFArrayGetCount(list);
    for (int i = 0; i < batteries; i++) {

        battery = IOPSGetPowerSourceDescription(info, CFArrayGetValueAtIndex(list, i));

        int curCapacityPercent = 0;
        int maxCapacity = 0;

        const void *psValue;
        psValue = CFDictionaryGetValue(battery, CFSTR(kIOPSCurrentCapacityKey));
        CFNumberGetValue((CFNumberRef)psValue, kCFNumberSInt32Type, &curCapacityPercent);

        psValue = CFDictionaryGetValue(battery, CFSTR(kIOPSMaxCapacityKey));
        CFNumberGetValue((CFNumberRef)psValue, kCFNumberSInt32Type, &maxCapacity);

        QBatteryInfo::BatteryStatus stat = QBatteryInfo::BatteryStatusUnknown;

        if (curCapacityPercent < 2) {
            stat = QBatteryInfo::BatteryEmpty;
        } else if (curCapacityPercent < 11) {
             stat =  QBatteryInfo::BatteryLow;
        } else if (curCapacityPercent > 10 && curCapacityPercent < 100) {
             stat = QBatteryInfo::BatteryOk;
        } else if (curCapacityPercent == 100) {
             stat = QBatteryInfo::BatteryFull;
        }
        if (batteryStatuses.value(i) != stat) {
            batteryStatuses[i] = stat;
            Q_EMIT batteryStatusChanged(i,stat);
        }

        cVoltage = [[(NSDictionary*)batDoctionary objectForKey:@kIOPSVoltageKey] intValue];

        if (cVoltage != voltages.value(i) && cVoltage != 0) {
            voltages[i] = cVoltage;
            Q_EMIT(voltageChanged(i, cVoltage));
        }

        cEnergy = [[(NSDictionary*)batDoctionary objectForKey:@kIOPSCurrentKey] doubleValue];

        if (cEnergy != currentFlows.value(i) && cEnergy != 0) {
            currentFlows[i] = cEnergy;
            Q_EMIT currentFlowChanged(i,cEnergy);
        }

        cTime = [[(NSDictionary*)batDoctionary objectForKey:@kIOPSTimeToFullChargeKey] intValue];

        if (cTime != remainingChargingTimes.value(i)) {
            remainingChargingTimes[i] = cTime * 60;
            Q_EMIT remainingChargingTimeChanged(i,remainingChargingTimes[i]);
        }

        rEnergy = [[(NSDictionary*)batDoctionary objectForKey:@"CurrentCapacity"] intValue];

        if (rEnergy != remainingCapacities.value(i) && rEnergy != 0) {
            remainingCapacities[i] = rEnergy;
            Q_EMIT remainingCapacityChanged(i,remainingCapacities[i]);
        }
        int max = rEnergy / ((qreal)(curCapacityPercent) / 100);
        maximumCapacities[i] = max;
    }

    if (batteries == 0) {
        maximumCapacities[0] = -1;
        remainingCapacities[0] = -1;
        remainingChargingTimes[0] = -1;
        voltages[0] = -1;
        chargingStates[0] = QBatteryInfo::UnknownChargingState;
        batteryStatuses[0] = QBatteryInfo::BatteryStatusUnknown;
    }
    if (batDoctionary != NULL)
        CFRelease(batDoctionary);
    CFRelease(info);
    CFRelease(list);
}

QT_END_NAMESPACE
