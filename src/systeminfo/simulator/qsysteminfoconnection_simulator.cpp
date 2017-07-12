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

#include <QtSimulator/QtSimulator>

#include <QTimer>
#include <QEventLoop>
#include <QMutex>
#include <QVersionNumber>

QT_BEGIN_NAMESPACE

const QString SERVERNAME(QStringLiteral("SystemInfo.Battery"));
const int PORT(0xbeef);
const QVersionNumber VERSION(1, 0, 0);

SystemInfoConnection::SystemInfoConnection(QObject *parent)
    : QObject(parent)
{
    qt_registerSystemInfoTypes();
    mConnection = new QSimulatorConnection(SERVERNAME, VERSION);
    mConnection->addPeerInfo(QLatin1String("name"), QLatin1String("systeminfo battery backend"));
    mConnection->addPeerInfo(QLatin1String("version"), QLatin1String("1.0.0"));
    mWorker = mConnection->connectToHost(QSimulatorConnection::simulatorHostName(true), PORT);
    if (mWorker) {
        mWorker->addReceiver(this);

        // wait until initial data is received
        QTimer timer;
        QEventLoop loop;
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        connect(this, &SystemInfoConnection::dataReceived, &loop, &QEventLoop::quit);
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
    emit dataReceived();
}

QT_END_NAMESPACE
