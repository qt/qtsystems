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

#include "qdeviceinfo_win_p.h"
#include "qscreensaver_win_p.h"

#include <QtCore/qsettings.h>

#include <windows.h>

QT_BEGIN_NAMESPACE

QDeviceInfoPrivate::QDeviceInfoPrivate(QDeviceInfo *parent)
    : q_ptr(parent)
{
}

bool QDeviceInfoPrivate::hasFeature(QDeviceInfo::Feature feature)
{
    switch (feature) {
    case QDeviceInfo::Bluetooth:
    case QDeviceInfo::Camera:
    case QDeviceInfo::FmRadio:
    case QDeviceInfo::FmTransmitter:
    case QDeviceInfo::Infrared:
    case QDeviceInfo::Led:
    case QDeviceInfo::MemoryCard:
    case QDeviceInfo::Usb:
    case QDeviceInfo::Vibration:
    case QDeviceInfo::Wlan:
    case QDeviceInfo::Sim:
    case QDeviceInfo::Positioning:
    case QDeviceInfo::VideoOut:
    case QDeviceInfo::Haptics:
    case QDeviceInfo::Nfc:
        return false;
    }
}

QDeviceInfo::LockTypeFlags QDeviceInfoPrivate::activatedLocks()
{
    QDeviceInfo::LockTypeFlags types = QDeviceInfo::NoLock;

    bool value(false);
    SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &value, 0);
    if (value)
        types |= QDeviceInfo::TouchOrKeyboardLock;

    // TODO: check if the workstation is currently locked

    return types;
}

QDeviceInfo::LockTypeFlags QDeviceInfoPrivate::enabledLocks()
{
    QDeviceInfo::LockTypeFlags types = QDeviceInfo::PinLock; // We can always lock the computer

    QScreenSaverPrivate screenSaverPrivate(0);
    if (screenSaverPrivate.screenSaverEnabled())
        types |= QDeviceInfo::TouchOrKeyboardLock;

    return types;
}

QDeviceInfo::ThermalState QDeviceInfoPrivate::thermalState()
{
    return QDeviceInfo::UnknownThermal;
}

QString QDeviceInfoPrivate::imei(int interface)
{
    Q_UNUSED(interface)
    return QString();
}

QString QDeviceInfoPrivate::manufacturer()
{
    if (systemManufacturerName.isEmpty()) {
        QSettings manufacturerSetting("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\BIOS", QSettings::NativeFormat);
        systemManufacturerName = manufacturerSetting.value("SystemManufacturer").toString();
    }
    return systemManufacturerName;
}

QString QDeviceInfoPrivate::model()
{
    return QString();
}

QString QDeviceInfoPrivate::productName()
{
    if (systemProductName.isEmpty()) {
        QSettings productNameSetting("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\BIOS", QSettings::NativeFormat);
        systemProductName = productNameSetting.value("SystemProductName").toString();
    }
    return systemProductName;
}

QString QDeviceInfoPrivate::uniqueDeviceID()
{
    if (deviceID.isEmpty()) {
        QSettings deviceIDSetting("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", QSettings::NativeFormat);
        deviceID = deviceIDSetting.value("ProductId").toString();
    }
    return deviceID;
}

QString QDeviceInfoPrivate::version(QDeviceInfo::Version type)
{
    switch (type) {
    case QDeviceInfo::Os:
        if (osVersion.isEmpty()) {
            OSVERSIONINFOEX versionInfo;
            versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
            if (GetVersionEx((_OSVERSIONINFOW*)&versionInfo)) {
                osVersion = QString::number(versionInfo.dwMajorVersion) + "." + QString::number(versionInfo.dwMinorVersion) + "."
                            + QString::number(versionInfo.dwBuildNumber) + "." + QString::number(versionInfo.wServicePackMajor) + "."
                            + QString::number(versionInfo.wServicePackMinor);
            }
        }
        return osVersion;

    case QDeviceInfo::Firmware:
        return QString();
    };
}

QT_END_NAMESPACE
