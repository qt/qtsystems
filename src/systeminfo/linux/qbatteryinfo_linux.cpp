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

#include "qbatteryinfo_linux_p.h"

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qtimer.h>

#if !defined(QT_NO_UDEV)
#include "qudevwrapper_p.h"
#endif // QT_NO_UDEV

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(const QString, AC_ONLINE_SYSFS_PATH, (QLatin1String("/sys/class/power_supply/AC/online")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, BATTERY_SYSFS_PATH, (QLatin1String("/sys/class/power_supply/BAT%1/")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, POWER_SUPPLY_SYSFS_PATH, (QLatin1String("/sys/class/power_supply/")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, USB_PRESENT_SYSFS_PATH, (QLatin1String("/sys/class/power_supply/usb/present")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, USB_TYPE_SYSFS_PATH, (QLatin1String("/sys/class/power_supply/usb/type")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, USB0_PRESENT_SYSFS_PATH, (QLatin1String("/sys/class/power_supply/USB0/present")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, USB0_TYPE_SYSFS_PATH, (QLatin1String("/sys/class/power_supply/USB0/type")))

QBatteryInfoPrivate::QBatteryInfoPrivate(QBatteryInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
    , watchBatteryCount(false)
    , watchChargerType(false)
    , watchChargingState(false)
    , watchCurrentFlow(false)
    , watchRemainingCapacity(false)
    , watchRemainingChargingTime(false)
    , watchVoltage(false)
    , watchLevelStatus(false)
    , batteryCounts(-1)
    , index(0)
    , currentChargerType(QBatteryInfo::UnknownCharger)
#if !defined(QT_NO_UDEV)
    , uDevWrapper(0)
#else
    , timer(0)
#endif // QT_NO_UDEV
{
}

QBatteryInfoPrivate::QBatteryInfoPrivate(int batteryIndex, QBatteryInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
    , watchBatteryCount(false)
    , watchChargerType(false)
    , watchChargingState(false)
    , watchCurrentFlow(false)
    , watchRemainingCapacity(false)
    , watchRemainingChargingTime(false)
    , watchVoltage(false)
    , watchLevelStatus(false)
    , batteryCounts(-1)
    , index(batteryIndex)
    , currentChargerType(QBatteryInfo::UnknownCharger)
#if !defined(QT_NO_UDEV)
    , uDevWrapper(0)
#else
    , timer(0)
#endif // QT_NO_UDEV
{
}

QBatteryInfoPrivate::~QBatteryInfoPrivate()
{
#if defined(QT_NO_UDEV)
    delete timer;
#endif // QT_NO_UDEV
}

int QBatteryInfoPrivate::batteryCount()
{
    if (!watchBatteryCount)
        return getBatteryCount();

    return batteryCounts;
}

int QBatteryInfoPrivate::batteryIndex() const
{
    return index;
}

void QBatteryInfoPrivate::setBatteryIndex(int batteryIndex)
{
    if (index != batteryIndex) {
        index = batteryIndex;
        emit batteryIndexChanged(index);
    }
}

int QBatteryInfoPrivate::currentFlow(int battery)
{
    if (!watchCurrentFlow)
        return getCurrentFlow(battery);

    return currentFlows.value(battery);
}

int QBatteryInfoPrivate::currentFlow()
{
    return currentFlow(index);
}

int QBatteryInfoPrivate::maximumCapacity(int battery)
{
    if (maximumCapacities[battery] == 0) {
        QFile maximum(BATTERY_SYSFS_PATH()->arg(battery) + QStringLiteral("charge_full"));
        if (maximum.open(QIODevice::ReadOnly)) {
            bool ok = false;
            int capacity = maximum.readAll().simplified().toInt(&ok);
            if (ok)
                maximumCapacities[battery] = capacity / 1000;
            else
                maximumCapacities[battery] = -1;
        } else {
            maximumCapacities[battery] = -1;
        }
    }

    return maximumCapacities[battery];
}

int QBatteryInfoPrivate::maximumCapacity()
{
    return maximumCapacity(index);
}

int QBatteryInfoPrivate::remainingCapacity(int battery)
{
    if (!watchRemainingCapacity)
        return getRemainingCapacity(battery);

    return remainingCapacities.value(battery);
}

int QBatteryInfoPrivate::remainingCapacity()
{
    return remainingCapacity(index);
}

int QBatteryInfoPrivate::remainingChargingTime(int battery)
{
    if (!watchRemainingChargingTime)
        return getRemainingChargingTime(battery);

    return remainingChargingTimes.value(battery);
}

int QBatteryInfoPrivate::remainingChargingTime()
{
    return remainingChargingTime(index);
}

int QBatteryInfoPrivate::voltage(int battery)
{
    if (!watchVoltage)
        return getVoltage(battery);

    return voltages.value(battery);
}

int QBatteryInfoPrivate::voltage()
{
    return voltage(index);
}

QBatteryInfo::ChargerType QBatteryInfoPrivate::chargerType()
{
    if (!watchChargerType)
        return getChargerType();

    return currentChargerType;
}

QBatteryInfo::ChargingState QBatteryInfoPrivate::chargingState(int battery)
{
    if (!watchChargingState)
        return getChargingState(battery);

    return chargingStates.value(battery);
}

QBatteryInfo::ChargingState QBatteryInfoPrivate::chargingState()
{
    return chargingState(index);
}

QBatteryInfo::LevelStatus QBatteryInfoPrivate::levelStatus(int battery)
{
    if (!watchLevelStatus)
        return getLevelStatus(battery);

    return levelStatuss.value(battery);
}

QBatteryInfo::LevelStatus QBatteryInfoPrivate::levelStatus()
{
    return levelStatus(index);
}

QBatteryInfo::Health QBatteryInfoPrivate::health()
{
    return QBatteryInfo::UnknownHealth;
}

void QBatteryInfoPrivate::connectNotify(const QMetaMethod &signal)
{
    static const QMetaMethod batteryCountChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::batteryCountChanged);
    static const QMetaMethod chargerTypeChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::chargerTypeChanged);
    static const QMetaMethod chargingStateChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::chargingStateChanged);
    static const QMetaMethod currentFlowChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::currentFlowChanged);
    static const QMetaMethod remainingCapacityChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::remainingCapacityChanged);
    static const QMetaMethod remainingChargingTimeChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::remainingChargingTimeChanged);
    static const QMetaMethod voltageChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::voltageChanged);
    static const QMetaMethod levelStatusChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::levelStatusChanged);

#if !defined(QT_NO_UDEV)
    if (!uDevWrapper)
        uDevWrapper = new QUDevWrapper(this);
    if (!watchChargerType && signal == chargerTypeChangedSignal) {
        connect(uDevWrapper, SIGNAL(chargerTypeChanged(QByteArray,bool)), this, SLOT(onChargerTypeChanged(QByteArray,bool)));
    } else if (!watchCurrentFlow && !watchVoltage && !watchChargingState && !watchRemainingCapacity
               && !watchRemainingChargingTime && !watchBatteryCount && !watchLevelStatus) {
        connect(uDevWrapper, SIGNAL(batteryDataChanged(int,QByteArray,QByteArray)), this, SLOT(onBatteryDataChanged(int,QByteArray,QByteArray)));
    }
#else
    if (timer == 0) {
       timer = new QTimer;
       timer->setInterval(2000);
       connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    }

    if (!timer->isActive())
       timer->start();
#endif // QT_NO_UDEV

    if (signal == batteryCountChangedSignal) {
        watchBatteryCount = true;
        batteryCounts = getBatteryCount();
    } else if (signal == currentFlowChangedSignal) {
        watchCurrentFlow = true;
        int count = batteryCount();
        for (int i = 0; i < count; ++i)
            currentFlows[i] = getCurrentFlow(i);
    } else if (signal == voltageChangedSignal) {
        watchVoltage = true;
        int count = batteryCount();
        for (int i = 0; i < count; ++i)
            voltages[i] = getVoltage(i);
    } else if (signal == remainingCapacityChangedSignal) {
        watchRemainingCapacity = true;
        int count = batteryCount();
        for (int i = 0; i < count; ++i)
            remainingCapacities[i] = getRemainingCapacity(i);
    } else if (signal == remainingChargingTimeChangedSignal) {
        watchRemainingChargingTime = true;
        int count = batteryCount();
        for (int i = 0; i < count; ++i)
            remainingChargingTimes[i] = getRemainingChargingTime(i);
    } else if (signal == chargerTypeChangedSignal) {
        watchChargerType = true;
        currentChargerType = getChargerType();
    } else if (signal == chargingStateChangedSignal) {
        watchChargingState = true;
        int count = batteryCount();
        for (int i = 0; i < count; ++i)
            chargingStates[i] = getChargingState(i);
    } else if (signal == levelStatusChangedSignal) {
        watchLevelStatus = true;
        int count = batteryCount();
        for (int i = 0; i < count; i++)
            levelStatuss[i] = getLevelStatus(i);
    }
}

