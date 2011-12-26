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

#ifndef QDECLARATIVEINPUTDEVICEINFO_P_H
#define QDECLARATIVEINPUTDEVICEINFO_P_H

#include <qinputdeviceinfo.h>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QDeclarativeInputDeviceInfo : public QObject
{
    Q_OBJECT

    Q_FLAGS(InputDeviceType InputDeviceTypes)
    Q_FLAGS(KeyboardType KeyboardTypes)
    Q_FLAGS(TouchDeviceType TouchDeviceTypes)

    Q_PROPERTY(bool monitorKeyboardFlipped READ monitorKeyboardFlipped WRITE setMonitorKeyboardFlipped NOTIFY monitorKeyboardFlippedChanged)
    Q_PROPERTY(bool monitorWirelessKeyboardConnected READ monitorWirelessKeyboardConnected WRITE setMonitorWirelessKeyboardConnected NOTIFY monitorWirelessKeyboardConnectedChanged)

    Q_PROPERTY(bool isKeyboardFlippedOpen READ isKeyboardFlippedOpen NOTIFY isKeyboardFlippedOpenChanged)
    Q_PROPERTY(bool isWirelessKeyboardConnected READ isWirelessKeyboardConnected NOTIFY isWirelessKeyboardConnectedChanged)

public:
    enum InputDeviceType {
        UnknownInputDevice = QInputDeviceInfo::UnknownInputDevice,
        Keys = QInputDeviceInfo::Keys,
        Touch = QInputDeviceInfo::Touch,
        Mouse = QInputDeviceInfo::Mouse
    };
    Q_DECLARE_FLAGS(InputDeviceTypes, InputDeviceType)

    enum KeyboardType {
        UnknownKeyboard = QInputDeviceInfo::UnknownKeyboard,
        SoftwareKeyboard = QInputDeviceInfo::SoftwareKeyboard,
        ITUKeypad = QInputDeviceInfo::ITUKeypad,
        HalfQwertyKeyboard = QInputDeviceInfo::HalfQwertyKeyboard,
        FullQwertyKeyboard = QInputDeviceInfo::FullQwertyKeyboard,
        WirelessKeyboard = QInputDeviceInfo::WirelessKeyboard,
        FlipableKeyboard = QInputDeviceInfo::FlipableKeyboard
    };
    Q_DECLARE_FLAGS(KeyboardTypes, KeyboardType)

    enum TouchDeviceType {
        UnknownTouchDevice = QInputDeviceInfo::UnknownTouchDevice,
        SingleTouch = QInputDeviceInfo::SingleTouch,
        MultiTouch = QInputDeviceInfo::MultiTouch
    };
    Q_DECLARE_FLAGS(TouchDeviceTypes, TouchDeviceType)

    QDeclarativeInputDeviceInfo(QObject *parent = 0);
    virtual ~QDeclarativeInputDeviceInfo();

    bool monitorKeyboardFlipped() const;
    void setMonitorKeyboardFlipped(bool monitor);
    bool isKeyboardFlippedOpen() const;

    bool monitorWirelessKeyboardConnected() const;
    void setMonitorWirelessKeyboardConnected(bool monitor);
    bool isWirelessKeyboardConnected() const;

    Q_INVOKABLE bool isKeyboardLightOn() const;
    Q_INVOKABLE InputDeviceTypes availableInputDevices() const;
    Q_INVOKABLE KeyboardTypes availableKeyboards() const;
    Q_INVOKABLE TouchDeviceTypes availableTouchDevices() const;

Q_SIGNALS:
    void monitorKeyboardFlippedChanged();
    void monitorWirelessKeyboardConnectedChanged();

    void isKeyboardFlippedOpenChanged();
    void isWirelessKeyboardConnectedChanged();

private:
    QInputDeviceInfo *inputDeviceInfo;

    bool isMonitorKeyboardFlipped;
    bool isMonitorWirelessKeyboardConnected;
};

QT_END_NAMESPACE
QT_END_HEADER

#endif // QDECLARATIVEINPUTDEVICEINFO_P_H
