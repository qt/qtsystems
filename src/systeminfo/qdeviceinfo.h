/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDEVICEINFO_H
#define QDEVICEINFO_H

#include "qsysteminfo_p.h"
#include <QtCore/qobject.h>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QDeviceInfoPrivate;

class Q_SYSTEMINFO_EXPORT QDeviceInfo : public QObject
{
    Q_OBJECT

    Q_ENUMS(Feature)
    Q_ENUMS(LockType)
    Q_ENUMS(ThermalState)
    Q_ENUMS(Version)

    Q_FLAGS(LockType LockTypeFlags)

    Q_PROPERTY(LockTypeFlags activatedLocks READ activatedLocks NOTIFY activatedLocksChanged)
    Q_PROPERTY(LockTypeFlags enabledLocks READ enabledLocks NOTIFY enabledLocksChanged)
    Q_PROPERTY(ThermalState thermalState READ thermalState NOTIFY thermalStateChanged)

public:
    enum Feature {
        Bluetooth = 0,
        Camera,
        FmRadio,
        FmTransmitter,
        Infrared,
        Led,
        MemoryCard,
        Usb,
        Vibration,
        Wlan,
        Sim,
        Positioning,
        VideoOut,
        Haptics,
        Nfc
    };

    enum LockType {
        NoLock = 0,
        PinLock = 0x0000001,
        TouchOrKeyboardLock = 0x0000002
    };
    Q_DECLARE_FLAGS(LockTypeFlags, LockType)

    enum ThermalState {
        UnknownThermal = 0,
        NormalThermal,
        WarningThermal,
        AlertThermal,
        ErrorThermal
    };

    enum Version {
        Os = 0,
        Firmware
    };

    QDeviceInfo(QObject *parent = 0);
    virtual ~QDeviceInfo();

    QDeviceInfo::LockTypeFlags activatedLocks() const;
    QDeviceInfo::LockTypeFlags enabledLocks() const;
    QDeviceInfo::ThermalState thermalState() const;

    Q_INVOKABLE bool hasFeature(QDeviceInfo::Feature feature) const;
    Q_INVOKABLE QByteArray uniqueDeviceID() const;
    Q_INVOKABLE QString imei(int interface) const;
    Q_INVOKABLE QString manufacturer() const;
    Q_INVOKABLE QString model() const;
    Q_INVOKABLE QString productName() const;
    Q_INVOKABLE QString version(QDeviceInfo::Version type) const;

Q_SIGNALS:
    void activatedLocksChanged(QDeviceInfo::LockTypeFlags types);
    void enabledLocksChanged(QDeviceInfo::LockTypeFlags types);
    void thermalStateChanged(QDeviceInfo::ThermalState state);

protected:
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);

private:
    Q_DISABLE_COPY(QDeviceInfo)
    QDeviceInfoPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(QDeviceInfo)
};

QT_END_NAMESPACE
QT_END_HEADER

#endif // QDEVICEINFO_H