void QBatteryInfoPrivate::disconnectNotify(const QMetaMethod &signal)
{
    static const QMetaMethod batteryCountChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::batteryCountChanged);
    static const QMetaMethod chargerTypeChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::chargerTypeChanged);
    static const QMetaMethod chargingStateChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::chargingStateChanged);
    static const QMetaMethod currentFlowChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::currentFlowChanged);
    static const QMetaMethod remainingCapacityChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::remainingCapacityChanged);
    static const QMetaMethod remainingChargingTimeChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::remainingChargingTimeChanged);
    static const QMetaMethod voltageChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::voltageChanged);
    static const QMetaMethod levelStatusChangedSignal = QMetaMethod::fromSignal(&QBatteryInfoPrivate::levelStatusChanged);
    if (signal == batteryCountChangedSignal) {
        watchBatteryCount = false;
        batteryCounts = -1;
    } else if (signal == currentFlowChangedSignal) {
        watchCurrentFlow = false;
        currentFlows.clear();
    } else if (signal == voltageChangedSignal) {
        watchVoltage = false;
        voltages.clear();
    } else if (signal == remainingCapacityChangedSignal) {
        watchRemainingCapacity = false;
        remainingCapacities.clear();
    } else if (signal == remainingChargingTimeChangedSignal) {
        watchRemainingChargingTime = false;
        remainingChargingTimes.clear();
    } else if (signal == chargerTypeChangedSignal) {
        watchChargerType = false;
        currentChargerType = QBatteryInfo::UnknownCharger;
    } else if (signal == chargingStateChangedSignal) {
        watchChargingState = false;
        chargingStates.clear();
    } else if (signal == levelStatusChangedSignal) {
        watchLevelStatus = false;
        levelStatuss.clear();
    }

