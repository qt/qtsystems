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

//TESTED_COMPONENT=src/serviceframework

#include <QtTest/QtTest>
#include <QtCore>
#include <qservicefilter.h>

QT_USE_NAMESPACE
class tst_QServiceFilter: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void versionMatching();
    void versionMatching_data();
    void setInterface();
    void testAssignmentOperator();
    void testConstructor();
#ifndef QT_NO_DATASTREAM
    void streamTest();
#endif
    void testCustomAttribute();
    void testCapabilities();

};

void tst_QServiceFilter::initTestCase()
{
}

void tst_QServiceFilter::versionMatching_data()
{
    QTest::addColumn<QString>("version");
    QTest::addColumn<int>("majorV");
    QTest::addColumn<int>("minorV");
    QTest::addColumn<int>("rule");

    //invalid cases
    QTest::newRow("versionMatching_data():Invalid 1") << "" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 3") << "01.3" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 4") << "1.03" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 5") << "x.y" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 6") << "23" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 7") << "sdfsfs" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 8") << "%#5346" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 9") << ".66" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 10") << "1.3.4" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 11") << "1.a" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 12") << "b.1" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 13") << "3." << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 14") << "-1" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 16") << ".x" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 17") << "x." << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 18") << "1.  0" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 19") << "1  .0" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 20") << "1  0" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 21") << "1 . 0" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 22") << " 1.5" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 23") << "1.5 " << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 24") << " 1.5 " << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 25") << "1.5 1.6" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 26") << "-1.0" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 27") << "1.-1" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 28") << "-5.-1" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 29") << "4,8" << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():Invalid 30") << "   " << -1 << -1 << (int)QServiceFilter::MinimumVersionMatch;


    //valid cases
    QTest::newRow("versionMatching_data():ValidMin 1") << "1.0" << 1 << 0 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():ValidMin 2") << "1.00" << 1 << 0 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():ValidMin 3") << "99.99" << 99 << 99 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():ValidMin 4") << "2.3" << 2 << 3 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():ValidMin 5") << "10.3" << 10 << 3 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():ValidMin 6") << "5.10" << 5 << 10 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():ValidMin 7") << "10.10" << 10 << 10 << (int)QServiceFilter::MinimumVersionMatch;

    QTest::newRow("versionMatching_data():ValidMin 8") << "0.3" << 0 << 3 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():ValidMin 10") << "0.0" << 0 << 0 << (int)QServiceFilter::MinimumVersionMatch;
    QTest::newRow("versionMatching_data():ValidMin 11") << "00.00" << 0 << 0 << (int)QServiceFilter::MinimumVersionMatch;

    QTest::newRow("versionMatching_data():ValidExact 1") << "1.0" << 1 << 0 << (int)QServiceFilter::ExactVersionMatch;
    QTest::newRow("versionMatching_data():ValidExact 2") << "1.00" << 1 << 0 << (int)QServiceFilter::ExactVersionMatch;
    QTest::newRow("versionMatching_data():ValidExact 3") << "99.99" << 99 << 99 << (int)QServiceFilter::ExactVersionMatch;
    QTest::newRow("versionMatching_data():ValidExact 4") << "2.3" << 2 << 3 << (int)QServiceFilter::ExactVersionMatch;
    QTest::newRow("versionMatching_data():ValidExact 5") << "10.3" << 10 << 3 << (int)QServiceFilter::ExactVersionMatch;
    QTest::newRow("versionMatching_data():ValidExact 6") << "5.10" << 5 << 10 << (int)QServiceFilter::ExactVersionMatch;
    QTest::newRow("versionMatching_data():ValidExact 7") << "10.10" << 10 << 10 << (int)QServiceFilter::ExactVersionMatch;

    QTest::newRow("versionMatching_data():ValidExact 8") << "0.3" << 0 << 3 << (int)QServiceFilter::ExactVersionMatch;
    QTest::newRow("versionMatching_data():ValidExact 10") << "0.0" << 0 << 0 << (int)QServiceFilter::ExactVersionMatch;
    QTest::newRow("versionMatching_data():ValidExact 11") << "00.00" << 0 << 0 << (int)QServiceFilter::ExactVersionMatch;
}

