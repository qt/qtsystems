/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Copyright (C) 2014 BlackBerry Limited. All rights reserved.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QCoreApplication>
#include <QDebug>
#include <QStringList>


#include "qbatteryinfo.h"
#include "qdeviceinfo.h"
#include "qnetworkinfo.h"
//#include "qscreensaverinfo.h"


#define X(expr) qDebug() << #expr << "->" << (expr);

struct symbol_t
{
  const char *key;
  int val;
};

int lookup(const symbol_t *stab, const char *key, int def)
{
  for (;stab->key;++stab) {
    if (!strcmp(stab->key,key)) return stab->val;
  }
  return def;
}

const char *rlookup(const symbol_t *stab, int val, const char *def)
{
  for (;stab->key; ++stab) {
    if (stab->val == val) return stab->key;
  }
  return def;
}

#define SYM(x) { #x, x }

static const symbol_t Version_lut[] =
{
  SYM(QDeviceInfo::Os),
  SYM(QDeviceInfo::Firmware),
  {0,0}
};

static const symbol_t Feature_lut[] =
{
  SYM(QDeviceInfo::BluetoothFeature),
  SYM(QDeviceInfo::CameraFeature),
  SYM(QDeviceInfo::FmRadioFeature),
  SYM(QDeviceInfo::FmTransmitterFeature),
  SYM(QDeviceInfo::InfraredFeature),
  SYM(QDeviceInfo::LedFeature),
  SYM(QDeviceInfo::MemoryCardFeature),
  SYM(QDeviceInfo::UsbFeature),
  SYM(QDeviceInfo::VibrationFeature),
  SYM(QDeviceInfo::WlanFeature),
  SYM(QDeviceInfo::SimFeature),
  SYM(QDeviceInfo::PositioningFeature),
  SYM(QDeviceInfo::VideoOutFeature),
  SYM(QDeviceInfo::HapticsFeature),
  SYM(QDeviceInfo::NfcFeature),
  {0,0}
};

static const symbol_t Thermal_lut[] =
{
  SYM(QDeviceInfo::UnknownThermal),
    SYM(QDeviceInfo::UnknownThermal),
    SYM(QDeviceInfo::NormalThermal),
    SYM(QDeviceInfo::WarningThermal),
    SYM(QDeviceInfo::AlertThermal),
    SYM(QDeviceInfo::ErrorThermal),
    {0,0}
};
static const symbol_t NetworkStatus_lut[] =
{
  SYM(QNetworkInfo::UnknownStatus),
  SYM(QNetworkInfo::NoNetworkAvailable),
  SYM(QNetworkInfo::EmergencyOnly),
  SYM(QNetworkInfo::Searching),
  SYM(QNetworkInfo::Busy),
//  SYM(QNetworkInfo::Connected),
  SYM(QNetworkInfo::HomeNetwork),
  SYM(QNetworkInfo::Denied),
  SYM(QNetworkInfo::Roaming),
  {0,0}
};

static const symbol_t NetworkMode_lut[] =
{
  SYM(QNetworkInfo::UnknownMode),
  SYM(QNetworkInfo::GsmMode),
  SYM(QNetworkInfo::CdmaMode),
  SYM(QNetworkInfo::WcdmaMode),
  SYM(QNetworkInfo::WlanMode),
  SYM(QNetworkInfo::EthernetMode),
  SYM(QNetworkInfo::BluetoothMode),
  SYM(QNetworkInfo::WimaxMode),
  SYM(QNetworkInfo::LteMode),
  SYM(QNetworkInfo::TdscdmaMode),
  {0,0}
};

static const symbol_t Health_lut[] =
{
    SYM(QBatteryInfo::HealthUnknown),
    SYM(QBatteryInfo::HealthOk),
    SYM(QBatteryInfo::HealthBad),
};

static const symbol_t LevelStatus_lut[] =
{
    SYM(QBatteryInfo::LevelUnknown),
    SYM(QBatteryInfo::LevelEmpty),
    SYM(QBatteryInfo::LevelLow),
    SYM(QBatteryInfo::LevelOk),
    SYM(QBatteryInfo::LevelFull),
};

