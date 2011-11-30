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

#include <Winsock2.h>
#include <windows.h>
#include <Vfw.h>
#include <BluetoothAPIs.h>
#include <Wlanapi.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(const QString, REGISTRY_BIOS_PATH, (QStringLiteral("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\BIOS")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, REGISTRY_CURRENT_VERSION_PATH, (QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, REGISTRY_MANUFACTURER_KEY, (QStringLiteral("SystemManufacturer")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, REGISTRY_PRODUCTNAME_KEY, (QStringLiteral("SystemProductName")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, REGISTRY_PRODUCTID_KEY, (QStringLiteral("ProductId")))

QDeviceInfoPrivate::QDeviceInfoPrivate(QDeviceInfo *parent)
    : q_ptr(parent)
{
}

bool QDeviceInfoPrivate::hasFeature(QDeviceInfo::Feature feature)
{
    switch (feature) {
    case QDeviceInfo::Bluetooth: {
        BLUETOOTH_DEVICE_SEARCH_PARAMS searchParameter;
        searchParameter.dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);
        searchParameter.fReturnAuthenticated = TRUE;
        searchParameter.fReturnRemembered = TRUE;
        searchParameter.fReturnUnknown = TRUE;
        searchParameter.fReturnConnected = TRUE;
        searchParameter.fIssueInquiry = TRUE;
        searchParameter.cTimeoutMultiplier = 1;
        searchParameter.hRadio = NULL;

        BLUETOOTH_DEVICE_INFO deviceInfo;
        HBLUETOOTH_DEVICE_FIND handle = BluetoothFindFirstDevice(&searchParameter, &deviceInfo);
        if (handle) {
            BluetoothFindDeviceClose(handle);
            return true;
        } else {
            return false;
        }
    }

    case QDeviceInfo::Wlan: {
        bool supportsWlan(false);
        DWORD negotiatedVersion;
        HANDLE handle;
        if (ERROR_SUCCESS == WlanOpenHandle(1, NULL, &negotiatedVersion, &handle)) {
            PWLAN_INTERFACE_INFO_LIST list;
            if (ERROR_SUCCESS == WlanEnumInterfaces(handle, NULL, &list)) {
                if (list->dwNumberOfItems > 0)
                    supportsWlan = true;
                WlanFreeMemory(list);
            }
            WlanCloseHandle(handle, NULL);
        }
        return supportsWlan;
    }

    case QDeviceInfo::VideoOut:
        return (GetSystemMetrics(SM_CMONITORS) > 0);

    case QDeviceInfo::Infrared: {
        WSADATA wsaData;
        if (0 == WSAStartup(MAKEWORD(1,1), &wsaData)) {
            SOCKET irdaSocket = socket(AF_IRDA, SOCK_STREAM, 0);
            if (INVALID_SOCKET != irdaSocket) {
                closesocket(irdaSocket);
                return true;
            }
        }
        return false;
    }

    case QDeviceInfo::Camera: {
        char name[256];
        char version[256];
        for (WORD i = 0; i < 10; i++) {
            if (capGetDriverDescriptionA(i, name, 256, version, 256))
                return true;
        }
        return false;
    }

//    not sure if we can use WDK, thus not implemented as of now
//    case QDeviceInfo::MemoryCard:
//    case QDeviceInfo::Usb:

//    case QDeviceInfo::Led:
//    case QDeviceInfo::Positioning:
//    case QDeviceInfo::FmRadio:
//    case QDeviceInfo::FmTransmitter:
//    case QDeviceInfo::Vibration:
//    case QDeviceInfo::Sim:
//    case QDeviceInfo::Haptics:
//    case QDeviceInfo::Nfc:
    default:
        return false;
    }
}

int QDeviceInfoPrivate::imeiCount()
{
    return -1;
}

QDeviceInfo::LockTypeFlags QDeviceInfoPrivate::activatedLocks()
{
    QDeviceInfo::LockTypeFlags types = QDeviceInfo::NoLock;

    bool value(false);
    SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &value, 0);
    if (value)
        types |= QDeviceInfo::TouchOrKeyboardLock;

    HDESK desktop = OpenDesktopA("Default", 0, false, DESKTOP_SWITCHDESKTOP);
    if (desktop) {
        if (0 == SwitchDesktop(desktop))
            types |= QDeviceInfo::PinLock;
        CloseDesktop(desktop);
    }

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

QString QDeviceInfoPrivate::imei(int /* interfaceNumber */)
{
    return QString();
}

QString QDeviceInfoPrivate::manufacturer()
{
    if (systemManufacturerName.isEmpty()) {
        QSettings manufacturerSetting(*REGISTRY_BIOS_PATH(), QSettings::NativeFormat);
        systemManufacturerName = manufacturerSetting.value(*REGISTRY_MANUFACTURER_KEY()).toString();
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
        QSettings productNameSetting(*REGISTRY_BIOS_PATH(), QSettings::NativeFormat);
        systemProductName = productNameSetting.value(*REGISTRY_PRODUCTNAME_KEY()).toString();
    }
    return systemProductName;
}

QString QDeviceInfoPrivate::uniqueDeviceID()
{
    if (deviceID.isEmpty()) {
        QSettings deviceIDSetting(*REGISTRY_CURRENT_VERSION_PATH(), QSettings::NativeFormat);
        deviceID = deviceIDSetting.value(*REGISTRY_PRODUCTID_KEY()).toString();
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
                osVersion = QString::number(versionInfo.dwMajorVersion) + QLatin1Char('.') + QString::number(versionInfo.dwMinorVersion) + QLatin1Char('.')
                            + QString::number(versionInfo.dwBuildNumber) + QLatin1Char('.') + QString::number(versionInfo.wServicePackMajor) + QLatin1Char('.')
                            + QString::number(versionInfo.wServicePackMinor);
            }
        }
        return osVersion;

    case QDeviceInfo::Firmware:
        break;
    }
    return QString();
}

QT_END_NAMESPACE