#if !defined(QT_NO_UDEV)
    if (uDevWrapper && !watchChargerType && signal == chargerTypeChangedSignal) {
        disconnect(uDevWrapper, SIGNAL(chargerTypeChanged(QByteArray,bool)),
                   this, SLOT(onChargerTypeChanged(QByteArray,bool)));
    } else if (uDevWrapper && !watchCurrentFlow && !watchVoltage && !watchChargingState && !watchRemainingCapacity
               && !watchRemainingChargingTime && !watchBatteryCount && !watchLevelStatus) {
        disconnect(uDevWrapper, SIGNAL(batteryDataChanged(int,QByteArray,QByteArray)),
                   this, SLOT(onBatteryDataChanged(int,QByteArray,QByteArray)));
    }
#endif

    if (!watchBatteryCount && !watchChargerType && !watchChargingState
            && !watchCurrentFlow && !watchRemainingCapacity
            && !watchRemainingChargingTime && !watchVoltage && !watchLevelStatus) {
#if !defined(QT_NO_UDEV)
        if (uDevWrapper) {
            delete uDevWrapper;
            uDevWrapper = 0;
        }
#else
        timer->stop();
#endif // QT_NO_UDEV
    }
}

#if !defined(QT_NO_UDEV)

void QBatteryInfoPrivate::onBatteryDataChanged(int battery, const QByteArray &attribute, const QByteArray &value)
{
    if (watchBatteryCount) {
        int count = getBatteryCount();
        if (batteryCounts != count) {
            batteryCounts = count;
            emit batteryCountChanged(count);
        }
    }

    if (watchChargingState && attribute.contains("status")) {
        QBatteryInfo::ChargingState state = QBatteryInfo::UnknownChargingState;
        if (qstrcmp(value, "Charging") == 0)
            state = QBatteryInfo::Charging;
        else if (qstrcmp(value, "Not charging") == 0)
            state = QBatteryInfo::IdleChargingState;
        else if (qstrcmp(value, "Discharging") == 0)
            state = QBatteryInfo::Discharging;
        else if (qstrcmp(value, "Full") == 0)
            state = QBatteryInfo::IdleChargingState;
        if (chargingStates.value(battery) != state) {
            chargingStates[battery] = state;
            if (battery == index)
                emit chargingStateChanged(state);
        }
    }

    if (watchRemainingCapacity && attribute.contains("charge_now")) {
        if (!value.isEmpty()) {
            int remainingCapacity = value.toInt() / 1000;
            if (remainingCapacities.value(battery) != remainingCapacity) {
                remainingCapacities[battery] = remainingCapacity;
                if (battery == index)
                    emit remainingCapacityChanged(remainingCapacity);
            }
        }
    }

    if (watchRemainingChargingTime && attribute.contains("time_to_full_avg")) {
        if (!value.isEmpty()) {
            int remainingChargingTime = value.toInt();
            if (remainingChargingTimes.value(battery) != remainingChargingTime) {
                remainingChargingTimes[battery] = remainingChargingTime;
                if (battery == index)
                    emit remainingChargingTimeChanged(remainingChargingTime);
            }
        }
    }

    if (watchVoltage && attribute.contains("voltage_now")) {
        if (!value.isEmpty()) {
            int voltage = value.toInt() / 1000;
            if (voltages.value(battery) != voltage) {
                voltages[battery] = voltage;
                if (battery == index)
                    emit voltageChanged(voltage);
            }
        }
    }

    if (watchCurrentFlow && attribute.contains("current_now")) {
        if (!value.isEmpty()) {
            int currentFlow = value.toInt() / -1000;
            if (chargingStates.value(battery) == QBatteryInfo::Discharging && currentFlow < 0)
                currentFlow = -currentFlow;

            if (currentFlows.value(battery) != currentFlow) {
                currentFlows[battery] = currentFlow;
                if (battery == index)
                    emit currentFlowChanged(currentFlow);
            }
        }
    }

    if (watchLevelStatus && attribute.contains("capacity_level")) {
        QBatteryInfo::LevelStatus levelStatus = QBatteryInfo::LevelUnknown;
        if (qstrcmp(value, "Critical") == 0)
            levelStatus = QBatteryInfo::LevelEmpty;
        else if (qstrcmp(value, "Low") == 0)
            levelStatus = QBatteryInfo::LevelLow;
        else if (qstrcmp(value, "Normal") == 0)
            levelStatus = QBatteryInfo::LevelOk;
        else if (qstrcmp(value, "Full") == 0)
            levelStatus = QBatteryInfo::LevelFull;
        if (levelStatuss.value(battery) != levelStatus) {
            levelStatuss[battery] = levelStatus;
            if (battery == index)
                emit levelStatusChanged(levelStatus);
        }
    }
}

