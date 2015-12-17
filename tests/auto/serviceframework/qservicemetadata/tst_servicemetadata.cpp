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
#define private public
#define SERVICE_XML_GENERATOR
#include <qserviceinterfacedescriptor.h>
#include <private/qserviceinterfacedescriptor_p.h>
#if !defined(Q_CC_MINGW)
    #include "../../../../src/serviceframework/qserviceinterfacedescriptor.cpp"
#endif
#define IGNORE_SERVICEMETADATA_EXPORT
#include "private/servicemetadata_p.h"
#include "../../../../src/serviceframework/servicemetadata.cpp"

QT_USE_NAMESPACE
class ServiceMetadataTest: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void parseInvalidServiceXML_oldXml();
    void parseValidServiceXML();
    void parseInvalidServiceXML_data();
    void parseInvalidServiceXML();
    void noCapability();
    void checkVersion_data();
    void checkVersion();
    void latestInterfaceVersion_data();
    void latestInterfaceVersion();
    void cleanupTestCase();

private:
    QDir dir;

};
void ServiceMetadataTest::initTestCase()
{
    const QString directory = QFINDTESTDATA("testdata");
    QVERIFY2(!directory.isEmpty(), "Unable to locate 'testdata'.");
    dir = QDir(directory);
}

void ServiceMetadataTest::cleanupTestCase()
{
}

void ServiceMetadataTest::parseInvalidServiceXML_oldXml()
{
    ServiceMetaData parser(dir.absoluteFilePath("ServiceTestOld.xml"));
    QCOMPARE(parser.extractMetadata(), false);
}

void ServiceMetadataTest::parseValidServiceXML()
{
    ServiceMetaData parser(dir.absoluteFilePath("ServiceTest.xml"));
    QCOMPARE(parser.extractMetadata(),true);
    const ServiceMetaDataResults data = parser.parseResults();
    QCOMPARE(data.name, QString("TestService"));
    QCOMPARE(data.description, QString("Test service description"));
    QCOMPARE(data.location, QString("C:/TestData/testservice.dll"));

    ServiceMetaData parserIPC(dir.absoluteFilePath("ServiceTestIPC.xml"));
    QCOMPARE(parserIPC.extractMetadata(),true);
    const ServiceMetaDataResults dataIPC = parserIPC.parseResults();
    QCOMPARE(dataIPC.name, QString("TestIPCService"));
    QCOMPARE(dataIPC.description, QString("Test IPC service description"));
    QCOMPARE(dataIPC.location, QString("test_service"));

    QList<QServiceInterfaceDescriptor> allInterfaces = data.interfaces;
    allInterfaces.append(dataIPC.interfaces);

    QServiceInterfaceDescriptor aInterface = allInterfaces.at(0);
    QCOMPARE(aInterface.interfaceName(),QString("com.nokia.qt.tests.IDownloader"));
    QCOMPARE(aInterface.majorVersion(), 1);
    QCOMPARE(aInterface.minorVersion(), 4);
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList().count() == 0);
    QCOMPARE(aInterface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString(), QString("Interface that provides download support"));
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::ServiceType) ==  QService::Plugin);

    aInterface = allInterfaces.at(1);
    QCOMPARE(aInterface.interfaceName(),QString("com.nokia.qt.tests.ILocation"));
    QCOMPARE(aInterface.majorVersion(), 1);
    QCOMPARE(aInterface.minorVersion(), 4);
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList().count() == 0);
    QCOMPARE(aInterface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString(), QString("Interface that provides location support"));
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::ServiceType) ==  QService::Plugin);

    aInterface = allInterfaces.at(2);
    QCOMPARE(aInterface.interfaceName(),QString("com.nokia.qt.tests.ISysInfo"));
    QCOMPARE(aInterface.majorVersion(), 2);
    QCOMPARE(aInterface.minorVersion(), 3);
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList().count() == 1);
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList().contains("ReadUserData"));
    QCOMPARE(aInterface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString(), QString("Interface that provides system information support"));
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::ServiceType) ==  QService::Plugin);

    aInterface = allInterfaces.at(3);
    QCOMPARE(aInterface.interfaceName(),QString("com.nokia.qt.tests.ISendMessage"));
    QCOMPARE(aInterface.majorVersion(), 3);
    QCOMPARE(aInterface.minorVersion(), 0);
    QStringList capabilities = aInterface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList();
    QVERIFY(capabilities.count() == 2);
    QVERIFY(capabilities.contains("ReadUserData"));
    QVERIFY(capabilities.contains("WriteUserData"));
    QCOMPARE(aInterface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString(), QString("Interface that provides message sending support"));
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::ServiceType) ==  QService::Plugin);

    aInterface = allInterfaces.at(4);
    QCOMPARE(aInterface.interfaceName(), QString("com.nokia.qt.tests.IReceiveMessage"));
    QCOMPARE(aInterface.majorVersion(), 1);
    QCOMPARE(aInterface.minorVersion(), 1);
    capabilities = aInterface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList();
    QVERIFY(capabilities.count() == 3);
    QVERIFY(capabilities.contains("ReadUserData"));
    QVERIFY(capabilities.contains("WriteUserData"));
    QVERIFY(capabilities.contains("ExecUserData"));
    QCOMPARE(aInterface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString(), QString("Interface that provides message receiving support"));
    QCOMPARE(aInterface.customAttribute("key1"), QString("value1"));
    QCOMPARE(aInterface.customAttribute("key2"), QString("value2"));
    QCOMPARE(aInterface.customAttribute("key3"), QString(""));
    QCOMPARE(aInterface.customAttribute("key4"), QString());
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::ServiceType) ==  QService::Plugin);

    aInterface = allInterfaces.at(5);
    QCOMPARE(aInterface.interfaceName(),QString("com.nokia.qt.tests.IDownloader"));
    QCOMPARE(aInterface.majorVersion(), 1);
    QCOMPARE(aInterface.minorVersion(), 4);
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList().count() == 0);
    QCOMPARE(aInterface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString(), QString("Interface that provides download support"));
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::ServiceType) ==  QService::InterProcess);

    aInterface = allInterfaces.at(6);
    QCOMPARE(aInterface.interfaceName(),QString("com.nokia.qt.tests.ILocation"));
    QCOMPARE(aInterface.majorVersion(), 1);
    QCOMPARE(aInterface.minorVersion(), 4);
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList().count() == 0);
    QCOMPARE(aInterface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString(), QString("Interface that provides location support"));
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::ServiceType) ==  QService::InterProcess);

    aInterface = allInterfaces.at(7);
    QCOMPARE(aInterface.interfaceName(),QString("com.nokia.qt.tests.ISysInfo"));
    QCOMPARE(aInterface.majorVersion(), 2);
    QCOMPARE(aInterface.minorVersion(), 3);
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList().count() == 1);
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList().contains("ReadUserData"));
    QCOMPARE(aInterface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString(), QString("Interface that provides system information support"));
    QVERIFY(aInterface.attribute(QServiceInterfaceDescriptor::ServiceType) ==  QService::InterProcess);

    ServiceMetaData parser1(dir.absoluteFilePath("WrongOrder.xml"));
    QCOMPARE(parser1.extractMetadata(),true);
    QList<QServiceInterfaceDescriptor> allInterfacesWrongOrder = parser1.parseResults().interfaces;
    foreach(const QServiceInterfaceDescriptor d, allInterfacesWrongOrder) {
        QCOMPARE(d.serviceName(), QString("ovi"));
        QCOMPARE(d.attribute(QServiceInterfaceDescriptor::Location).toString(), QString("C:/Nokia/ovi.dll"));
        QCOMPARE(d.attribute(QServiceInterfaceDescriptor::ServiceDescription).toString(), QString("Ovi Services"));
    }
}

