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

#ifndef QDECLARATIVEDEVICEINFO_P_H
#define QDECLARATIVEDEVICEINFO_P_H

#include <qdeviceinfo.h>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QDeclarativeDeviceInfo : public QObject
{
    Q_OBJECT

    Q_ENUMS(Feature)
    Q_ENUMS(LockType)
    Q_ENUMS(ThermalState)
    Q_ENUMS(Version)

    Q_FLAGS(LockType LockTypeFlags)

    Q_PROPERTY(bool monitorActivatedLocks READ monitorActivatedLocks WRITE setMonitorActivatedLocks NOTIFY monitorActivatedLocksChanged)
    Q_PROPERTY(bool monitorEnabledLocks READ monitorEnabledLocks WRITE setMonitorEnabledLocks NOTIFY monitorEnabledLocksChanged)
    Q_PROPERTY(bool monitorThermalState READ monitorThermalState WRITE setMonitorThermalState NOTIFY monitorThermalStateChanged)

    Q_PROPERTY(LockTypeFlags activatedLocks READ activatedLocks NOTIFY activatedLocksChanged)
    Q_PROPERTY(LockTypeFlags enabledLocks READ enabledLocks NOTIFY enabledLocksChanged)
    Q_PROPERTY(ThermalState thermalState READ thermalState NOTIFY thermalStateChanged)

public:
    enum Feature {
        Bluetooth = QDeviceInfo::Bluetooth,
        Camera = QDeviceInfo::Camera,
        FmRadio = QDeviceInfo::FmRadio,
        FmTransmitter = QDeviceInfo::FmTransmitter,
        Infrared = QDeviceInfo::Infrared,
        Led = QDeviceInfo::Led,
        MemoryCard = QDeviceInfo::MemoryCard,
        Usb = QDeviceInfo::Usb,
        Vibration = QDeviceInfo::Vibration,
        Wlan = QDeviceInfo::Wlan,
        Sim = QDeviceInfo::Sim,
        Positioning = QDeviceInfo::Positioning,
        VideoOut = QDeviceInfo::VideoOut,
        Haptics = QDeviceInfo::Haptics,
        Nfc = QDeviceInfo::Nfc
    };

    enum LockType {
        NoLock = QDeviceInfo::NoLock,
        PinLock = QDeviceInfo::PinLock,
        TouchOrKeyboardLock = QDeviceInfo::TouchOrKeyboardLock
    };
    Q_DECLARE_FLAGS(LockTypeFlags, LockType)

    enum ThermalState {
        UnknownThermal = QDeviceInfo::UnknownThermal,
        NormalThermal = QDeviceInfo::NormalThermal,
        WarningThermal = QDeviceInfo::WarningThermal,
        AlertThermal = QDeviceInfo::AlertThermal,
        ErrorThermal = QDeviceInfo::ErrorThermal
    };

    enum Version {
        Os = QDeviceInfo::Os,
        Firmware = QDeviceInfo::Firmware
    };

    QDeclarativeDeviceInfo(QObject *parent = 0);
    virtual ~QDeclarativeDeviceInfo();

    bool monitorActivatedLocks() const;
    void setMonitorActivatedLocks(bool monitor);
    LockTypeFlags activatedLocks() const;

    bool monitorEnabledLocks() const;
    void setMonitorEnabledLocks(bool monitor);
    LockTypeFlags enabledLocks() const;

    bool monitorThermalState() const;
    void setMonitorThermalState(bool monitor);
    ThermalState thermalState() const;

    Q_INVOKABLE bool hasFeature(Feature feature) const;
    Q_INVOKABLE int imeiCount() const;
    Q_INVOKABLE QString imei(int interface) const;
    Q_INVOKABLE QString manufacturer() const;
    Q_INVOKABLE QString model() const;
    Q_INVOKABLE QString productName() const;
    Q_INVOKABLE QString uniqueDeviceID() const;
    Q_INVOKABLE QString version(Version type) const;

Q_SIGNALS:
    void monitorActivatedLocksChanged();
    void monitorEnabledLocksChanged();
    void monitorThermalStateChanged();

    void activatedLocksChanged();
    void enabledLocksChanged();
    void thermalStateChanged();

private:
    QDeviceInfo *deviceInfo;

    bool isMonitorActivatedLocks;
    bool isMonitorEnabledLocks;
    bool isMonitorThermalState;
};

QT_END_NAMESPACE
QT_END_HEADER

#endif // QDECLARATIVEDEVICEINFO_P_H
