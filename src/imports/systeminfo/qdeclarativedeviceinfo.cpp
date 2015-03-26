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

#include "qdeclarativedeviceinfo_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype DeviceInfo
    \instantiates QDeclarativeDeviceInfo
    \inqmlmodule QtSystemInfo
    \ingroup qml-systeminfo
    \brief The DeviceInfo element provides various information about the device.
*/

/*!
    \internal
*/
QDeclarativeDeviceInfo::QDeclarativeDeviceInfo(QObject *parent)
    : QObject(parent)
    , deviceInfo(new QDeviceInfo(this))
    , isMonitorThermalState(false)
{
}

/*!
    \internal
 */
QDeclarativeDeviceInfo::~QDeclarativeDeviceInfo()
{
}

/*!
    \qmlproperty flag DeviceInfo::activatedLocks

    This property holds the activated locks. Available locks include:
    \list
    \li DeviceInfo.NoLock               - No lock.
    \li DeviceInfo.PinLock              - Device can be locked by PIN code or password.
    \li DeviceInfo.TouchOrKeyboardLock  - Device can be locked by touch or keyboard.
    \li DeviceInfo.UnknownLock          - Lock types requested but no result received yet.
    \endlist
*/
QDeclarativeDeviceInfo::LockTypeFlags QDeclarativeDeviceInfo::activatedLocks() const
{
    connect(deviceInfo, SIGNAL(activatedLocksChanged(QDeviceInfo::LockTypeFlags)),
            this, SIGNAL(activatedLocksChanged()), Qt::UniqueConnection);

    QDeviceInfo::LockTypeFlags locks(deviceInfo->activatedLocks());
    if (locks.testFlag(QDeviceInfo::UnknownLock))
       return QDeclarativeDeviceInfo::UnknownLock;
    LockTypeFlags declarativeLocks(NoLock);
    if (locks.testFlag(QDeviceInfo::PinLock))
        declarativeLocks |= PinLock;
    if (locks.testFlag(QDeviceInfo::TouchOrKeyboardLock))
        declarativeLocks |= TouchOrKeyboardLock;
    return declarativeLocks;
}

/*!
    \qmlproperty flag DeviceInfo::enabledLocks

    This property holds the enabled locks. Available locks include:
    \list
    \li DeviceInfo.NoLock               - No lock.
    \li DeviceInfo.PinLock              - Device can be locked by PIN code or password.
    \li DeviceInfo.TouchOrKeyboardLock  - Device can be locked by touch or keyboard.
    \li DeviceInfo.UnknownLock          - Lock types requested but no result received yet.
    \endlist
*/
QDeclarativeDeviceInfo::LockTypeFlags QDeclarativeDeviceInfo::enabledLocks() const
{
    connect(deviceInfo, SIGNAL(enabledLocksChanged(QDeviceInfo::LockTypeFlags)),
            this, SIGNAL(enabledLocksChanged()), Qt::UniqueConnection);

    QDeviceInfo::LockTypeFlags locks(deviceInfo->enabledLocks());
    if (locks.testFlag(QDeviceInfo::UnknownLock))
       return QDeclarativeDeviceInfo::UnknownLock;
    LockTypeFlags declarativeLocks(NoLock);
    if (locks.testFlag(QDeviceInfo::PinLock))
        declarativeLocks |= PinLock;
    if (locks.testFlag(QDeviceInfo::TouchOrKeyboardLock))
        declarativeLocks |= TouchOrKeyboardLock;
    return declarativeLocks;
}

/*!
    \qmlproperty bool DeviceInfo::monitorThermalState

    This property holds whether or not monitor the change of thermal state.
 */
bool QDeclarativeDeviceInfo::monitorThermalState() const
{
    return isMonitorThermalState;
}

void QDeclarativeDeviceInfo::setMonitorThermalState(bool monitor)
{
    if (monitor != isMonitorThermalState) {
        isMonitorThermalState = monitor;
        if (monitor) {
            connect(deviceInfo, SIGNAL(thermalStateChanged(QDeviceInfo::ThermalState)),
                    this, SIGNAL(thermalStateChanged()));
        } else {
            disconnect(deviceInfo, SIGNAL(thermalStateChanged(QDeviceInfo::ThermalState)),
                       this, SIGNAL(thermalStateChanged()));
        }
        emit monitorThermalStateChanged();
    }
}