void QBatteryInfoPrivate::onChargerTypeChanged(const QByteArray &value, bool enabled)
{
    if (watchChargerType) {
        QBatteryInfo::ChargerType charger = QBatteryInfo::UnknownCharger;
        if (enabled) {
            if ((qstrcmp(value, "AC") == 0) || qstrcmp(value, "USB_DCP") == 0)
                charger = QBatteryInfo::WallCharger;
            else if (qstrcmp(value, "USB") == 0)
                charger = QBatteryInfo::USBCharger;
            else if (qstrcmp(value, "USB_CDP") == 0 || qstrcmp(value, "USB_SDP") == 0)
                charger = QBatteryInfo::VariableCurrentCharger;
        }
        if (currentChargerType != charger) {
            currentChargerType = charger;
            emit chargerTypeChanged(charger);
        }
    }
}

#else

void QBatteryInfoPrivate::onTimeout()
{
    int count = getBatteryCount();
    int value;
    if (watchBatteryCount) {
        value = getBatteryCount();
        if (batteryCounts != value) {
            batteryCounts = value;
            emit batteryCountChanged(value);
        }
    }

    for (int i = 0; i < count; ++i) {
        if (watchCurrentFlow) {
            value = getCurrentFlow(i);
            if (currentFlows.value(i) != value) {
                currentFlows[i] = value;
                if (i == index)
                    emit currentFlowChanged(value);
            }
        }

        if (watchVoltage) {
            value = getVoltage(i);
            if (voltages.value(i) != value) {
                voltages[i] = value;
                if (i == index)
                    emit voltageChanged(value);
            }
        }

        if (watchRemainingCapacity) {
            value = getRemainingCapacity(i);
            if (remainingCapacities.value(i) != value) {
                remainingCapacities[i] = value;
                if (i == index)
                    emit remainingCapacityChanged(value);
            }
        }

        if (watchRemainingChargingTime) {
            value = getRemainingChargingTime(i);
            if (remainingChargingTimes.value(i) != value) {
                remainingChargingTimes[i] = value;
                if (i == index)
                    emit remainingChargingTimeChanged(value);
            }
        }

        if (watchChargerType) {
            QBatteryInfo::ChargerType charger = getChargerType();
            if (currentChargerType != charger) {
                currentChargerType = charger;
                emit chargerTypeChanged(charger);
            }
        }

        if (watchChargingState) {
            QBatteryInfo::ChargingState state = getChargingState(i);
            if (chargingStates.value(i) != state) {
                chargingStates[i] = state;
                if (i == index)
                    emit chargingStateChanged(state);
            }
        }

        if (watchLevelStatus) {
            QBatteryInfo::LevelStatus levelStatus = getLevelStatus(i);
            if (levelStatuss.value(i) != levelStatus) {
                levelStatuss[i] = levelStatus;
                if (i == index)
                    emit levelStatusChanged(levelStatus);
            }
        }
    }
}