void ServiceMetadataTest::parseInvalidServiceXML_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<int>("expectedError");

    QTest::newRow("no such file") << "!#@!CSC" << (int)ServiceMetaData::SFW_ERROR_UNABLE_TO_OPEN_FILE;

    QTest::newRow("Test1.xml") << "Test1.xml" << (int)ServiceMetaData::SFW_ERROR_INVALID_XML_FILE;
    QTest::newRow("Test2.xml") << "Test2.xml" << (int)ServiceMetaData::SFW_ERROR_NO_SERVICE;
    QTest::newRow("Test3.xml") << "Test3.xml" << (int)ServiceMetaData::SFW_ERROR_PARSE_SERVICE;
    QTest::newRow("Test4.xml") << "Test4.xml" << (int)ServiceMetaData::SFW_ERROR_NO_INTERFACE_VERSION;
    QTest::newRow("Test5.xml") << "Test5.xml" << (int)ServiceMetaData::SFW_ERROR_NO_SERVICE_PATH;
    QTest::newRow("Test7.xml") << "Test7.xml" << (int)ServiceMetaData::SFW_ERROR_PARSE_SERVICE;
    QTest::newRow("Test8.xml") << "Test8.xml" << (int)ServiceMetaData::SFW_ERROR_PARSE_INTERFACE;
    QTest::newRow("Test9.xml") << "Test9.xml" << (int)ServiceMetaData::SFW_ERROR_PARSE_SERVICE; ///check error code
    QTest::newRow("Test10.xml") << "Test10.xml" << (int)ServiceMetaData::SFW_ERROR_NO_SERVICE_INTERFACE;
    QTest::newRow("Test11.xml") << "Test11.xml" << (int)ServiceMetaData::SFW_ERROR_NO_INTERFACE_NAME;
    QTest::newRow("Test12.xml") << "Test12.xml" << (int)ServiceMetaData::SFW_ERROR_NO_SERVICE_PATH;
    //QTest::newRow("Test8.xml") << "Test8.xml" << (int)ServiceMetaData::SFW_ERROR_NO_INTERFACE_NAME;

    QStringList badVersionXml;
    badVersionXml << "Test14.xml" << "Test15.xml" << "Test17.xml" << "Test18.xml";
    for (int i=0; i<badVersionXml.count(); i++)
        QTest::newRow(qPrintable(badVersionXml[i])) << badVersionXml[i] << (int)ServiceMetaData::SFW_ERROR_INVALID_VERSION;

    QTest::newRow("empty interface name") << "emptyInterfaceName.xml" << (int)ServiceMetaData::SFW_ERROR_NO_INTERFACE_NAME;
    QTest::newRow("empty service name") << "emptyServiceName.xml" << (int)ServiceMetaData::SFW_ERROR_NO_SERVICE_NAME;
    QTest::newRow("empty version") << "emptyVersion.xml" << (int)ServiceMetaData::SFW_ERROR_NO_INTERFACE_VERSION;
    QTest::newRow("duplicated interface A") << "Test13.xml" << (int)ServiceMetaData::SFW_ERROR_DUPLICATED_INTERFACE;
    QTest::newRow("duplicated interface B") << "Test19.xml" << (int)ServiceMetaData::SFW_ERROR_DUPLICATED_INTERFACE;
    QTest::newRow("duplicated service tag") << "DuplicatedServiceTag.xml" << (int)ServiceMetaData::SFW_ERROR_INVALID_XML_FILE;

    QTest::newRow("Test20.xml") << "Test20.xml" << (int)ServiceMetaData::SFW_ERROR_NO_SERVICE_NAME;
    QTest::newRow("Test21.xml") << "Test21.xml" << (int)ServiceMetaData::SFW_ERROR_NO_SERVICE_INTERFACE;
    QTest::newRow("Test22.xml") << "Test22.xml" << (int)ServiceMetaData::SFW_ERROR_NO_INTERFACE_NAME;
    //duplicated service name tag
    QTest::newRow("Test23.xml") << "Test23.xml" << (int)ServiceMetaData::SFW_ERROR_DUPLICATED_TAG;
    //duplicated service description tag
    QTest::newRow("Test24.xml") << "Test24.xml" << (int)ServiceMetaData::SFW_ERROR_DUPLICATED_TAG;
    //duplicated service filepath tag
    QTest::newRow("Test25.xml") << "Test25.xml" << (int)ServiceMetaData::SFW_ERROR_DUPLICATED_TAG;
    //duplicated interface name tag
    QTest::newRow("Test26.xml") << "Test26.xml" << (int)ServiceMetaData::SFW_ERROR_DUPLICATED_TAG;
    //duplicated interface version tag
    QTest::newRow("Test27.xml") << "Test27.xml" << (int)ServiceMetaData::SFW_ERROR_DUPLICATED_TAG;
    //duplicated interface description tag
    QTest::newRow("Test28.xml") << "Test28.xml" << (int)ServiceMetaData::SFW_ERROR_DUPLICATED_TAG;
    //duplicated interface capabilities tag
    QTest::newRow("Test29.xml") << "Test29.xml" << (int)ServiceMetaData::SFW_ERROR_DUPLICATED_TAG;
    //missing key attribute for custom property tag
    QTest::newRow("Test30.xml") << "Test30.xml" << (int)ServiceMetaData::SFW_ERROR_INVALID_CUSTOM_TAG;
    //empty key attribute for custom property tag
    QTest::newRow("Test31.xml") << "Test31.xml" << (int)ServiceMetaData::SFW_ERROR_INVALID_CUSTOM_TAG;
    //same key exists twice
    QTest::newRow("Test32.xml") << "Test32.xml" << (int)ServiceMetaData::SFW_ERROR_DUPLICATED_CUSTOM_KEY;
    //multiple definitions of the service type
    QTest::newRow("Test33.xml") << "Test33.xml" << (int)ServiceMetaData::SFW_ERROR_MULTIPLE_SERVICE_TYPES;
    //invalid filepath from ipc service type prefix
    QTest::newRow("Test34.xml") << "Test34.xml" << (int)ServiceMetaData::SFW_ERROR_INVALID_FILEPATH;
    //invalid version in service version tag
    QTest::newRow("Test35.xml") << "Test35.xml" << (int)ServiceMetaData::SFW_ERROR_INVALID_XML_VERSION;
    //invalid missing version attribute in service version tag
    QTest::newRow("Test36.xml") << "Test36.xml" << (int)ServiceMetaData::SFW_ERROR_INVALID_XML_VERSION;
    //unsupported version attribute in service version tag
    QTest::newRow("Test37.xml") << "Test37.xml" << (int)ServiceMetaData::SFW_ERROR_UNSUPPORTED_XML_VERSION;
    //unsupported version attribute in service version tag
    QTest::newRow("Test38.xml") << "Test38.xml" << (int)ServiceMetaData::SFW_ERROR_UNSUPPORTED_IPC;
}