static const symbol_t ChargingState_lut[] =
{
    SYM(QBatteryInfo::UnknownChargingState),
    SYM(QBatteryInfo::Charging),
    SYM(QBatteryInfo::IdleChargingState),
    SYM(QBatteryInfo::Discharging)
};

static const symbol_t ChargerType_lut[] =
{
    SYM(QBatteryInfo::UnknownCharger),
    SYM(QBatteryInfo::WallCharger),
    SYM(QBatteryInfo::USBCharger),
    SYM(QBatteryInfo::VariableCurrentCharger),
};


/* ------------------------------------------------------------------------- *
 * test_systemdeviceinfo
 * ------------------------------------------------------------------------- */

static void test_deviceinfo(void)
{
  QDeviceInfo deviceinfo;

  X(deviceinfo.hasFeature(QDeviceInfo::BluetoothFeature));
  X(deviceinfo.hasFeature(QDeviceInfo::CameraFeature));
  X(deviceinfo.hasFeature(QDeviceInfo::FmRadioFeature));
  X(deviceinfo.hasFeature(QDeviceInfo::FmTransmitterFeature));
  X(deviceinfo.hasFeature(QDeviceInfo::InfraredFeature));
  X(deviceinfo.hasFeature(QDeviceInfo::LedFeature));
  X(deviceinfo.hasFeature(QDeviceInfo::MemoryCardFeature));
  X(deviceinfo.hasFeature(QDeviceInfo::UsbFeature));
  X(deviceinfo.hasFeature(QDeviceInfo::VibrationFeature));
  X(deviceinfo.hasFeature(QDeviceInfo::WlanFeature));
  X(deviceinfo.hasFeature(QDeviceInfo::SimFeature));
  X(deviceinfo.hasFeature(QDeviceInfo::PositioningFeature));
  X(deviceinfo.hasFeature(QDeviceInfo::VideoOutFeature));
  X(deviceinfo.hasFeature(QDeviceInfo::HapticsFeature));
  X(deviceinfo.hasFeature(QDeviceInfo::NfcFeature));

  X(deviceinfo.imei(0));
  X(deviceinfo.manufacturer());
  X(deviceinfo.model());
  X(deviceinfo.productName());
  X(deviceinfo.uniqueDeviceID());

  X(deviceinfo.version(QDeviceInfo::Os));
  X(deviceinfo.version(QDeviceInfo::Firmware));

  X(deviceinfo.boardName());
  X(deviceinfo.operatingSystemName());

  X(deviceinfo.thermalState());
}

/* ------------------------------------------------------------------------- *
 * test_systemnetworkinfo
 * ------------------------------------------------------------------------- */

static void test_networkinfo(void)
{
  QNetworkInfo networkinfo;

  X(networkinfo.cellId(0));
  X(networkinfo.currentMobileCountryCode(0));
  X(networkinfo.currentMobileNetworkCode(0));
  X(networkinfo.homeMobileCountryCode(0));
  X(networkinfo.homeMobileNetworkCode(0));
  X(networkinfo.locationAreaCode(0));
  X(networkinfo.currentCellDataTechnology(0));


  for (const symbol_t *sym = NetworkMode_lut; sym->key; ++sym) {
      QNetworkInfo::NetworkMode mode =
              (QNetworkInfo::NetworkMode) sym->val;

    if (QCoreApplication::arguments().count() > 2)
        if (!QString(sym->key).contains(QCoreApplication::arguments().at(2),Qt::CaseInsensitive))
            continue;

    qDebug() << "";
    qDebug() << "NetworkMode:" << sym->key;

    QNetworkInfo::NetworkMode netmode = networkinfo.currentNetworkMode();
    qDebug() << "  networkinfo.currentNetworkMode() ->" << netmode;

    int intCount = networkinfo.networkInterfaceCount(mode);
    qDebug() << "  networkinfo.networkInterfaceCount() ->" << intCount;

    for (int j=0; j < networkinfo.networkInterfaceCount(mode);j++) {
        QNetworkInterface iface = networkinfo.interfaceForMode(mode,j);
        qDebug() << "  networkinfo.interfaceForMode() ->" << iface;

        QString macaddr = networkinfo.macAddress(mode,j);
        qDebug() << "  networkinfo.macAddress() ->" << macaddr;

        QNetworkInfo::NetworkStatus status = networkinfo.networkStatus(mode,j);
        qDebug() << "  networkinfo.networkStatus() ->" << status;

        QString network = networkinfo.networkName(mode,j);
        qDebug() << "  networkinfo.networkName() ->" << network;

        int sigstr = networkinfo.networkSignalStrength(mode,j);
        qDebug() << "  networkinfo.networkSignalStrength() ->" << sigstr;
    }
  }
}