#endif // QT_NO_UDEV

int QBatteryInfoPrivate::getBatteryCount()
{
    return QDir(*POWER_SUPPLY_SYSFS_PATH()).entryList(QStringList() << QStringLiteral("BAT*")).size();
}

int QBatteryInfoPrivate::getCurrentFlow(int battery)
{
    QBatteryInfo::ChargingState state = chargingState(battery);
    if (state == QBatteryInfo::UnknownChargingState)
        return 0;

    QFile current(BATTERY_SYSFS_PATH()->arg(battery) + QStringLiteral("current_now"));
    if (!current.open(QIODevice::ReadOnly))
        return 0;

    bool ok = false;
    int flow = current.readAll().simplified().toInt(&ok);
    if (ok) {
        // We want discharging current to be positive and charging current to be negative.
        if (state == QBatteryInfo::Charging) {
          // In case some drivers make charging current negative already and others are opposite
          return flow < 0 ? flow / 1000 : flow / -1000;
        } else if (state == QBatteryInfo::Discharging) {
          // In case some drivers make discharging current positive already and others are opposite
          return flow > 0 ? flow / 1000 : flow / -1000;
        }
    }

    return 0;
}

int QBatteryInfoPrivate::getRemainingCapacity(int battery)
{
    QFile remaining(BATTERY_SYSFS_PATH()->arg(battery) + QStringLiteral("charge_now"));
    if (!remaining.open(QIODevice::ReadOnly))
        return -1;

    bool ok = false;
    int capacity = remaining.readAll().simplified().toInt(&ok);
    if (ok)
        return capacity / 1000;
    return -1;
}