void tst_QServiceFilter::versionMatching()
{
    QFETCH(QString, version);
    QFETCH(int, majorV);
    QFETCH(int, minorV);
    QFETCH(int, rule);

    QServiceFilter filter;
    QCOMPARE(filter.majorVersion(), -1);
    QCOMPARE(filter.minorVersion(), -1);
    QCOMPARE(filter.versionMatchRule(), QServiceFilter::MinimumVersionMatch);

    filter.setInterface("com.nokia.qt.test",version, (QServiceFilter::VersionMatchRule)rule);
    QCOMPARE(filter.majorVersion(), majorV);
    QCOMPARE(filter.minorVersion(), minorV);
    QCOMPARE(filter.versionMatchRule(), (QServiceFilter::VersionMatchRule)rule);
}


void tst_QServiceFilter::setInterface()
{
    //don't separate this out into test_data() function as we test the behavior
    //on the same filter object
    QServiceFilter filter;
    QCOMPARE(filter.majorVersion(), -1);
    QCOMPARE(filter.minorVersion(), -1);
    QCOMPARE(filter.versionMatchRule(), QServiceFilter::MinimumVersionMatch);
    QCOMPARE(filter.serviceName(), QString());
    QCOMPARE(filter.interfaceName(), QString());

    filter.setInterface("com.nokia.qt.text", "1.0", QServiceFilter::ExactVersionMatch);

    QCOMPARE(filter.majorVersion(), 1);
    QCOMPARE(filter.minorVersion(), 0);
    QCOMPARE(filter.versionMatchRule(), QServiceFilter::ExactVersionMatch);
    QCOMPARE(filter.serviceName(), QString());
    QCOMPARE(filter.interfaceName(), QString("com.nokia.qt.text"));

    filter.setInterface("com.nokia.qt.text", "1.5", QServiceFilter::MinimumVersionMatch);

    QCOMPARE(filter.majorVersion(), 1);
    QCOMPARE(filter.minorVersion(), 5);
    QCOMPARE(filter.versionMatchRule(), QServiceFilter::MinimumVersionMatch);
    QCOMPARE(filter.serviceName(), QString());
    QCOMPARE(filter.interfaceName(), QString("com.nokia.qt.text"));

    //invalid version tag -> ignore the call
    filter.setInterface("com.nokia.qt.label", "f.0", QServiceFilter::ExactVersionMatch);

    QCOMPARE(filter.majorVersion(), 1);
    QCOMPARE(filter.minorVersion(), 5);
    QCOMPARE(filter.versionMatchRule(), QServiceFilter::MinimumVersionMatch); //default
    QCOMPARE(filter.serviceName(), QString());
    QCOMPARE(filter.interfaceName(), QString("com.nokia.qt.text"));

    //empty version tag -> reset version
    filter.setInterface("com.nokia.qt.label", "", QServiceFilter::ExactVersionMatch);

    QCOMPARE(filter.majorVersion(), -1);
    QCOMPARE(filter.minorVersion(), -1);
    QCOMPARE(filter.versionMatchRule(), QServiceFilter::ExactVersionMatch); //default
    QCOMPARE(filter.serviceName(), QString());
    QCOMPARE(filter.interfaceName(), QString("com.nokia.qt.label"));

    //empty.interfaceName() tag -> ignore the call
    filter.setInterface("", "4.5", QServiceFilter::MinimumVersionMatch);

    QCOMPARE(filter.majorVersion(), -1);
    QCOMPARE(filter.minorVersion(), -1);
    QCOMPARE(filter.versionMatchRule(), QServiceFilter::ExactVersionMatch); //default
    QCOMPARE(filter.serviceName(), QString());
    QCOMPARE(filter.interfaceName(), QString("com.nokia.qt.label"));

    //set a valid
    filter.setInterface("com.nokia.qt.valid", "4.77", QServiceFilter::ExactVersionMatch);

    QCOMPARE(filter.majorVersion(), 4);
    QCOMPARE(filter.minorVersion(), 77);
    QCOMPARE(filter.versionMatchRule(), QServiceFilter::ExactVersionMatch);
    QCOMPARE(filter.serviceName(), QString());
    QCOMPARE(filter.interfaceName(), QString("com.nokia.qt.valid"));

    filter.setServiceName("myService");
    QCOMPARE(filter.majorVersion(), 4);
    QCOMPARE(filter.minorVersion(), 77);
    QCOMPARE(filter.versionMatchRule(), QServiceFilter::ExactVersionMatch);
    QCOMPARE(filter.serviceName(), QString("myService"));
    QCOMPARE(filter.interfaceName(), QString("com.nokia.qt.valid"));

    //test default constructed version and matching rule
    filter.setInterface("com.nokia.qt.valid2");
    QCOMPARE(filter.majorVersion(), -1);
    QCOMPARE(filter.minorVersion(), -1);
    QCOMPARE(filter.versionMatchRule(), QServiceFilter::MinimumVersionMatch);
    QCOMPARE(filter.serviceName(), QString("myService"));
    QCOMPARE(filter.interfaceName(), QString("com.nokia.qt.valid2"));
}