/* ------------------------------------------------------------------------- *
 * test_systemscreensaver
 * ------------------------------------------------------------------------- */
//static void test_systemscreensaver(void)
//{
//  QSystemScreenSaver screensaver;

//  X(screensaver.screenSaverInhibited());
//  X(screensaver.setScreenSaverInhibit());
//}*/

/* ------------------------------------------------------------------------- *
 * test_systembatteryinfo
 * ------------------------------------------------------------------------- */

static void test_batteryinfo(void)
{
    QBatteryInfo batInfo;
    X(batInfo.batteryIndex());
    X(batInfo.chargerType());
    X(batInfo.chargingState() );
    X(batInfo.maximumCapacity());
    X(batInfo.remainingCapacity());
    X(batInfo.voltage());
    X(batInfo.remainingChargingTime());
    X(batInfo.currentFlow());
    X(batInfo.levelStatus());
    X(batInfo.batteryCount());
    X(batInfo.health());
}


struct dummy_t
{
  const char *name;
  void (*func)(void);
} lut[] = {
#define ADD(x) {#x, test_##x }
//  ADD(systeminfo),
  ADD(deviceinfo),
  ADD(networkinfo),
//  ADD(screensaver),
  ADD(batteryinfo),
#undef ADD
  {0,0}
};

static bool endswith(const char *str, const char *pat)
{
  int slen = strlen(str);
  int plen = strlen(pat);
  return (slen >= plen) && !strcmp(str+slen-plen, pat);
}

int lookup_test(const char *name)
{
  for (int i = 0; lut[i].name; ++i) {
    if (!strcmp(lut[i].name, name)) return i;
  }
  for (int i = 0; lut[i].name; ++i) {
    if (endswith(lut[i].name, name)) return i;
  }
  for (int i = 0; lut[i].name; ++i) {
    if (strstr(lut[i].name, name)) return i;
  }
  return -1;
}

int main(int ac, char **av)
{
#if !defined(Q_OS_WIN)
    if (!getenv("DISPLAY")) {
    qDebug() << "$DISPLAY not set, assuming :0";
    setenv("DISPLAY", ":0", 1);
  }
  if (!getenv("DBUS_SESSION_BUS_ADDRESS")) {
    qDebug() << "session bus not configured";
  }
#endif
  QCoreApplication app(ac, av, true);

  if (ac < 2) {
    qDebug() << "available tests:";
    for (int k = 0; lut[k].name; ++k) {
      qDebug() << *av << lut[k].name;
    }
    exit(0);
  }

  for (int i = 1; i < ac; ++i) {
    const char *name = av[i];

    int k = lookup_test(name);

    if (k != -1) {
      qDebug() << "";
      qDebug() << "----(" << lut[k].name << ")----";
      qDebug() << "";
      lut[k].func();
    } else if ( !strcmp(name, "all")) {
      for (int k = 0; lut[k].name; ++k) {
        qDebug() << "";
        qDebug() << "----(" << lut[k].name << ")----";
        qDebug() << "";
        lut[k].func();
      }
    } else {
      break;
    }
  }
}

// EOF
