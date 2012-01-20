/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qinputdeviceinfo.h>

#if defined(Q_OS_LINUX)
#  include "qinputdeviceinfo_linux_p.h"
#elif defined(Q_OS_WIN)
#  include "qinputdeviceinfo_win_p.h"
#else
QT_BEGIN_NAMESPACE
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
QT_END_NAMESPACE
#endif

QT_BEGIN_NAMESPACE

/*!
    \class QInputDeviceInfo
    \inmodule QtSystemInfo
    \brief The QInputDeviceInfo class provides various information about the input devices.
    \ingroup systeminfo
*/

/*!
    \enum QInputDeviceInfo::InputDeviceType
    This enum describes the type of input devices.

    \value UnknownInputDevice  The input device is unknown or no input device available.
    \value Keys                The device has keypads or keyboard.
    \value Touch               The device has touch pads or touch screens.
    \value Mouse               The device has mouses.
*/

/*!
    \enum QInputDeviceInfo::KeyboardType
    This enum describes the type of keyboards.

    \value UnknownKeyboard     The keyboard type is unknown or no keyboard available.
    \value SoftwareKeyboard    Software or virtual Keyboard.
    \value ITUKeypad           Standard phone keyboard.
    \value HalfQwertyKeyboard  Half qwerty keyboard like on Nokia E55.
    \value FullQwertyKeyboard  Standard qwerty type keyboard.
    \value WirelessKeyboard    Bluetooth or other wireless keyboard.
    \value FlipableKeyboard    The keyboard can be flipped out.
*/

/*!
    \enum QInputDeviceInfo::TouchDeviceType
    This enum describes the type of touch devices.

    \value UnknownTouchDevice  The touch device is unknown or no touch device available.
    \value SingleTouch         The device has single-point touch devices.
    \value MultiTouch          The device has multi-point touch devices.
*/

/*!
    \fn void QInputDeviceInfo::wirelessKeyboardConnected(bool connected)

    This signal is emitted whenever a wireless keyboard is \a connected, or otherwise disconnected.
*/

/*!
    \fn void QInputDeviceInfo::keyboardFlipped(bool open)

    This signal is emitted whenever the keyboard is flipped \a open, or otherwise close.
*/

/*!
    Constructs a QInputDeviceInfo object with the given \a parent.
*/
QInputDeviceInfo::QInputDeviceInfo(QObject *parent)
    : QObject(parent)
    , d_ptr(new QInputDeviceInfoPrivate(this))
{
}

/*!
    Destroys the object
*/
QInputDeviceInfo::~QInputDeviceInfo()
{
#if !defined(Q_OS_LINUX)
    delete d_ptr;
#endif
}

/*!
    Returns if the keyboard is currently flipped out.
*/
bool QInputDeviceInfo::isKeyboardFlippedOpen() const
{
    return d_ptr->isKeyboardFlippedOpen();
}

/*!
    Returns if the keyboard is light-on.
*/
bool QInputDeviceInfo::isKeyboardLightOn() const
{
    return d_ptr->isKeyboardLightOn();
}

/*!
    Returns if any wireless keyboard is currently connected.
*/
bool QInputDeviceInfo::isWirelessKeyboardConnected() const
{
    return d_ptr->isWirelessKeyboardConnected();
}

/*!
    Returns the available input devices on this device.
*/
QInputDeviceInfo::InputDeviceTypes QInputDeviceInfo::availableInputDevices() const
{
    return d_ptr->availableInputDevices();
}

/*!
    Returns the available keyboards on this device.
*/
QInputDeviceInfo::KeyboardTypes QInputDeviceInfo::availableKeyboards() const
{
    return d_ptr->availableKeyboards();
}

/*!
    Returns the available touch devices on this device.
*/
QInputDeviceInfo::TouchDeviceTypes QInputDeviceInfo::availableTouchDevices() const
{
    return d_ptr->availableTouchDevices();
}

/*!
    \internal
*/
void QInputDeviceInfo::connectNotify(const char *signal)
{
#if defined(Q_OS_LINUX)
    connect(d_ptr, signal, this, signal, Qt::UniqueConnection);
#else
    Q_UNUSED(signal)
#endif
}

/*!
    \internal
*/
void QInputDeviceInfo::disconnectNotify(const char *signal)
{
#if defined(Q_OS_LINUX)
    // We can only disconnect with the private implementation, when there is no receivers for the signal.
    if (receivers(signal) > 0)
        return;

    disconnect(d_ptr, signal, this, signal);
#else
    Q_UNUSED(signal)
#endif
}

QT_END_NAMESPACE