void tst_QServiceFilter::testAssignmentOperator()
{
    QServiceFilter emptyFilter;
    QServiceFilter tempFilter(emptyFilter);

    //assign empty filter to empty filter
    QCOMPARE(emptyFilter.majorVersion(), -1);
    QCOMPARE(emptyFilter.minorVersion(), -1);
    QCOMPARE(emptyFilter.versionMatchRule(), QServiceFilter::MinimumVersionMatch);
    QCOMPARE(emptyFilter.serviceName(), QString(""));
    QCOMPARE(emptyFilter.interfaceName(), QString(""));
    QCOMPARE(emptyFilter.customAttribute("key1"), QString());
    QCOMPARE(emptyFilter.capabilities(), QStringList());
    QCOMPARE(emptyFilter.capabilityMatchRule(), QServiceFilter::MatchMinimum);

    QCOMPARE(tempFilter.majorVersion(), -1);
    QCOMPARE(tempFilter.minorVersion(), -1);
    QCOMPARE(tempFilter.versionMatchRule(), QServiceFilter::MinimumVersionMatch);
    QCOMPARE(tempFilter.serviceName(), QString(""));
    QCOMPARE(tempFilter.interfaceName(), QString(""));
    QCOMPARE(tempFilter.customAttribute("key1"), QString());
    QCOMPARE(tempFilter.capabilities(), QStringList());
    QCOMPARE(tempFilter.capabilityMatchRule(), QServiceFilter::MatchMinimum);

    tempFilter = emptyFilter;

    QCOMPARE(tempFilter.majorVersion(), -1);
    QCOMPARE(tempFilter.minorVersion(), -1);
    QCOMPARE(tempFilter.versionMatchRule(), QServiceFilter::MinimumVersionMatch);
    QCOMPARE(tempFilter.serviceName(), QString(""));
    QCOMPARE(tempFilter.interfaceName(), QString(""));
    QCOMPARE(tempFilter.customAttribute("key1"), QString());
    QCOMPARE(tempFilter.capabilities(), QStringList());
    QCOMPARE(tempFilter.capabilityMatchRule(), QServiceFilter::MatchMinimum);

    //assign filter to new filter via constructor
    tempFilter.setInterface("com.nokia.qt.valid", "4.77", QServiceFilter::ExactVersionMatch);
    tempFilter.setServiceName("ServiceName");
    tempFilter.setCustomAttribute("key1", "value1");
    tempFilter.setCapabilities(QServiceFilter::MatchLoadable, QStringList() << "read" << "write");
    QCOMPARE(tempFilter.majorVersion(), 4);
    QCOMPARE(tempFilter.minorVersion(), 77);
    QCOMPARE(tempFilter.versionMatchRule(), QServiceFilter::ExactVersionMatch);
    QCOMPARE(tempFilter.serviceName(), QString("ServiceName"));
    QCOMPARE(tempFilter.interfaceName(), QString("com.nokia.qt.valid"));
    QCOMPARE(tempFilter.customAttribute("key1"), QString("value1"));
    QCOMPARE(tempFilter.capabilities(), (QStringList() << "read" << "write"));
    QCOMPARE(tempFilter.capabilityMatchRule(), QServiceFilter::MatchLoadable);

    QServiceFilter constructFilter(tempFilter);
    QCOMPARE(constructFilter.majorVersion(), 4);
    QCOMPARE(constructFilter.minorVersion(), 77);
    QCOMPARE(constructFilter.versionMatchRule(), QServiceFilter::ExactVersionMatch);
    QCOMPARE(constructFilter.serviceName(), QString("ServiceName"));
    QCOMPARE(constructFilter.interfaceName(), QString("com.nokia.qt.valid"));
    QCOMPARE(constructFilter.customAttribute("key1"), QString("value1"));
    QCOMPARE(constructFilter.capabilities(), (QStringList() << "read" << "write"));
    QCOMPARE(constructFilter.capabilityMatchRule(), QServiceFilter::MatchLoadable);

    //ensure that we don't have any potential references between tempFilter and
    //constructedFilter
    tempFilter.setServiceName("NewServiceName");
    tempFilter.setInterface("com.nokia.qt.valid2", "5.88", QServiceFilter::MinimumVersionMatch);
    tempFilter.setCustomAttribute("key2", "value2");
    tempFilter.setCapabilities(QServiceFilter::MatchMinimum,QStringList() << "execute");
    QCOMPARE(tempFilter.majorVersion(), 5);
    QCOMPARE(tempFilter.minorVersion(), 88);
    QCOMPARE(tempFilter.versionMatchRule(), QServiceFilter::MinimumVersionMatch);
    QCOMPARE(tempFilter.serviceName(), QString("NewServiceName"));
    QCOMPARE(tempFilter.interfaceName(), QString("com.nokia.qt.valid2"));
    QCOMPARE(tempFilter.customAttribute("key1"), QString("value1"));
    QCOMPARE(tempFilter.customAttribute("key2"), QString("value2"));
    QCOMPARE(tempFilter.capabilities(), (QStringList() << "execute"));
    QCOMPARE(tempFilter.capabilityMatchRule(), QServiceFilter::MatchMinimum);
    QCOMPARE(constructFilter.majorVersion(), 4);
    QCOMPARE(constructFilter.minorVersion(), 77);
    QCOMPARE(constructFilter.versionMatchRule(), QServiceFilter::ExactVersionMatch);
    QCOMPARE(constructFilter.serviceName(), QString("ServiceName"));
    QCOMPARE(constructFilter.interfaceName(), QString("com.nokia.qt.valid"));
    QCOMPARE(constructFilter.customAttribute("key1"), QString("value1"));
    QCOMPARE(constructFilter.customAttribute("key2"), QString());
    QCOMPARE(constructFilter.capabilities(), (QStringList() << "read" << "write"));
    QCOMPARE(constructFilter.capabilityMatchRule(), QServiceFilter::MatchLoadable);

    //assign empty filter to filter with values
    constructFilter = emptyFilter;
    QCOMPARE(constructFilter.majorVersion(), -1);
    QCOMPARE(constructFilter.minorVersion(), -1);
    QCOMPARE(constructFilter.versionMatchRule(), QServiceFilter::MinimumVersionMatch);
    QCOMPARE(constructFilter.serviceName(), QString(""));
    QCOMPARE(constructFilter.interfaceName(), QString(""));
    QCOMPARE(constructFilter.customAttribute("key1"), QString());
    QCOMPARE(constructFilter.customAttribute("key2"), QString());
    QCOMPARE(constructFilter.capabilities(), QStringList());
    QCOMPARE(constructFilter.capabilityMatchRule(), QServiceFilter::MatchMinimum);
}

