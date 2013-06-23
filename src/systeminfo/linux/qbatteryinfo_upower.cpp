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

#include "qbatteryinfo_upower_p.h"

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qtimer.h>
#include <QFile>
#include "qdevicekitservice_linux_p.h"


QT_BEGIN_NAMESPACE

QBatteryInfoPrivate::QBatteryInfoPrivate(QBatteryInfo *parent)
    : QObject(parent)
    , q_ptr(parent),
      cType(QBatteryInfo::UnknownCharger),
      cState(QBatteryInfo::UnknownChargingState)
{

    watcher = new QDBusServiceWatcher("org.freedesktop.UPower",QDBusConnection::systemBus(),
                                      QDBusServiceWatcher::WatchForRegistration |
                                      QDBusServiceWatcher::WatchForUnregistration, this);
    connect(watcher, SIGNAL(serviceRegistered(QString)),
            this, SLOT(connectToUpower()));
    connect(watcher, SIGNAL(serviceUnregistered(QString)),
            this, SLOT(disconnectFromUpower()));

    bool uPowerAvailable = QDBusConnection::systemBus().interface()->isServiceRegistered("org.freedesktop.UPower");

    if (uPowerAvailable)
        connectToUpower();
}

QBatteryInfoPrivate::~QBatteryInfoPrivate()
{
}

void QBatteryInfoPrivate::connectToUpower()
{
    getBatteryStats();
}

void QBatteryInfoPrivate::disconnectFromUpower()
{
}

int QBatteryInfoPrivate::batteryCount()
{
    return batteryMap.count();
}

int QBatteryInfoPrivate::currentFlow(int battery)
{
    if (batteryMap.count() >= battery)
        return (batteryMap.value(battery).value(QStringLiteral("EnergyRate")).toDouble()
                / (batteryMap.value(battery).value(QStringLiteral("Voltage")).toInt()) * 1000);
    else
        return 0;
}

int QBatteryInfoPrivate::maximumCapacity(int battery)
{
    if (batteryMap.count() >= battery)
        return batteryMap.value(battery).value(QStringLiteral("EnergyFull")).toInt() * 1000;
    else
        return 0;
}

int QBatteryInfoPrivate::remainingCapacity(int battery)
{
    if (batteryMap.count() >= battery)
        return batteryMap.value(battery).value(QStringLiteral("Energy")).toInt() * 1000;
    else
        return 0;
}

int QBatteryInfoPrivate::remainingChargingTime(int battery)
{
    if (batteryMap.count() >= battery)
        return batteryMap.value(battery).value(QStringLiteral("TimeToFull")).toInt();
    else
        return 0;
}

int QBatteryInfoPrivate::voltage(int battery)
{
    if (batteryMap.count() >= battery)
        return (batteryMap.value(battery).value(QStringLiteral("Voltage")).toInt() * 1000);
    else
        return 0;
}

QBatteryInfo::ChargerType QBatteryInfoPrivate::chargerType()
{
    return cType;
}

QBatteryInfo::ChargingState QBatteryInfoPrivate::chargingState(int battery)
{

    return cState;
}

QBatteryInfo::EnergyUnit QBatteryInfoPrivate::energyUnit()
{
    return QBatteryInfo::UnitmAh;
}

QBatteryInfo::BatteryStatus QBatteryInfoPrivate::batteryStatus(int battery)
{
    QBatteryInfo::BatteryStatus stat = QBatteryInfo::BatteryStatusUnknown;
    if (batteryMap.count() >= battery) {
        int level = batteryMap.value(battery).value(QStringLiteral("Percentage")).toInt();
        if (level < 2)
            stat = QBatteryInfo::BatteryEmpty;
        else if (level > 1 && level < 11)
            stat = QBatteryInfo::BatteryLow;
        else if (level > 10 && level < 99)
            stat = QBatteryInfo::BatteryOk;
        else if (level == 100)
            stat = QBatteryInfo::BatteryFull;
    }
    return stat;
}