void ServiceMetadataTest::parseInvalidServiceXML()
{
    QFETCH(QString, fileName);
    QFETCH(int, expectedError);

    ServiceMetaData parser(dir.absoluteFilePath(fileName));
    QVERIFY(!parser.extractMetadata());
    QCOMPARE(parser.getLatestError(), expectedError);
}

void ServiceMetadataTest::noCapability()
{
    ServiceMetaData parser(dir.absoluteFilePath("Test6.xml"));
    QVERIFY(parser.extractMetadata());
}

void ServiceMetadataTest::checkVersion_data()
{
    QTest::addColumn<QString>("version");
    QTest::addColumn<bool>("result");
    QTest::addColumn<int>("major");
    QTest::addColumn<int>("minor");

    QTest::newRow("checkVersion_data():Invalid 1") << "" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 2") << "0.3" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 3") << "01.3" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 4") << "1.03" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 5") << "x.y" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 6") << "23" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 7") << "sdfsfs" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 8") << "%#5346" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 9") << ".66" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 10") << "1.3.4" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 11") << "1.a" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 12") << "b.1" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 13") << "3." << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 14") << "-1" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 15") << "0.0" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 16") << ".x" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 17") << "x." << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 18") << "1. 0" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 19") << "1  .0" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 20") << "1  0" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 21") << "1 . 0" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 22") << " 1.5" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 23") << "1.5 " << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 24") << " 1.5 " << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 25") << "1.5 1.6" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 26") << "-1.0" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 27") << "1.-1" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 28") << "-5.-1" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 29") << "4,8" << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 30") << "  " << false << -1 << -1;
    QTest::newRow("checkVersion_data():Invalid 31") << "1.9b" << false << -1 << -1;

    QTest::newRow("checkVersion_data():Valid 1") << "1.0" << true << 1 << 0;
    QTest::newRow("checkVersion_data():Valid 2") << "1.00" << true << 1 << 0;
    QTest::newRow("checkVersion_data():Valid 3") << "99.99" << true << 99 << 99;
    QTest::newRow("checkVersion_data():Valid 4") << "2.3" << true << 2 << 3;
    QTest::newRow("checkVersion_data():Valid 5") << "10.3" << true << 10 << 3;
    QTest::newRow("checkVersion_data():Valid 6") << "5.10" << true << 5 << 10;
    QTest::newRow("checkVersion_data():Valid 7") << "10.10" << true << 10 << 10;
}

