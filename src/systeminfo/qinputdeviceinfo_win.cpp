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

#include "qinputdeviceinfo_win_p.h"

#include <QtCore/qdir.h>

#include <windows.h>

QT_BEGIN_NAMESPACE

QInputDeviceInfoPrivate::QInputDeviceInfoPrivate(QInputDeviceInfo *parent)
    : q_ptr(parent)
{
}

bool QInputDeviceInfoPrivate::isKeyboardFlippedOpen()
{
    return false;
}

bool QInputDeviceInfoPrivate::isKeyboardLightOn()
{
    return false;
}

bool QInputDeviceInfoPrivate::isWirelessKeyboardConnected()
{
    return false;
}

QInputDeviceInfo::InputDeviceTypes QInputDeviceInfoPrivate::availableInputDevices()
{
    QInputDeviceInfo::InputDeviceTypes types(QInputDeviceInfo::UnknownInputDevice);

    if (GetKeyboardType(0) > 0)
        types |= QInputDeviceInfo::Keys;

    if (GetSystemMetrics(SM_TABLETPC) > 0)
        types |= QInputDeviceInfo::Touch;

    if (GetSystemMetrics(SM_CMOUSEBUTTONS) > 0)
        types |= QInputDeviceInfo::Mouse;

    return types;
}

QInputDeviceInfo::KeyboardTypes QInputDeviceInfoPrivate::availableKeyboards()
{
    QInputDeviceInfo::KeyboardTypes types(QInputDeviceInfo::UnknownKeyboard);

    int keyboardType = GetKeyboardType(0);
    switch(keyboardType) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 7:
        types |= QInputDeviceInfo::FullQwertyKeyboard;
        break;

    case 5: // Nokia 1050 and similar keyboards
    case 6: // Nokia 9140 and similar keyboards
        // I don't know what're these :(
        break;

    default:
        break;
    };

    return types;
}

QInputDeviceInfo::TouchDeviceTypes QInputDeviceInfoPrivate::availableTouchDevices()
{
    QInputDeviceInfo::TouchDeviceTypes types(QInputDeviceInfo::UnknownTouchDevice);

#if defined(SM_DIGITIZER)
    int touchDevices = GetSystemMetrics(SM_DIGITIZER);
    if (SM_DIGITIZER & NID_MULTI_INPUT)
        types |= QInputDeviceInfo::MultiTouch;
    else if (SM_DIGITIZER > 0)
        types |= QInputDeviceInfo::SingleTouch;
#else
    if (GetSystemMetrics(SM_TABLETPC) > 0)
        types |= QInputDeviceInfo::SingleTouch;
#endif

    return types;
}

QT_END_NAMESPACE