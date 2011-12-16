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

#include "qsysteminfoconnection_simulator_p.h"
#include "qsysteminfobackend_simulator_p.h"

#include <QtSimulator/connection.h>
#include <QtSimulator/version.h>
#include <QtSimulator/connectionworker.h>
#include <QtSimulator/QtSimulator>

QT_BEGIN_NAMESPACE

using namespace Simulator;

const QString SystemInfoConnection::SERVERNAME(QStringLiteral("QtSimulator_Mobility_ServerName1.3.0.0"));
const int SystemInfoConnection::PORT(0xbeef+1);
const Simulator::Version SystemInfoConnection::VERSION(1,3,0,0);

SystemInfoConnection::SystemInfoConnection(QObject *parent)
    : QObject(parent)
    , mInitialDataSent(false)
{
    qt_registerSystemInfoTypes();
    mConnection = new Connection(Connection::Client, SERVERNAME, PORT, VERSION, this);
    mWorker = mConnection->connectToServer(Connection::simulatorHostName(true), PORT);
    if (!mWorker)
        qFatal("Could not connect to server");

    mWorker->addReceiver(this);
    mWorker->call("setRequestsSystemInfo");

    // wait until initial data is received
    QEventLoop loop;
    connect(this, SIGNAL(initialDataReceived()), &loop, SLOT(quit()));
    loop.exec();
}

SystemInfoConnection::~SystemInfoConnection()
{
    delete mWorker;
}

void SystemInfoConnection::ensureSimulatorConnection()
{
    static SystemInfoConnection systemInfoConnection;
}

void SystemInfoConnection::initialSystemInfoDataSent()
{
    mInitialDataSent = true;
    emit initialDataReceived();
}

void SystemInfoConnection::setBatteryInfoData(const QBatteryInfoData &data)
{
    QBatteryInfoSimulatorBackend *batteryInfoBackend = QBatteryInfoSimulatorBackend::getSimulatorBackend();

    batteryInfoBackend->setRemainingCapacity(data.remainingCapacity);
    batteryInfoBackend->setMaximumCapacity(data.maximumCapacity);
    batteryInfoBackend->setCurrentFlow(data.currentFlow);
    batteryInfoBackend->setVoltage(data.voltage);
    batteryInfoBackend->setRemainingChargingTime(data.remainingChargingTime);
    batteryInfoBackend->setChargerType(data.chargerType);
    batteryInfoBackend->setEnergyUnit(data.energyMeasurementUnit);
    batteryInfoBackend->setChargingState(data.chargingState);
}

void SystemInfoConnection::setDeviceInfoData(const QDeviceInfoData &data)
{
    QDeviceInfoSimulatorBackend *deviceInfoBackend = QDeviceInfoSimulatorBackend::getSimulatorBackend();

    deviceInfoBackend->setManufacturer(data.manufacturer);
    deviceInfoBackend->setModel(data.model);
    deviceInfoBackend->setProductName(data.productName);
    deviceInfoBackend->setActivatedLocks(data.activatedLocks);
    deviceInfoBackend->setEnabledLocks(data.enabledLocks);
    deviceInfoBackend->setUniqueDeviceID(data.uniqueDeviceID);
    deviceInfoBackend->setThermalState(data.currentThermalState);
    deviceInfoBackend->setVersion(QDeviceInfo::Os, data.versionList.value(QDeviceInfo::Os));
    deviceInfoBackend->setVersion(QDeviceInfo::Firmware, data.versionList.value(QDeviceInfo::Firmware));

    int imeiCount = deviceInfoBackend->getImeiCount();
    int receivedImeiCount = data.imeiList.count();
    if (imeiCount > receivedImeiCount) {
        int imeiIndex;
        for (imeiIndex = 0; imeiIndex < imeiCount; imeiIndex++)
            deviceInfoBackend->setImei(imeiIndex, QStringLiteral(""));
        for (imeiIndex = 0; imeiIndex < receivedImeiCount; imeiIndex++)
            deviceInfoBackend->setImei(imeiIndex, data.imeiList.value(imeiIndex));
    } else {
        for (int imeiIndex = 0; imeiIndex < imeiCount; imeiIndex++)
            deviceInfoBackend->setImei(imeiIndex, data.imeiList.value(imeiIndex));
    }

    QList<QDeviceInfo::Feature> featureKeys = data.featureList.keys();
    foreach (const QDeviceInfo::Feature &featureKey, featureKeys)
        deviceInfoBackend->setFeature(featureKey, data.featureList.value(featureKey));
}

QT_END_NAMESPACE