void QBatteryInfoPrivate::upowerChanged()
{
//    QUPowerInterface *uPower = qobject_cast<QUPowerInterface*>(sender());
//    if (uPower->onBattery()) {
//        chargerTypeChanged();
//    }
////    if (uPowerAvailable()) {
//        QBatteryInfo::ChargingState pState = QBatteryInfo::UnknownChargingState;

//        QUPowerInterface power(this);
//        foreach (const QDBusObjectPath &objpath, power.enumerateDevices()) {
//            QUPowerDeviceInterface powerDevice(objpath.path(),this);
//            if (powerDevice.getType() == 2) {
//                switch (powerDevice.getState()) {
//                case 0:
//                    break;
//                case 1:
//                case 5:
//                    pState = QBatteryInfo::Charging;
//                    break;
//                case 2:
//                case 6:
//                    pState = QBatteryInfo::Discharging;
//                    break;
//                case 4:
//                    pState = QBatteryInfo::Full;
//                    break;
//                default:
//                    pState = QBatteryInfo::UnknownChargingState;
//                };
//            }
//        }
//        if (!power.onBattery() && pState == QBatteryInfo::UnknownChargingState)
//            pState = QBatteryInfo::NotCharging;
//        if (curPowerState != pState) {
//            curPowerState = pState;
//            Q_EMIT powerStateChanged(pState);
//        }
//        return pState;
 //   }
}

void QBatteryInfoPrivate::upowerDeviceChanged()
{
    QUPowerDeviceInterface *uPowerDevice = qobject_cast<QUPowerDeviceInterface*>(sender());

    if (uPowerDevice->type() == 1) {
//line power
        if (uPowerDevice->nativePath().contains(QStringLiteral("usb")))
            Q_EMIT chargerTypeChanged(QBatteryInfo::USBCharger);
        else
            Q_EMIT chargerTypeChanged(QBatteryInfo::WallCharger);
    }
    if (uPowerDevice->type() == 2) {
//battery
    }
}
void QBatteryInfoPrivate::uPowerBatteryPropertyChanged(const QString &prop, const QVariant &v)
{
    QUPowerDeviceInterface *uPowerDevice = qobject_cast<QUPowerDeviceInterface*>(sender());

    int foundBattery = 0;
    QMapIterator<int, QVariantMap> i(batteryMap);
    while (i.hasNext()) {
        i.next();
        if (i.value().value(QStringLiteral("NativePath")).toString() == uPowerDevice->nativePath()) {
            foundBattery = i.key();
            break;
        }
    }

    QVariantMap foundMap = batteryMap.value(foundBattery);
    foundMap.insert(prop,v);
    batteryMap.insert(foundBattery,foundMap);

    if (prop == QLatin1String("Energy")) {
        Q_EMIT remainingCapacityChanged(foundBattery, v.toDouble() * 1000);

    } else if (prop == QLatin1String("EnergyRate")) {
        Q_EMIT currentFlowChanged(foundBattery, v.toDouble() / (uPowerDevice->voltage() * 1000));

    } else if (prop == QLatin1String("Percentage")) {
        int level = v.toInt();
        //  Q_EMIT remainingCapacityChanged(foundBattery, level);

        QBatteryInfo::BatteryStatus stat = QBatteryInfo::BatteryStatusUnknown;

        if (level < 2)
            stat = QBatteryInfo::BatteryEmpty;
        else if (level > 1 && level < 11)
            stat = QBatteryInfo::BatteryLow;
        else if (level > 10 && level < 99)
            stat = QBatteryInfo::BatteryOk;
        else if (level == 100)
            stat = QBatteryInfo::BatteryFull;

        //   if (batteryMap.value(foundBattery).value(QStringLiteral("Percentage")).toInt() != stat) {
        Q_EMIT batteryStatusChanged(foundBattery, stat);
        //   }

    } else if (prop == QLatin1String("Voltage")) {
        Q_EMIT voltageChanged(foundBattery,v.toDouble() * 1000 );

    } else if (prop == QLatin1String("State")) {

        QBatteryInfo::ChargingState curChargeState = getCurrentChargingState(v.toInt());

        if (curChargeState != cState) {
            cState = curChargeState;
            Q_EMIT chargingStateChanged(foundBattery,curChargeState);
        }

        } else if (prop == QLatin1String("Capacity")) {
        qDebug() << "Your battery just got less capacity";
    } else if (prop == QLatin1String("TimeToFull")) {

        Q_EMIT remainingChargingTimeChanged(foundBattery,v.toInt());

    } else if (prop == QLatin1String("Type")) {
        if (uPowerDevice->isOnline()) {
            QBatteryInfo::ChargerType curCharger = curCharger = getChargerType(uPowerDevice->nativePath());
            if (curCharger != cType) {
                cType = curCharger;
                Q_EMIT chargerTypeChanged(cType);
            }
        }
    }
}