void tst_QServiceFilter::testConstructor()
{
    QServiceFilter tempFilter1("");
    QCOMPARE(tempFilter1.majorVersion(), -1);
    QCOMPARE(tempFilter1.minorVersion(), -1);
    QCOMPARE(tempFilter1.versionMatchRule(), QServiceFilter::MinimumVersionMatch);
    QCOMPARE(tempFilter1.serviceName(), QString());
    QCOMPARE(tempFilter1.interfaceName(), QString(""));
    QCOMPARE(tempFilter1.customAttribute("key1"), QString());

    QServiceFilter tempFilter2("com.nokia.qt.test");
    QCOMPARE(tempFilter2.majorVersion(), -1);
    QCOMPARE(tempFilter2.minorVersion(), -1);
    QCOMPARE(tempFilter2.versionMatchRule(), QServiceFilter::MinimumVersionMatch);
    QCOMPARE(tempFilter2.serviceName(), QString());
    QCOMPARE(tempFilter2.interfaceName(), QString("com.nokia.qt.test"));
    QCOMPARE(tempFilter2.customAttribute("key1"), QString());

    QServiceFilter tempFilter3("com.nokia.qt.test", "10.5");
    QCOMPARE(tempFilter3.majorVersion(), 10);
    QCOMPARE(tempFilter3.minorVersion(), 5);
    QCOMPARE(tempFilter3.versionMatchRule(), QServiceFilter::MinimumVersionMatch);
    QCOMPARE(tempFilter3.serviceName(), QString());
    QCOMPARE(tempFilter3.interfaceName(), QString("com.nokia.qt.test"));
    QCOMPARE(tempFilter3.customAttribute("key1"), QString());

    QServiceFilter tempFilter4("com.nokia.qt.test", "11.7", QServiceFilter::ExactVersionMatch);
    QCOMPARE(tempFilter4.majorVersion(), 11);
    QCOMPARE(tempFilter4.minorVersion(), 7);
    QCOMPARE(tempFilter4.versionMatchRule(), QServiceFilter::ExactVersionMatch);
    QCOMPARE(tempFilter4.serviceName(), QString());
    QCOMPARE(tempFilter4.interfaceName(), QString("com.nokia.qt.test"));
    QCOMPARE(tempFilter4.customAttribute("key1"), QString());

}

