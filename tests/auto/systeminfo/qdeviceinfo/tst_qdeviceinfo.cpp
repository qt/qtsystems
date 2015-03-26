/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
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

#include <QtTest/qtest.h>
#include "qdeviceinfo.h"

QT_USE_NAMESPACE

class tst_QDeviceInfo : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void tst_imei();
    void tst_lockTypes();
    void tst_version();
    void tst_manufacturer();
    void tst_model();
    void tst_productName();
    void tst_uniqueDeviceID();
    void tst_hasFeature();
    void tst_thermalState();

private:
    QDeviceInfo *deviceInfo;

};

void tst_QDeviceInfo::initTestCase()
{
    deviceInfo = new QDeviceInfo();
}

void tst_QDeviceInfo::cleanupTestCase()
{
    delete deviceInfo;
}

void tst_QDeviceInfo::tst_imei()
{
    int imeiCount = deviceInfo->imeiCount();
    QVERIFY(imeiCount >= -1);

    QVERIFY(deviceInfo->imei(-1).isEmpty());
    QVERIFY(deviceInfo->imei(imeiCount).isEmpty());

    QRegExp imeiPattern("(\\d{0,0}|\\d{15,15})");
    for (int i = 0; i < imeiCount; ++i)
        QVERIFY(imeiPattern.exactMatch(deviceInfo->imei(i)));
}

void tst_QDeviceInfo::tst_lockTypes()
{
    QDeviceInfo::LockTypeFlags enabledLocks = deviceInfo->enabledLocks();
    QDeviceInfo::LockTypeFlags activeLocks = deviceInfo->activatedLocks();
    QVERIFY((activeLocks & enabledLocks) == activeLocks);
    QVERIFY((activeLocks | enabledLocks) == enabledLocks);
}

void tst_QDeviceInfo::tst_version()
{
    QRegExp osversion(QString::fromLatin1("(\\d+(\\.\\d+)*)?"));
    QVERIFY(osversion.exactMatch(deviceInfo->version(QDeviceInfo::Os)));

    QRegExp firmwareversion(QString::fromLatin1(".*"));
    QVERIFY(firmwareversion.exactMatch(deviceInfo->version(QDeviceInfo::Firmware)));
}

void tst_QDeviceInfo::tst_manufacturer()
{
    QRegExp manufacturer(".*");
    QVERIFY(manufacturer.exactMatch(deviceInfo->manufacturer()));
}

void tst_QDeviceInfo::tst_model()
{
    QRegExp model(".*");
    QVERIFY(model.exactMatch(deviceInfo->model()));
}

void tst_QDeviceInfo::tst_productName()
{
    QRegExp productName(".*");
    QVERIFY(productName.exactMatch(deviceInfo->productName()));
}

void tst_QDeviceInfo::tst_uniqueDeviceID()
{
    QRegExp uniqueId("[A-Fa-f\\d-]*");
    QVERIFY(uniqueId.exactMatch(deviceInfo->uniqueDeviceID()));
}

void tst_QDeviceInfo::tst_hasFeature()
{
    QList<QDeviceInfo::Feature> featureList;
    featureList << QDeviceInfo::BluetoothFeature
                << QDeviceInfo::CameraFeature
                << QDeviceInfo::FmRadioFeature
                << QDeviceInfo::FmTransmitterFeature
                << QDeviceInfo::HapticsFeature
                << QDeviceInfo::InfraredFeature
                << QDeviceInfo::LedFeature
                << QDeviceInfo::MemoryCardFeature
                << QDeviceInfo::PositioningFeature
                << QDeviceInfo::SimFeature
                << QDeviceInfo::UsbFeature
                << QDeviceInfo::VibrationFeature
                << QDeviceInfo::VideoOutFeature
                << QDeviceInfo::WlanFeature;

    foreach (QDeviceInfo::Feature feature, featureList) {
        bool isPresent = deviceInfo->hasFeature(feature);
        QVERIFY(isPresent == true || isPresent == false);
    }
}

void tst_QDeviceInfo::tst_thermalState()
{
    QDeviceInfo::ThermalState state = deviceInfo->thermalState();
    QVERIFY(state == QDeviceInfo::UnknownThermal ||
            state == QDeviceInfo::NormalThermal ||
            state == QDeviceInfo::WarningThermal ||
            state == QDeviceInfo::AlertThermal ||
            state == QDeviceInfo::ErrorThermal);
}

QTEST_APPLESS_MAIN(tst_QDeviceInfo)
#include "tst_qdeviceinfo.moc"
