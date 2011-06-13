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

#include "qinputdeviceinfo_linux_p.h"

#include <QtCore/qdir.h>

QT_BEGIN_NAMESPACE

static const QString INPUT_SYSFS_PATH(QString::fromAscii("/sys/class/input/"));

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
    const QStringList dirs = QDir(INPUT_SYSFS_PATH).entryList(QStringList() << QString::fromAscii("input*"));
    if (dirs.isEmpty())
        return QInputDeviceInfo::UnknownInputDevice;

    QInputDeviceInfo::InputDeviceTypes types(QInputDeviceInfo::UnknownInputDevice);
    foreach (const QString &dir, dirs) {
        QFile deviceName(INPUT_SYSFS_PATH + dir + QString::fromAscii("/name"));
        if (deviceName.open(QIODevice::ReadOnly)) {
            QString value(QString::fromAscii(deviceName.readAll().data()));
            if (value.contains(QString::fromAscii("keyboard"), Qt::CaseInsensitive)
                || value.contains(QString::fromAscii("keypad"), Qt::CaseInsensitive)) {
                types |= QInputDeviceInfo::Keys;
            } else if (value.contains(QString::fromAscii("mouse"), Qt::CaseInsensitive)) {
                types |= QInputDeviceInfo::Mouse;
            } else if (value.contains(QString::fromAscii("touch"), Qt::CaseInsensitive)) {
                types |= QInputDeviceInfo::Touch;
            }
        }
    }
    return types;
}

QInputDeviceInfo::KeyboardTypes QInputDeviceInfoPrivate::availableKeyboards()
{
    // TODO wireless key board

    const QStringList dirs = QDir(INPUT_SYSFS_PATH).entryList(QStringList() << QString::fromAscii("input*"));
    if (dirs.isEmpty())
        return QInputDeviceInfo::UnknownKeyboard;

    QInputDeviceInfo::KeyboardTypes types(QInputDeviceInfo::UnknownKeyboard);
    foreach (const QString &dir, dirs) {
        QFile deviceName(INPUT_SYSFS_PATH + dir + QString::fromAscii("/name"));
        if (deviceName.open(QIODevice::ReadOnly)) {
            QString value(QString::fromAscii(deviceName.readAll().data()));
            if (value.contains(QString::fromAscii("keyboard"), Qt::CaseInsensitive))
                types |= QInputDeviceInfo::FullQwertyKeyboard;
            else if (value.contains(QString::fromAscii("keypad"), Qt::CaseInsensitive))
                types |= QInputDeviceInfo::ITUKeypad;
        }
    }
    return types;
}

QInputDeviceInfo::TouchDeviceTypes QInputDeviceInfoPrivate::availableTouchDevices()
{
    const QStringList dirs = QDir(INPUT_SYSFS_PATH).entryList(QStringList() << QString::fromAscii("input*"));
    if (dirs.isEmpty())
        return QInputDeviceInfo::UnknownTouchDevice;

    QInputDeviceInfo::TouchDeviceTypes types(QInputDeviceInfo::UnknownTouchDevice);
    foreach (const QString &dir, dirs) {
        QFile deviceName(INPUT_SYSFS_PATH + dir + QString::fromAscii("/name"));
        if (deviceName.open(QIODevice::ReadOnly)) {
            QString value(QString::fromAscii(deviceName.readAll().data()));
            if (value.contains(QString::fromAscii("multi touch"), Qt::CaseInsensitive))
                types |= QInputDeviceInfo::MultiTouch;
            else if (value.contains(QString::fromAscii("touch"), Qt::CaseInsensitive))
                types |= QInputDeviceInfo::SingleTouch;
        }
    }
    return types;
}

QT_END_NAMESPACE
