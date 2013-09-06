/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
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
#include "qstorageinfo.h"
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

static const symbol_t BatteryStatus_lut[] =
{
    SYM(QBatteryInfo::BatteryStatusUnknown),
    SYM(QBatteryInfo::BatteryEmpty),
    SYM(QBatteryInfo::BatteryLow),
    SYM(QBatteryInfo::BatteryOk),
    SYM(QBatteryInfo::BatteryFull),
};

static const symbol_t ChargingState_lut[] =
{
    SYM(QBatteryInfo::UnknownChargingState),
    SYM(QBatteryInfo::NotCharging),
    SYM(QBatteryInfo::Charging),
    SYM(QBatteryInfo::Discharging),
    SYM(QBatteryInfo::Full),
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
 * test_systemstorageinfo
 * ------------------------------------------------------------------------- */

static const char *human_size(qlonglong n)
{
  if (n == 0) return "0B";

  static char buf[256];
  char *pos = buf;
  char *end = buf + sizeof buf;

  int b = n & 1023; n >>= 10;
  int k = n & 1023; n >>= 10;
  int m = n & 1023; n >>= 10;
  int g = n & 1023; n >>= 10;

  *pos = 0;
#if defined(Q_OS_WIN)
  if (g) _snprintf_s(pos, sizeof(pos), end-pos,"%s%dGB", *buf?" ":"", g), pos = strchr(pos,0);
  if (m) _snprintf_s(pos, sizeof(pos), end-pos,"%s%dMB", *buf?" ":"", m), pos = strchr(pos,0);
  if (k) _snprintf_s(pos, sizeof(pos), end-pos,"%s%dkB", *buf?" ":"", k), pos = strchr(pos,0);
  if (b) _snprintf_s(pos, sizeof(pos), end-pos,"%s%dB",  *buf?" ":"", b), pos = strchr(pos,0);
#else
  if (g) snprintf(pos, end-pos, "%s%dGB", *buf?" ":"", g), pos = strchr(pos,0);
  if (m) snprintf(pos, end-pos, "%s%dMB", *buf?" ":"", m), pos = strchr(pos,0);
  if (k) snprintf(pos, end-pos, "%s%dkB", *buf?" ":"", k), pos = strchr(pos,0);
  if (b) snprintf(pos, end-pos, "%s%dB",  *buf?" ":"", b), pos = strchr(pos,0);
#endif
  return buf;
}

static void test_storageinfo(void)
{
  QStorageInfo storageinfo;

  QStringList lst = storageinfo.allLogicalDrives();

  qDebug() << "storageinfo.logicalDrives ->" << lst;

  for (int i = 0; i < lst.size(); ++i) {
    const QString &drv = lst.at(i);

    qDebug() << "Logical drive:" << drv;

    qlonglong avail = storageinfo.availableDiskSpace(drv);
    qDebug() << "  storageinfo.availableDiskSpace() ->" << human_size(avail);

    qlonglong total = storageinfo.totalDiskSpace(drv);
    qDebug() << "  storageinfo.totalDiskSpace() ->" << human_size(total);

    QStorageInfo::DriveType dtype = storageinfo.driveType(drv);
    qDebug() << "  storageinfo.typeForDrive() ->" << dtype;

    QString duri = storageinfo.uriForDrive(drv);
    qDebug() << "  storageinfo.uriForDrive() ->" << duri;
  }
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
    X(batInfo.chargerType());
    X(batInfo.chargingState(0) );
    X(batInfo.maximumCapacity(0));
    X(batInfo.remainingCapacity(0));
    X(batInfo.voltage(0));
    X(batInfo.remainingChargingTime(0));
    X(batInfo.currentFlow(0));
    X(batInfo.batteryStatus(0));
    X(batInfo.energyUnit());
    X(batInfo.batteryCount());
}


struct dummy_t
{
  const char *name;
  void (*func)(void);
} lut[] = {
#define ADD(x) {#x, test_##x }
//  ADD(systeminfo),
  ADD(deviceinfo),
  ADD(storageinfo),
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