void ServiceMetadataTest::checkVersion()
{
    QFETCH(QString, version);
    QFETCH(bool, result);
    QFETCH(int, major);
    QFETCH(int, minor);

    ServiceMetaData parser(dir.absoluteFilePath("ServiceTest.xml"));
    QCOMPARE(parser.checkVersion(version), result);
    int majorVer;
    int minorVer;
    parser.transformVersion(version, &majorVer, &minorVer);
    QCOMPARE(majorVer, major);
    QCOMPARE(minorVer, minor);
}

void ServiceMetadataTest::latestInterfaceVersion_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<int>("major");
    QTest::addColumn<int>("minor");

    //cases 1-3 contains services with more than one interface
    QTest::newRow("latestVersion_data() 1") << "latestVersion.xml" << 2 << 1; //latest version in middle
    QTest::newRow("latestVersion_data() 2") << "latestVersion2.xml" << 3 << 0; //latest version at end
    QTest::newRow("latestVersion_data() 3") << "latestVersion3.xml" << 5 << 7; //latest version at start
    QTest::newRow("latestVersion_data() 4") << "latestVersion4.xml" << 1 << 7; //only one version

}

void ServiceMetadataTest::latestInterfaceVersion()
{
    QFETCH(QString, fileName);
    QFETCH(int, major);
    QFETCH(int, minor);

    ServiceMetaData parser(dir.absoluteFilePath(fileName));
    QCOMPARE(parser.extractMetadata(), true);
    QServiceInterfaceDescriptor serviceInterface = parser.latestInterfaceVersion("com.nokia.service.contacts");
    QCOMPARE(serviceInterface.majorVersion(), major);
    QCOMPARE(serviceInterface.minorVersion(), minor);
    QCOMPARE(serviceInterface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString(),
            QString("Contacts management service")); //make sure we're getting the right interface
}

QTEST_MAIN(ServiceMetadataTest)
#include "tst_servicemetadata.moc"
