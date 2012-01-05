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

#ifndef QSYSTEMINFO_SIMULATOR_P_H
#define QSYSTEMINFO_SIMULATOR_P_H


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

#include <qbatteryinfo.h>
#include <qdeviceinfo.h>
#include <qstorageinfo.h>

QT_BEGIN_NAMESPACE

class QBatteryInfoSimulatorBackend;
class QDeviceInfoSimulatorBackend;
class QStorageInfoSimulatorBackend;

#if defined(Q_OS_LINUX) && !defined(QT_NO_JSONDB)
class QDeviceInfoPrivate;
#endif

class QBatteryInfoSimulator : public QObject
{
    Q_OBJECT

public:
    QBatteryInfoSimulator(QBatteryInfo *parent);
    ~QBatteryInfoSimulator();

    int batteryCount();
    int currentFlow(int battery);
    int maximumCapacity(int battery);
    int remainingCapacity(int battery);
    int remainingChargingTime(int battery);
    int voltage(int battery);
    QBatteryInfo::ChargerType chargerType();
    QBatteryInfo::ChargingState chargingState(int battery);
    QBatteryInfo::EnergyUnit energyUnit();

Q_SIGNALS:
    void batteryCountChanged(int count);
    void chargerTypeChanged(QBatteryInfo::ChargerType type);
    void chargingStateChanged(int battery, QBatteryInfo::ChargingState state);
    void currentFlowChanged(int battery, int flow);
    void remainingCapacityChanged(int battery, int capacity);
    void remainingChargingTimeChanged(int battery, int seconds);
    void voltageChanged(int battery, int voltage);

protected:
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);

private:
    QBatteryInfo * const q_ptr;
    Q_DECLARE_PUBLIC(QBatteryInfo)

    QBatteryInfoSimulatorBackend *batteryInfoSimulatorBackend;
};

class QDeviceInfoSimulator : public QObject
{
    Q_OBJECT

public:
    QDeviceInfoSimulator(QDeviceInfo *parent);
    ~QDeviceInfoSimulator();

    bool hasFeature(QDeviceInfo::Feature feature);
    int imeiCount();
    QDeviceInfo::LockTypeFlags activatedLocks();
    QDeviceInfo::LockTypeFlags enabledLocks();
    QDeviceInfo::ThermalState thermalState();
    QString imei(int interface);
    QString manufacturer();
    QString model();
    QString productName();
    QString uniqueDeviceID();
    QString version(QDeviceInfo::Version type);

Q_SIGNALS:
    void activatedLocksChanged(QDeviceInfo::LockTypeFlags types);
    void enabledLocksChanged(QDeviceInfo::LockTypeFlags types);
    void thermalStateChanged(QDeviceInfo::ThermalState state);

protected:
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);

private:
    QDeviceInfo * const q_ptr;
    Q_DECLARE_PUBLIC(QDeviceInfo)

    QDeviceInfoSimulatorBackend *deviceInfoSimulatorBackend;

#if defined(Q_OS_LINUX) && !defined(QT_NO_JSONDB)
    QDeviceInfoPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(QDeviceInfo)
#endif
};

class QStorageInfoSimulator : public QObject
{
    Q_OBJECT

public:
    QStorageInfoSimulator(QStorageInfo *parent);
    ~QStorageInfoSimulator();

    qlonglong availableDiskSpace(const QString &drive);
    qlonglong totalDiskSpace(const QString &drive);
    QString uriForDrive(const QString &drive);
    QStringList allLogicalDrives();
    QStorageInfo::DriveType driveType(const QString &drive);

Q_SIGNALS:
    void logicalDriveChanged(const QString &drive, bool added);

protected:
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);

private:
    QStorageInfo * const q_ptr;
    Q_DECLARE_PUBLIC(QStorageInfo)

    QStorageInfoSimulatorBackend *storageInfoSimulatorBackend;
};

QT_END_NAMESPACE

#endif /*QSYSTEMINFO_SIMULATOR_P_H*/
