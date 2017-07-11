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

#include "qsysteminfoconnection_simulator_p.h"
#include "qsysteminfobackend_simulator_p.h"

#include <QtSimulator/connection.h>
#include <QtSimulator/version.h>
#include <QtSimulator/connectionworker.h>
#include <QtSimulator/QtSimulator>

#include <QTimer>
#include <QEventLoop>
#include <QMutex>

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
    if (mWorker) {
        mWorker->addReceiver(this);
        mWorker->call("setRequestsSystemInfo");

        // wait until initial data is received
        QTimer timer;
        QEventLoop loop;
        connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        connect(this, SIGNAL(initialDataReceived()), &loop, SLOT(quit()));
        timer.start(1000);
        loop.exec();
        timer.stop();
    } else
        qWarning("Could not connect to server");
}

SystemInfoConnection::~SystemInfoConnection()
{
    delete mWorker;
}

void SystemInfoConnection::ensureSimulatorConnection()
{
    static QMutex mutex;

    mutex.lock();
    static SystemInfoConnection systemInfoConnection;
    mutex.unlock();
}

void SystemInfoConnection::initialSystemInfoDataSent()
{
    mInitialDataSent = true;
    emit initialDataReceived();
}

void SystemInfoConnection::setBatteryInfoData(const QBatteryInfoData &data)
{
    QBatteryInfoSimulatorBackend *batteryInfoBackend = QBatteryInfoSimulatorBackend::getSimulatorBackend();

    batteryInfoBackend->setMaximumCapacity(data.maximumCapacity);
    batteryInfoBackend->setRemainingCapacity(data.remainingCapacity);
    batteryInfoBackend->setCurrentFlow(data.currentFlow);
    batteryInfoBackend->setCycleCount(data.cycleCount);
    batteryInfoBackend->setVoltage(data.voltage);
    batteryInfoBackend->setRemainingChargingTime(data.remainingChargingTime);
    batteryInfoBackend->setChargerType(data.chargerType);
    batteryInfoBackend->setChargingState(data.chargingState);
    batteryInfoBackend->setLevelStatus(data.levelStatus);
    batteryInfoBackend->setHealth(data.health);
    batteryInfoBackend->setTemperature(data.temperature);
}

QT_END_NAMESPACE
