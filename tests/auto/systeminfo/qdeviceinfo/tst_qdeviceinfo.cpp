/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/qregularexpression.h>
#include <QtTest/qtest.h>
#include "qdeviceinfo.h"

QT_USE_NAMESPACE

class tst_QDeviceInfo : public QObject
{
    Q_OBJECT

private:
    bool hasExactMatch(const QString& needle, const QString& haystack);

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

bool tst_QDeviceInfo::hasExactMatch(const QString& needle, const QString& haystack)
{
    QRegularExpression pattern(QRegularExpression::anchoredPattern(needle));
    return pattern.match(haystack).hasMatch();
}

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

    QRegularExpression imeiPattern(QRegularExpression::anchoredPattern("(\\d{0,0}|\\d{15,15})"));
    for (int i = 0; i < imeiCount; ++i)
        QVERIFY(imeiPattern.match(deviceInfo->imei(i)).hasMatch());
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
    QVERIFY(hasExactMatch(QString::fromLatin1("(\\d+(\\.\\d+)*)?"),
                          deviceInfo->version(QDeviceInfo::Os)));

    QVERIFY(hasExactMatch(QString::fromLatin1(".*"),
                          deviceInfo->version(QDeviceInfo::Firmware)));
}

void tst_QDeviceInfo::tst_manufacturer()
{
    QVERIFY(hasExactMatch(QString::fromLatin1(".*"),
            deviceInfo->manufacturer()));
}

void tst_QDeviceInfo::tst_model()
{
    QVERIFY(hasExactMatch(QString::fromLatin1(".*"),
            deviceInfo->model()));
}

void tst_QDeviceInfo::tst_productName()
{
    QVERIFY(hasExactMatch(QString::fromLatin1(".*"),
            deviceInfo->productName()));

}

void tst_QDeviceInfo::tst_uniqueDeviceID()
{
    QVERIFY(hasExactMatch(QString::fromLatin1("[A-Fa-f\\d-]*"),
            deviceInfo->uniqueDeviceID()));
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