#ifndef QT_NO_DATASTREAM
void tst_QServiceFilter::streamTest()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::ReadWrite);
    QDataStream stream(&buffer);

    QServiceFilter emptyFilter;
    QCOMPARE(emptyFilter.majorVersion(), -1);
    QCOMPARE(emptyFilter.minorVersion(), -1);
    QCOMPARE(emptyFilter.versionMatchRule(), QServiceFilter::MinimumVersionMatch);
    QCOMPARE(emptyFilter.serviceName(), QString(""));
    QCOMPARE(emptyFilter.interfaceName(), QString(""));
    QCOMPARE(emptyFilter.customAttribute("key1"), QString());
    QCOMPARE(emptyFilter.capabilities(), QStringList());
    QCOMPARE(emptyFilter.capabilityMatchRule(), QServiceFilter::MatchMinimum);

    buffer.seek(0);
    stream << emptyFilter;

    QServiceFilter tempFilter;
    QCOMPARE(tempFilter.majorVersion(), -1);
    QCOMPARE(tempFilter.minorVersion(), -1);
    QCOMPARE(tempFilter.versionMatchRule(), QServiceFilter::MinimumVersionMatch);
    QCOMPARE(tempFilter.serviceName(), QString(""));
    QCOMPARE(tempFilter.interfaceName(), QString(""));
    QCOMPARE(tempFilter.customAttribute("key1"), QString());
    QCOMPARE(tempFilter.capabilities(), QStringList());
    QCOMPARE(tempFilter.capabilityMatchRule(), QServiceFilter::MatchMinimum);

    buffer.seek(0);
    stream >> tempFilter;

    QCOMPARE(tempFilter.majorVersion(), -1);
    QCOMPARE(tempFilter.minorVersion(), -1);
    QCOMPARE(tempFilter.versionMatchRule(), QServiceFilter::MinimumVersionMatch);
    QCOMPARE(tempFilter.serviceName(), QString(""));
    QCOMPARE(tempFilter.interfaceName(), QString(""));
    QCOMPARE(tempFilter.customAttribute("key1"), QString());
    QCOMPARE(tempFilter.capabilities(), QStringList());
    QCOMPARE(tempFilter.capabilityMatchRule(), QServiceFilter::MatchMinimum);

    //assign filter to new filter via constructor
    tempFilter.setInterface("com.nokia.qt.valid", "4.77", QServiceFilter::ExactVersionMatch);
    tempFilter.setServiceName("ServiceName");
    tempFilter.setCustomAttribute("key1", "value1");
    tempFilter.setCapabilities(QServiceFilter::MatchLoadable, QStringList() << "execute" << "delete");
    QCOMPARE(tempFilter.majorVersion(), 4);
    QCOMPARE(tempFilter.minorVersion(), 77);
    QCOMPARE(tempFilter.versionMatchRule(), QServiceFilter::ExactVersionMatch);
    QCOMPARE(tempFilter.serviceName(), QString("ServiceName"));
    QCOMPARE(tempFilter.interfaceName(), QString("com.nokia.qt.valid"));
    QCOMPARE(tempFilter.customAttribute("key1"), QString("value1"));
    QCOMPARE(tempFilter.capabilities(), (QStringList()<<"execute" << "delete"));
    QCOMPARE(tempFilter.capabilityMatchRule(), QServiceFilter::MatchLoadable);
    buffer.seek(0);
    stream << tempFilter;

    QServiceFilter constructFilter;
    buffer.seek(0);
    stream >> constructFilter;
    QCOMPARE(constructFilter.majorVersion(), 4);
    QCOMPARE(constructFilter.minorVersion(), 77);
    QCOMPARE(constructFilter.versionMatchRule(), QServiceFilter::ExactVersionMatch);
    QCOMPARE(constructFilter.serviceName(), QString("ServiceName"));
    QCOMPARE(constructFilter.interfaceName(), QString("com.nokia.qt.valid"));
    QCOMPARE(constructFilter.customAttribute("key1"), QString("value1"));
    QCOMPARE(constructFilter.capabilities(), (QStringList()<<"execute" << "delete"));
    QCOMPARE(constructFilter.capabilityMatchRule(), QServiceFilter::MatchLoadable);

    //assign empty filter to filter with values

    buffer.seek(0);
    stream << emptyFilter;
    buffer.seek(0);
    stream >> constructFilter;
    QCOMPARE(constructFilter.majorVersion(), -1);
    QCOMPARE(constructFilter.minorVersion(), -1);
    QCOMPARE(constructFilter.versionMatchRule(), QServiceFilter::MinimumVersionMatch);
    QCOMPARE(constructFilter.serviceName(), QString(""));
    QCOMPARE(constructFilter.interfaceName(), QString(""));
    QCOMPARE(constructFilter.customAttribute("key1"), QString());
    QCOMPARE(constructFilter.capabilities(), QStringList());
    QCOMPARE(constructFilter.capabilityMatchRule(), QServiceFilter::MatchMinimum);
}
#endif

