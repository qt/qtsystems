/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
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

#include "qdeviceinfo.h"

QT_BEGIN_NAMESPACE

class QDeclarativeDeviceInfo : public QObject
{
    Q_OBJECT

    Q_ENUMS(Feature)
    Q_ENUMS(LockType)
    Q_ENUMS(ThermalState)
    Q_ENUMS(Version)

    Q_FLAGS(LockType LockTypeFlags)

    Q_PROPERTY(bool monitorThermalState READ monitorThermalState WRITE setMonitorThermalState NOTIFY monitorThermalStateChanged)

    Q_PROPERTY(LockTypeFlags activatedLocks READ activatedLocks NOTIFY activatedLocksChanged)
    Q_PROPERTY(LockTypeFlags enabledLocks READ enabledLocks NOTIFY enabledLocksChanged)
    Q_PROPERTY(ThermalState thermalState READ thermalState NOTIFY thermalStateChanged)

public:
    enum Feature {
        BluetoothFeature = QDeviceInfo::BluetoothFeature,
        CameraFeature = QDeviceInfo::CameraFeature,
        FmRadioFeature = QDeviceInfo::FmRadioFeature,
        FmTransmitterFeature = QDeviceInfo::FmTransmitterFeature,
        InfraredFeature = QDeviceInfo::InfraredFeature,
        LedFeature = QDeviceInfo::LedFeature,
        MemoryCardFeature = QDeviceInfo::MemoryCardFeature,
        UsbFeature = QDeviceInfo::UsbFeature,
        VibrationFeature = QDeviceInfo::VibrationFeature,
        WlanFeature = QDeviceInfo::WlanFeature,
        SimFeature = QDeviceInfo::SimFeature,
        PositioningFeature = QDeviceInfo::PositioningFeature,
        VideoOutFeature = QDeviceInfo::VideoOutFeature,
        HapticsFeature = QDeviceInfo::HapticsFeature,
        NfcFeature = QDeviceInfo::NfcFeature
    };

    enum LockType {
        NoLock = QDeviceInfo::NoLock,
        PinLock = QDeviceInfo::PinLock,
        TouchOrKeyboardLock = QDeviceInfo::TouchOrKeyboardLock,
        UnknownLock = QDeviceInfo::UnknownLock
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

    LockTypeFlags activatedLocks() const;
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
    void monitorThermalStateChanged();

    void activatedLocksChanged();
    void enabledLocksChanged();
    void thermalStateChanged();

private:
    QDeviceInfo *deviceInfo;

    bool isMonitorThermalState;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEDEVICEINFO_P_H
