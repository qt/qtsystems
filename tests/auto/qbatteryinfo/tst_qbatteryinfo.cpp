/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: http://www.qt-project.org/
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

#include <QtTest/QtTest>
#include "qbatteryinfo.h"

QT_USE_NAMESPACE

class tst_QBatteryInfo : public QObject
{
    Q_OBJECT

private slots:
    void tst_capacity();
    void tst_flow();
    void tst_invalid();
};

void tst_QBatteryInfo::tst_capacity()
{
    QBatteryInfo batteryInfo;

    int count = batteryInfo.batteryCount();
    for (int i = 0; i < count; ++i) {
        int max = batteryInfo.maximumCapacity(i);
        int remaining = batteryInfo.remainingCapacity(i);

        QVERIFY(max == -1 || remaining == -1 || remaining <= max);
    }
}

void tst_QBatteryInfo::tst_flow()
{
    QBatteryInfo batteryInfo;

    int count = batteryInfo.batteryCount();
    for (int i = 0; i < count; ++i) {
        QBatteryInfo::ChargingState chargingState = batteryInfo.chargingState(i);
        switch (chargingState) {
        case QBatteryInfo::Charging:
            QVERIFY(batteryInfo.currentFlow(i) < 0);
            QVERIFY(batteryInfo.remainingChargingTime(i) > 0);
            break;
        case QBatteryInfo::NotCharging:
            QVERIFY(batteryInfo.currentFlow(i) == 0);
            QVERIFY(batteryInfo.remainingChargingTime(i) == 0);
            break;
        case QBatteryInfo::Discharging:
            QVERIFY(batteryInfo.currentFlow(i) > 0);
            QVERIFY(batteryInfo.remainingChargingTime(i) == 0);
            break;
        case QBatteryInfo::UnknownChargingState:
            break;
        }
    }
}

void tst_QBatteryInfo::tst_invalid()
{
    QBatteryInfo batteryInfo;

    QVERIFY(batteryInfo.currentFlow(-1) == 0);
    QVERIFY(batteryInfo.maximumCapacity(-1) == -1);
    QVERIFY(batteryInfo.remainingCapacity(-1) == -1);
    QVERIFY(batteryInfo.remainingChargingTime(-1) == -1);
    QVERIFY(batteryInfo.voltage(-1) == -1);
    QVERIFY(batteryInfo.chargingState(-1) == QBatteryInfo::UnknownChargingState);

    int count = batteryInfo.batteryCount();
    QVERIFY(batteryInfo.currentFlow(count) == 0);
    QVERIFY(batteryInfo.maximumCapacity(count) == -1);
    QVERIFY(batteryInfo.remainingCapacity(count) == -1);
    QVERIFY(batteryInfo.remainingChargingTime(count) == -1);
    QVERIFY(batteryInfo.voltage(count) == -1);
    QVERIFY(batteryInfo.chargingState(count) == QBatteryInfo::UnknownChargingState);
}

QTEST_MAIN(tst_QBatteryInfo)
#include "tst_qbatteryinfo.moc"