void tst_QServiceFilter::testCustomAttribute()
{
    //default constructor
    QServiceFilter emptyFilter;
    QCOMPARE(emptyFilter.customAttribute("key1"), QString());

    QServiceFilter filter("com.nokia.qt.testinterface", "4.5");
    QCOMPARE(filter.customAttribute("key1"), QString());
    filter.setCustomAttribute("key1", "newValue");
    QCOMPARE(filter.customAttribute("key1"), QString("newValue"));
    filter.setCustomAttribute("key1", "revisedValue");
    QCOMPARE(filter.customAttribute("key1"), QString("revisedValue"));
    filter.setCustomAttribute("key2", "Value");
    QCOMPARE(filter.customAttribute("key1"), QString("revisedValue"));
    QCOMPARE(filter.customAttribute("key2"), QString("Value"));
    filter.setCustomAttribute("key1", QString());
    QCOMPARE(filter.customAttribute("key1"), QString());
    QCOMPARE(filter.customAttribute("key2"), QString("Value"));
}

void tst_QServiceFilter::testCapabilities()
{
    QServiceFilter filter;
    QCOMPARE(filter.capabilities(), QStringList());
    QCOMPARE(filter.capabilityMatchRule(), QServiceFilter::MatchMinimum);

    filter.setCapabilities(QServiceFilter::MatchMinimum, QStringList() << "execute");
    QCOMPARE(filter.capabilities(), (QStringList() << "execute"));
    QCOMPARE(filter.capabilityMatchRule(), QServiceFilter::MatchMinimum);

    filter.setCapabilities(QServiceFilter::MatchMinimum, QStringList() << "execute" << "read");
    QCOMPARE(filter.capabilities(), (QStringList() << "execute" << "read"));
    QCOMPARE(filter.capabilityMatchRule(), QServiceFilter::MatchMinimum);

    filter.setCapabilities(QServiceFilter::MatchMinimum);
    QCOMPARE(filter.capabilities(), QStringList());
    QCOMPARE(filter.capabilityMatchRule(), QServiceFilter::MatchMinimum);

    filter.setCapabilities(QServiceFilter::MatchLoadable, QStringList() << "execute" << "read");
    QCOMPARE(filter.capabilities(), (QStringList() << "execute" << "read"));
    QCOMPARE(filter.capabilityMatchRule(), QServiceFilter::MatchLoadable);

    filter.setCapabilities(QServiceFilter::MatchLoadable);
    QCOMPARE(filter.capabilities(), QStringList());
    QCOMPARE(filter.capabilityMatchRule(), QServiceFilter::MatchLoadable);
}

void tst_QServiceFilter::cleanupTestCase()
{
}

QTEST_MAIN(tst_QServiceFilter)
#include "tst_qservicefilter.moc"