/*!
    \qmlproperty enumeration DeviceInfo::thermalState

    This property holds the thermal state. Possible values are:
    \list
    \li DeviceInfo.UnknownThermal   - The thermal state is unknown.
    \li DeviceInfo.NormalThermal    - The thermal state is normal.
    \li DeviceInfo.WarningThermal   - The thermal state is warning.
    \li DeviceInfo.AlertThermal     - The thermal state is alert.
    \li DeviceInfo.ErrorThermal     - The thermal state is error.
    \endlist
*/
QDeclarativeDeviceInfo::ThermalState QDeclarativeDeviceInfo::thermalState() const
{
    return static_cast<ThermalState>(deviceInfo->thermalState());
}

/*!
    \qmlmethod bool DeviceInfo::hasFeature(Feature feature)

    Returns true if the \a feature is supported, otherwise false. The following features can be
    queried:
    \list
    \li DeviceInfo.BluetoothFeature      - Bluetooth feature.
    \li DeviceInfo.CameraFeature         - Camera feature.
    \li DeviceInfo.FmRadioFeature        - Frequency modulation (FM) radio feature.
    \li DeviceInfo.FmTransmitterFeature  - Frequency modulation (FM) radio transmitter feature.
    \li DeviceInfo.InfraredFeature       - Infrared communication feature.
    \li DeviceInfo.LedFeature            - Light-emitting diode (LED) feature.
    \li DeviceInfo.MemoryCardFeature     - Memory card feature.
    \li DeviceInfo.UsbFeature            - Universal system bus (USB) feature.
    \li DeviceInfo.VibrationFeature      - Vibration feature.
    \li DeviceInfo.WlanFeature           - Wireless local area network (WLAN) feature.
    \li DeviceInfo.SimFeature            - Subscriber identity module (SIM) feature.
    \li DeviceInfo.PositioningFeature    - Positioning feature, e.g. Global Positioning System (GPS).
    \li DeviceInfo.VideoOutFeature       - Video out feature.
    \li DeviceInfo.HapticsFeature        - Haptics feature, the platform can provide audio and/or visual and/or vibration feedback.
    \li DeviceInfo.NfcFeature            - Near Field Communication (NFC) feature
    \endlist
*/
bool QDeclarativeDeviceInfo::hasFeature(QDeclarativeDeviceInfo::Feature feature) const
{
    return deviceInfo->hasFeature(static_cast<QDeviceInfo::Feature>(feature));
}

/*!
    \qmlmethod int DeviceInfo::imeiCount()

    Returns the count of available International Mobile Equipment Identity (IMEI) of the device. In
    case of error, or the information is not available, -1 is returned.
*/
int QDeclarativeDeviceInfo::imeiCount() const
{
    return deviceInfo->imeiCount();
}

/*!
    \qmlmethod string DeviceInfo::imei(int interface)

    Returns the International Mobile Equipment Identity (IMEI) of the given \a interface on the device.
    In case of error, or the information is not available, an empty string is returned.
*/
QString QDeclarativeDeviceInfo::imei(int interface) const
{
    return deviceInfo->imei(interface);
}

/*!
    \qmlmethod string DeviceInfo::manufacturer()

    Returns the name of the manufacturer of this device, or the name of the vendor of the motherboard
    as a fallback. In case of error, or the information is not available, an empty string is returned.
*/
QString QDeclarativeDeviceInfo::manufacturer() const
{
    return deviceInfo->manufacturer();
}

/*!
    \qmlmethod string DeviceInfo::model()

    Returns the model information of the device, e.g. N8, or the CPU architect as a fallback. In case
    of error, or the information is not available, an empty string is returned.
*/
QString QDeclarativeDeviceInfo::model() const
{
    return deviceInfo->model();
}

/*!
    \qmlmethod string DeviceInfo::productName()

    Returns the internal product name of the device, e.g. RM-774. In case of error, or the information
    is not available, an empty string is returned.

    For Linux, it returns the codename of the distribution if any.
*/
QString QDeclarativeDeviceInfo::productName() const
{
    return deviceInfo->productName();
}

/*!
    \qmlmethod string DeviceInfo::uniqueDeviceID()

    Returns a unique identifier for the device, or an empty string if on error or not available.
*/
QString QDeclarativeDeviceInfo::uniqueDeviceID() const
{
    return deviceInfo->uniqueDeviceID();
}

/*!
    \qmlmethod string DeviceInfo::version(Version type)

    Returns the version of \a type. In case of error, or the version is unknown, an empty string
    is returned. The following versions can be queried:
    \list
    \li DeviceInfo.Os         - Operating system version. For Linux, it returns the version of the
                                distribution if any.
    \li DeviceInfo.Firmware   - Version of (flashable) system as a whole. For Linux, it's the version
                                of the kernel.
    \endlist
*/
QString QDeclarativeDeviceInfo::version(QDeclarativeDeviceInfo::Version type) const
{
    return deviceInfo->version(static_cast<QDeviceInfo::Version>(type));
}

QT_END_NAMESPACE
