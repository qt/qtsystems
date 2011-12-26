/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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

#include "qdeclarativeinputdeviceinfo_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmlclass InputDeviceInfo QDeclarativeInputDeviceInfo
    \inmodule QtSystemInfo
    \ingroup qml-systeminfo
    \brief The InputDeviceInfo element provides various information of the input devices.
*/

/*!
    \internal
*/
QDeclarativeInputDeviceInfo::QDeclarativeInputDeviceInfo(QObject *parent)
    : QObject(parent)
    , inputDeviceInfo(new QInputDeviceInfo(this))
    , isMonitorKeyboardFlipped(false)
    , isMonitorWirelessKeyboardConnected(false)
{
}

/*!
    \internal
 */
QDeclarativeInputDeviceInfo::~QDeclarativeInputDeviceInfo()
{
}

/*!
    \qmlproperty bool InputDeviceInfo::monitorKeyboardFlipped

    This property triggers the active monitoring if the keyboard is flipped.
 */
bool QDeclarativeInputDeviceInfo::monitorKeyboardFlipped() const
{
    return isMonitorKeyboardFlipped;
}

void QDeclarativeInputDeviceInfo::setMonitorKeyboardFlipped(bool monitor)
{
    if (monitor != isMonitorKeyboardFlipped) {
        isMonitorKeyboardFlipped = monitor;
        if (monitor) {
            connect(inputDeviceInfo, SIGNAL(keyboardFlipped(bool)),
                    this, SIGNAL(isKeyboardFlippedOpenChanged()));
        } else {
            disconnect(inputDeviceInfo, SIGNAL(keyboardFlipped(bool)),
                       this, SIGNAL(isKeyboardFlippedOpenChanged()));
        }
        emit monitorKeyboardFlippedChanged();
    }
}

/*!
    \qmlproperty bool InputDeviceInfo::isKeyboardFlippedOpen

    This property holds if the keybord is currently flipped out.
*/
bool QDeclarativeInputDeviceInfo::isKeyboardFlippedOpen() const
{
    return inputDeviceInfo->isKeyboardFlippedOpen();
}

/*!
    \qmlproperty bool InputDeviceInfo::monitorWirelessKeyboardConnected

    This property triggers the active monitoring if any wireless keyboard is connected.
 */
bool QDeclarativeInputDeviceInfo::monitorWirelessKeyboardConnected() const
{
    return isMonitorWirelessKeyboardConnected;
}

void QDeclarativeInputDeviceInfo::setMonitorWirelessKeyboardConnected(bool monitor)
{
    if (monitor != isMonitorWirelessKeyboardConnected) {
        isMonitorWirelessKeyboardConnected = monitor;
        if (monitor) {
            connect(inputDeviceInfo, SIGNAL(wirelessKeyboardConnected(bool)),
                    this, SIGNAL(isWirelessKeyboardConnectedChanged()));
        } else {
            disconnect(inputDeviceInfo, SIGNAL(wirelessKeyboardConnected(bool)),
                       this, SIGNAL(isWirelessKeyboardConnectedChanged()));
        }
        emit monitorWirelessKeyboardConnectedChanged();
    }
}

/*!
    \qmlproperty bool InputDeviceInfo::isWirelessKeyboardConnected

    This property holds if any wireless keyboard is connected.
*/
bool QDeclarativeInputDeviceInfo::isWirelessKeyboardConnected() const
{
    return inputDeviceInfo->isWirelessKeyboardConnected();
}

/*!
    \qmlmethod bool InputDeviceInfo::isKeyboardLightOn()

    Returns if the keyboard is light-on.
*/
bool QDeclarativeInputDeviceInfo::isKeyboardLightOn() const
{
    return inputDeviceInfo->isKeyboardLightOn();
}

/*!
    \qmlmethod flag InputDeviceInfo::availableInputDevices()

    Returns the available input devices on this device. Possible values are:
    \list
    \o UnknownInputDevice  The input device is unknown or no input device available.
    \o Keys                The device has keypads or keyboard.
    \o Touch               The device has touch pads or touch screens.
    \o Mouse               The device has mouses.
    \endlist
*/
QDeclarativeInputDeviceInfo::InputDeviceTypes QDeclarativeInputDeviceInfo::availableInputDevices() const
{
    QInputDeviceInfo::InputDeviceTypes types(inputDeviceInfo->availableInputDevices());
    InputDeviceTypes declarativeTypes(UnknownInputDevice);
    if (types.testFlag(QInputDeviceInfo::Keys))
        declarativeTypes |= Keys;
    if (types.testFlag(QInputDeviceInfo::Touch))
        declarativeTypes |= Touch;
    if (types.testFlag(QInputDeviceInfo::Mouse))
        declarativeTypes |= Mouse;
    return declarativeTypes;
}

/*!
    \qmlmethod flag InputDeviceInfo::availableKeyboards()

    Returns the available keyboards on this device. Possible values are:
    \list
    \o UnknownKeyboard     The keyboard type is unknown or no keyboard available.
    \o SoftwareKeyboard    Software or virtual Keyboard.
    \o ITUKeypad           Standard phone keyboard.
    \o HalfQwertyKeyboard  Half qwerty keyboard like on Nokia E55.
    \o FullQwertyKeyboard  Standard qwerty type keyboard.
    \o WirelessKeyboard    Bluetooth or other wireless keyboard.
    \o FlipableKeyboard    The keyboard can be flipped out.
    \endlist
*/
QDeclarativeInputDeviceInfo::KeyboardTypes QDeclarativeInputDeviceInfo::availableKeyboards() const
{
    QInputDeviceInfo::KeyboardTypes types(inputDeviceInfo->availableKeyboards());
    KeyboardTypes declarativeTypes(UnknownKeyboard);
    if (types.testFlag(QInputDeviceInfo::SoftwareKeyboard))
        declarativeTypes |= SoftwareKeyboard;
    if (types.testFlag(QInputDeviceInfo::ITUKeypad))
        declarativeTypes |= ITUKeypad;
    if (types.testFlag(QInputDeviceInfo::HalfQwertyKeyboard))
        declarativeTypes |= HalfQwertyKeyboard;
    if (types.testFlag(QInputDeviceInfo::FullQwertyKeyboard))
        declarativeTypes |= FullQwertyKeyboard;
    if (types.testFlag(QInputDeviceInfo::WirelessKeyboard))
        declarativeTypes |= WirelessKeyboard;
    if (types.testFlag(QInputDeviceInfo::FlipableKeyboard))
        declarativeTypes |= FlipableKeyboard;
    return declarativeTypes;
}

/*!
    \qmlmethod flag InputDeviceInfo::availableTouchDevices()

    Returns the available touch devices on this device. Possible values are:
    \list
    \o UnknownTouchDevice  The touch device is unknown or no touch device available.
    \o SingleTouch         The device has single-point touch devices.
    \o MultiTouch          The device has multi-point touch devices.
    \endlist
*/
QDeclarativeInputDeviceInfo::TouchDeviceTypes QDeclarativeInputDeviceInfo::availableTouchDevices() const
{
    QInputDeviceInfo::TouchDeviceTypes types(inputDeviceInfo->availableTouchDevices());
    TouchDeviceTypes declarativeTypes(UnknownTouchDevice);
    if (types.testFlag(QInputDeviceInfo::SingleTouch))
        declarativeTypes |= SingleTouch;
    if (types.testFlag(QInputDeviceInfo::MultiTouch))
        declarativeTypes |= MultiTouch;
    return declarativeTypes;
}

QT_END_NAMESPACE