QBatteryInfo::ChargerType QBatteryInfoPrivate::getChargerType(const QString &path)
{
    QFile charger;
    QBatteryInfo::ChargerType chargerType = QBatteryInfo::UnknownCharger;
    charger.setFileName(path + "/type");
    if (charger.open(QIODevice::ReadOnly)) {
        QString line = charger.readAll().simplified();
        if (line  == QStringLiteral("USB")) {
            chargerType = QBatteryInfo::USBCharger;

        } else if (line == QStringLiteral("Mains")) {
            chargerType = QBatteryInfo::WallCharger;
        }
    }
    charger.close();
    return chargerType;
}

QBatteryInfo::ChargingState QBatteryInfoPrivate::getCurrentChargingState(int state)
{
    QBatteryInfo::ChargingState curChargeState = QBatteryInfo::UnknownChargingState;
    switch (state) {
    case 1: // charging
    {
        curChargeState = QBatteryInfo::Charging;
    }
        break;
    case 2: //discharging
    case 3: //empty
        curChargeState = QBatteryInfo::Discharging;
        break;
    case 4: //fully charged
        curChargeState = QBatteryInfo::NotCharging;
        break;
    case 5: //pending charge
    case 6: //pending discharge
        break;
    default:
        curChargeState = QBatteryInfo::UnknownChargingState;
        break;
    };
    qDebug() << Q_FUNC_INFO << state << curChargeState;

    return curChargeState;
}

void QBatteryInfoPrivate::getBatteryStats()
{
    qDebug() << Q_FUNC_INFO;

    int batteryNumber = 0;
    batteryMap.clear();
    QUPowerInterface *power;
    power = new QUPowerInterface(this);

    connect(power,SIGNAL(changed()),
            this,SLOT(upowerChanged()));
    connect(power,SIGNAL(deviceAdded(QString)),
            this,SLOT(deviceAdded(QString)));
    connect(power,SIGNAL(deviceRemoved(QString)),
            this,SLOT(deviceRemoved(QString)));

    foreach (const QDBusObjectPath &objpath, power->enumerateDevices()) {
        QUPowerDeviceInterface *battery;
        battery = new QUPowerDeviceInterface(objpath.path(),this);

        if (!battery->isPowerSupply())
            continue;
        if (battery->type() == 1) { //line power
            cType = getChargerType(battery->nativePath());
        }
        if (battery->type() == 2) { //battery power

            batteryMap.insert(++batteryNumber,battery->getProperties());

            connect(battery,SIGNAL(changed()),this,SLOT(upowerDeviceChanged()));
            connect(battery,SIGNAL(propertyChanged(QString,QVariant)),
                    this,SLOT(uPowerBatteryPropertyChanged(QString,QVariant)));

            cState = getCurrentChargingState(battery->state());

        } //end enumerateDevices
    }
}

void QBatteryInfoPrivate::deviceAdded(const QString &path)
{
    QUPowerDeviceInterface *battery;
    battery = new QUPowerDeviceInterface(path,this);
    int batteryNumber = batteryCount();

    if (battery->type() == 2) {
        batteryMap.insert(++batteryNumber,battery->getProperties());
        connect(battery,SIGNAL(changed()),this,SLOT(upowerDeviceChanged()));
        connect(battery,SIGNAL(propertyChanged(QString,QVariant)),
                this,SLOT(uPowerBatteryPropertyChanged(QString,QVariant)));

    }
}

void QBatteryInfoPrivate::deviceRemoved(const QString &path)
{
    QUPowerDeviceInterface *battery;
    battery = new QUPowerDeviceInterface(path,this);

    int foundBattery = 0;
    QMapIterator<int, QVariantMap> i(batteryMap);
     while (i.hasNext()) {
         i.next();
         if (i.value().value(QStringLiteral("NativePath")).toString()
                 == battery->nativePath()) {
             foundBattery = i.key();
             break;
         }
     }

    if (battery->type() == 2) {
        batteryMap.remove(foundBattery);
        disconnect(battery,SIGNAL(changed()),this,SLOT(upowerDeviceChanged()));
        disconnect(battery,SIGNAL(propertyChanged(QString,QVariant)),
                this,SLOT(uPowerBatteryPropertyChanged(QString,QVariant)));
    }
}

QT_END_NAMESPACE
