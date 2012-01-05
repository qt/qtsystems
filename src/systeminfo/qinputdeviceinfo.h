/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef QINPUTDEVICEINFO_H
#define QINPUTDEVICEINFO_H

#include <qsysteminfoglobal.h>
#include <QtCore/qobject.h>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QInputDeviceInfoPrivate;

class Q_SYSTEMINFO_EXPORT QInputDeviceInfo : public QObject
{
    Q_OBJECT

    Q_FLAGS(InputDeviceType InputDeviceTypes)
    Q_FLAGS(KeyboardType KeyboardTypes)
    Q_FLAGS(TouchDeviceType TouchDeviceTypes)

public:
    enum InputDeviceType {
        UnknownInputDevice = 0x0,
        Keys = 0x1,
        Touch = 0x2,
        Mouse = 0x4
    };
    Q_DECLARE_FLAGS(InputDeviceTypes, InputDeviceType)

    enum KeyboardType {
        UnknownKeyboard = 0x0,
        SoftwareKeyboard = 0x1,
        ITUKeypad = 0x2,
        HalfQwertyKeyboard = 0x4,
        FullQwertyKeyboard = 0x8,
        WirelessKeyboard = 0x10,
        FlipableKeyboard = 0x20
    };
    Q_DECLARE_FLAGS(KeyboardTypes, KeyboardType)

    enum TouchDeviceType {
        UnknownTouchDevice = 0x0,
        SingleTouch = 0x1,
        MultiTouch = 0x2
    };
    Q_DECLARE_FLAGS(TouchDeviceTypes, TouchDeviceType)

    QInputDeviceInfo(QObject *parent = 0);
    virtual ~QInputDeviceInfo();

    Q_INVOKABLE bool isKeyboardFlippedOpen() const;
    Q_INVOKABLE bool isKeyboardLightOn() const;
    Q_INVOKABLE bool isWirelessKeyboardConnected() const;
    Q_INVOKABLE QInputDeviceInfo::InputDeviceTypes availableInputDevices() const;
    Q_INVOKABLE QInputDeviceInfo::KeyboardTypes availableKeyboards() const;
    Q_INVOKABLE QInputDeviceInfo::TouchDeviceTypes availableTouchDevices() const;

Q_SIGNALS:
    void keyboardFlipped(bool open);
    void wirelessKeyboardConnected(bool connected);

protected:
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);

private:
    Q_DISABLE_COPY(QInputDeviceInfo)
    QInputDeviceInfoPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(QInputDeviceInfo)
};

QT_END_NAMESPACE
QT_END_HEADER

#endif // QINPUTDEVICEINFO_H