int QBatteryInfoPrivate::getRemainingChargingTime(int battery)
{
    QBatteryInfo::ChargingState state = chargingState(battery);
    if (state == QBatteryInfo::UnknownChargingState)
        return -1;
    else if (state == QBatteryInfo::IdleChargingState || state == QBatteryInfo::Discharging)
        return 0;

    int remaining = 0;
    QFile timeToFull(BATTERY_SYSFS_PATH()->arg(battery) + QStringLiteral("time_to_full_avg"));
    if (timeToFull.open(QIODevice::ReadOnly)) {
        bool ok = false;
        remaining = timeToFull.readAll().simplified().toInt(&ok);
        if (ok)
            return remaining;
        return -1;
    }

    int max = 0;
    int current = 0;
    if ((max = maximumCapacity(battery)) == -1
        || (remaining = remainingCapacity(battery)) == -1
        || (current = currentFlow(battery)) == 0) {
        return -1;
    }
    return (max - remaining) * -3600 / current;
}

int QBatteryInfoPrivate::getVoltage(int battery)
{
    QFile current(BATTERY_SYSFS_PATH()->arg(battery) + QStringLiteral("voltage_now"));
    if (!current.open(QIODevice::ReadOnly))
        return -1;

    bool ok = false;
    int voltage = current.readAll().simplified().toInt(&ok);
    if (ok)
        return voltage / 1000;
    return -1;
}

QBatteryInfo::ChargerType QBatteryInfoPrivate::getChargerType()
{
    QFile charger(*AC_ONLINE_SYSFS_PATH());
    if (charger.open(QIODevice::ReadOnly)) {
        char online;
        if (charger.read(&online, 1) == 1 && online == '1')
            return QBatteryInfo::WallCharger;
        charger.close();
    }

    QMap<QString, QString> chargerMap;
    chargerMap.insert(*USB0_PRESENT_SYSFS_PATH(), *USB0_TYPE_SYSFS_PATH());
    chargerMap.insert(*USB_PRESENT_SYSFS_PATH(), *USB_TYPE_SYSFS_PATH());

    QList<QString> presentPaths = chargerMap.keys();
    foreach (const QString &presentPath, presentPaths) {
        charger.setFileName(presentPath);
        if (charger.open(QIODevice::ReadOnly)) {
            char present;
            if (charger.read(&present, 1) == 1 && present == '1') {
                charger.close();

                charger.setFileName(chargerMap.value(presentPath));
                if (charger.open(QIODevice::ReadOnly)) {
                    if (charger.readAll().simplified() == "USB_DCP")
                        return QBatteryInfo::WallCharger;
                    return QBatteryInfo::USBCharger;
                }
            }
            charger.close();
        }
    }

    return QBatteryInfo::UnknownCharger;
}

QBatteryInfo::ChargingState QBatteryInfoPrivate::getChargingState(int battery)
{
    QFile state(BATTERY_SYSFS_PATH()->arg(battery) + QStringLiteral("status"));
    if (!state.open(QIODevice::ReadOnly))
        return QBatteryInfo::UnknownChargingState;

    QByteArray status = state.readAll().simplified();
    if (status == "Charging")
        return QBatteryInfo::Charging;
    else if (status == "Not charging")
        return QBatteryInfo::IdleChargingState;
    else if (status == "Discharging")
        return QBatteryInfo::Discharging;
    else if (status == "Full")
        return QBatteryInfo::IdleChargingState;

    return QBatteryInfo::UnknownChargingState;
}

QBatteryInfo::LevelStatus QBatteryInfoPrivate::getLevelStatus(int battery)
{
    QFile levelStatusFile(BATTERY_SYSFS_PATH()->arg(battery) + QStringLiteral("capacity_level"));
    if (!levelStatusFile.open(QIODevice::ReadOnly))
        return QBatteryInfo::LevelUnknown;

    QByteArray levelStatus = levelStatusFile.readAll().simplified();
    if (qstrcmp(levelStatus, "Critical") == 0)
        return QBatteryInfo::LevelEmpty;
    else if (qstrcmp(levelStatus, "Low") == 0)
        return QBatteryInfo::LevelLow;
    else if (qstrcmp(levelStatus, "Normal") == 0)
        return QBatteryInfo::LevelOk;
    else if (qstrcmp(levelStatus, "Full") == 0)
        return QBatteryInfo::LevelFull;

    return QBatteryInfo::LevelUnknown;
}

QT_END_NAMESPACE
