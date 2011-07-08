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

#include "qinputdeviceinfo_linux_p.h"

#if !defined(QT_NO_BLUEZ)
#include "qbluezwrapper_p.h"
#endif // QT_NO_BLUEZ

#include <QtCore/qdir.h>

QT_BEGIN_NAMESPACE

static const QString INPUT_SYSFS_PATH(QString::fromAscii("/sys/class/input/"));

QInputDeviceInfoPrivate::QInputDeviceInfoPrivate(QInputDeviceInfo *parent)
    : QObject(parent)
    , q_ptr(parent)
    , watchWirelessKeyboard(false)
#if !defined(QT_NO_BLUEZ)
    , bluezWrapper(0)
#endif // QT_NO_BLUEZ
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
    if (watchWirelessKeyboard)
        return wirelessKeyboardConnectedBuffer;

#if !defined(QT_NO_BLUEZ)
    if (QBluezWrapper::isBluezAvailable()) {
        if (!bluezWrapper)
            bluezWrapper = new QBluezWrapper(this);
        return bluezWrapper->hasInputDevice();
    }
#endif

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

    if (isWirelessKeyboardConnected())
        types |= QInputDeviceInfo::WirelessKeyboard;

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

void QInputDeviceInfoPrivate::connectNotify(const char *signal)
{
#if !defined(QT_NO_BLUEZ)
    if (QBluezWrapper::isBluezAvailable()) {
        if (strcmp(signal, SIGNAL(wirelessKeyboardConnected(bool))) == 0) {
            if (!bluezWrapper)
                bluezWrapper = new QBluezWrapper(this);
            connect(bluezWrapper, signal, this, signal, Qt::UniqueConnection);
            wirelessKeyboardConnectedBuffer = isWirelessKeyboardConnected();
            watchWirelessKeyboard = true;
        }
    }
#else
    Q_UNUSED(signal)
#endif
}

void QInputDeviceInfoPrivate::disconnectNotify(const char *signal)
{
#if !defined(QT_NO_BLUEZ)
    if (!QBluezWrapper::isBluezAvailable() || !bluezWrapper)
        return;

    if (strcmp(signal, SIGNAL(wirelessKeyboardConnected(bool))) == 0) {
        disconnect(bluezWrapper, signal, this, signal);
        watchWirelessKeyboard = false;
    }
#else
    Q_UNUSED(signal)
#endif
}

QT_END_NAMESPACE
