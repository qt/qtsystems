/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystemKit module of the Qt Toolkit.
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

#include "qdeviceinfo.h"

#if defined(Q_OS_LINUX)
#  include "qdeviceinfo_linux_p.h"
#else
QT_BEGIN_NAMESPACE
class QDeviceInfoPrivate
{
public:
    QDeviceInfoPrivate(QDeviceInfo *) {}

    bool hasFeature(QDeviceInfo::Feature) { return false; }
    QDeviceInfo::LockTypeFlags activatedLocks() { return QDeviceInfo::NoLock; }
    QDeviceInfo::LockTypeFlags enabledLocks() { return QDeviceInfo::NoLock; }
    QDeviceInfo::ThermalState thermalState() { return QDeviceInfo::UnknownThermal; }
    QByteArray uniqueDeviceID() { return QByteArray(); }
    QString imei() { return QString(); }
    QString manufacturer() { return QString(); }
    QString model() { return QString(); }
    QString productName() { return QString(); }
    QString version(QDeviceInfo::Version) { return QString(); }
};
QT_END_NAMESPACE
#endif

QT_BEGIN_NAMESPACE

/*!
    \class QDeviceInfo
    \inmodule QtSystemKit
    \brief The QDeviceInfo class provides various information of the system.
*/

/*!
    \enum QDeviceInfo::Feature
    This enum describes the features of the device.

    \value Bluetooth      Bluetooth feature.
    \value Camera         Camera feature.
    \value FmRadio        Frequency modulation (FM) radio feature.
    \value FmTransmitter  Frequency modulation (FM) radio transmitter feature.
    \value Infrared       Infrared communication feature.
    \value Led            Light-emitting diode (LED) feature.
    \value MemoryCard     Memory card feature.
    \value Usb            Universal system bus (USB) feature.
    \value Vibration      Vibration feature.
    \value Wlan           Wireless local area network (WLAN) feature.
    \value Sim            Subscriber identity module (SIM) feature.
    \value Positioning    Positioning feature, e.g. Global Positioning System (GPS).
    \value VideoOut       Video out feature available.
    \value Haptics        Haptics feature available.
*/

/*!
    \enum QDeviceInfo::LockType
    This enum describes lock type on the device.

    \value NoLock               No lock, or unknown lock type.
    \value PinLock              Device can be locked by PIN code or password.
    \value TouchOrKeyboardLock  Device can be locked by touch or keyboard.
*/

/*!
    \enum QDeviceInfo::ThermalState
    This enum describes the thermal state:

    \value UnknownThermal   The thermal state is unKnown.
    \value NormalThermal    The thermal state is normal.
    \value WarningThermal   The thermal state is warning.
    \value AlertThermal     The thermal state is alert.
    \value ErrorThermal     The thermal state is error.
*/

/*!
    \enum QDeviceInfo::Version
    This enum describes the version component.

    \value Os                    Operating system version / platform ID.
    \value QtCore                Qt library version.
    \value Firmware              Version of (flashable) system as a whole.
    \value QtMobility            QtMobility library version. Since 1.1
*/

/*!
    \fn void QDeviceInfo::activatedLocksChanged(QDeviceInfo::LockTypeFlags types)

    This signal is emitted when the activated locks have changed to \a types.
*/

/*!
    \fn void QDeviceInfo::enabledLocksChanged(QDeviceInfo::LockTypeFlags types)

    This signal is emitted when the enabled locks have changed to \a types.
*/

/*!
    \fn void QDeviceInfo::thermalStateChanged(QDeviceInfo::ThermalState state)

    This signal is emitted when the thermal state has changed to \a state.
*/

/*!
    Constructs a QDeviceInfo object with the given \a parent.
*/
QDeviceInfo::QDeviceInfo(QObject *parent)
    : QObject(parent)
    , d_ptr(new QDeviceInfoPrivate(this))
{
}

/*!
    Destroys the object
*/
QDeviceInfo::~QDeviceInfo()
{
    delete d_ptr;
}

/*!
    \property QDeviceInfo::activatedLocks
    \brief The activated lock types.

    The current activated lock types of the device. It suggests that these lock types are also
    enabled.
*/
QDeviceInfo::LockTypeFlags QDeviceInfo::activatedLocks() const
{
    return d_ptr->activatedLocks();
}

/*!
    \property QDeviceInfo::enabledLocks
    \brief The enabled lock types.

    The current lock types that are enabled on the device. Note that it doesn't mean the device is
    currently locked.
*/
QDeviceInfo::LockTypeFlags QDeviceInfo::enabledLocks() const
{
    return d_ptr->enabledLocks();
}

/*!
    \property QDeviceInfo::thermalState
    \brief The thermal state.

    The current thermal state of the device.
*/
QDeviceInfo::ThermalState QDeviceInfo::thermalState() const
{
    return d_ptr->thermalState();
}

/*!
    Returns true if the \a feature is supported, otherwise false.
*/
bool QDeviceInfo::hasFeature(QDeviceInfo::Feature feature) const
{
    return d_ptr->hasFeature(feature);
}

/*!
    Returns a unique identifier for the device, or an empty byte array if on error or not available.
*/
QByteArray QDeviceInfo::uniqueDeviceID() const
{
    return d_ptr->uniqueDeviceID();
}

/*!
    Returns the International Mobile Equipment Identity (IMEI) of the device. In case of error, or
    the information is not available, an empty string is returned.
*/
QString QDeviceInfo::imei() const
{
    return d_ptr->imei();
}

/*!
    Returns the name of the manufacturer of this device, or the name of the vendor of the motherboard
    as a fallback. In case of error, or the information is not available, an empty string is returned.
*/
QString QDeviceInfo::manufacturer() const
{
    return d_ptr->manufacturer();
}

/*!
    Returns the model information of the device, or the CPU architect as a fallback. In case of
    error, or the information is not available, an empty string is returned.
*/
QString QDeviceInfo::model() const
{
    return d_ptr->model();
}

/*!
    Returns the product name of the device. In case of error, or the information is not available,
    an empty string is returned.
*/
QString QDeviceInfo::productName() const
{
    return d_ptr->productName();
}

/*!
    Returns the version of \a type. In case of error, or the version is unknown, an empty string
    is returned.
*/
QString QDeviceInfo::version(QDeviceInfo::Version type) const
{
    return d_ptr->version(type);
}

QT_END_NAMESPACE
