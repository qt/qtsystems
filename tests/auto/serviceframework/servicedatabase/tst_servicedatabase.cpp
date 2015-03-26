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
#include <qserviceinterfacedescriptor.h>
#include <private/qserviceinterfacedescriptor_p.h>
#include <private/servicedatabase_p.h>
#include <qservicefilter.h>

#define RESOLVERDATABASE "services.db"

QT_USE_NAMESPACE

class ServiceDatabaseUnitTest: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testRegistration();
    void getInterfaces();
    void searchByInterfaceName();
    void searchByInterfaceAndService();
    void searchByCustomAttribute();
    void searchByCapability();
    void attributes();
    void getServiceNames();
    void defaultExternalIfaceIDs();
    void interfaceDefault();
    void setInterfaceDefault();
    void unregister();
    void securityTokens();
    void cleanupTestCase();

private:

     bool compareDescriptor(QServiceInterfaceDescriptor interface,
                            QString interfaceName,
                            QString serviceName,
                            int majorVersion,
                            int minorVersion);

    bool compareDescriptor(QServiceInterfaceDescriptor interface,
                            QString interfaceName,
                            QString serviceName,
                            int majorVersion,
                            int minorVersion,
                            QStringList capabilities,
                            const QHash<QString,QString> customProps,
                            QString filePath="",
                            QString serviceDescription="",
                            QString interfaceDescription="");

    QStringList getInterfaceIDs(const QString &serviceName);
    QStringList getServiceIDs(const QString &serviceName);
    bool existsInInterfacePropertyTable(const QString &interfaceID);
    bool existsInServicePropertyTable(const QString &serviceID);
    bool existsInDefaultsTable(const QString &interfaceID);
    bool registerService(const ServiceMetaDataResults &service, const QString &securityToken = QString());
    bool unregisterService(const QString &serviceName, const QString &securityToken = QString());

    ServiceMetaData* parser;
    ServiceDatabase database;
};

void ServiceDatabaseUnitTest::initTestCase()
{
    database.close();
    QFile::remove(database.databasePath());
}

static const QString securityTokenOwner("SecurityTokenOwner");
static const QString securityTokenStranger("SecurityTokenStranger");

void ServiceDatabaseUnitTest::testRegistration()
{
    QDir testdir = QDir(QFINDTESTDATA("testdata"));

    ServiceMetaData parser(testdir.absoluteFilePath("ServiceAcme.xml"));
    QVERIFY(parser.extractMetadata());
    QVERIFY(!registerService(parser.parseResults()));

    QCOMPARE(database.lastError().code(), DBError::DatabaseNotOpen);
    QVERIFY(database.open());
    QVERIFY(registerService(parser.parseResults()));

    parser.setDevice(new QFile(testdir.absoluteFilePath("ServiceOmni.xml")));
    QVERIFY(parser.extractMetadata());
    QVERIFY(registerService(parser.parseResults()));

    parser.setDevice(new QFile(testdir.absoluteFilePath("ServiceLuthorCorp.xml")));
    QVERIFY(parser.extractMetadata());
    QVERIFY(registerService(parser.parseResults()));

    parser.setDevice(new QFile(testdir.absoluteFilePath("ServiceWayneEnt.xml")));
    QVERIFY(parser.extractMetadata());
    QVERIFY(registerService(parser.parseResults()));

    parser.setDevice(new QFile(testdir.absoluteFilePath("ServicePrimatech.xml")));
    QVERIFY(parser.extractMetadata());
    QVERIFY(registerService(parser.parseResults()));

    parser.setDevice(new QFile(testdir.absoluteFilePath("ServiceCyberdyne.xml")));
    QVERIFY(parser.extractMetadata());
    QVERIFY(registerService(parser.parseResults()));

    parser.setDevice(new QFile(testdir.absoluteFilePath("ServiceSkynet.xml")));
    QVERIFY(parser.extractMetadata());
    QVERIFY(registerService(parser.parseResults()));

    parser.setDevice(new QFile(testdir.absoluteFilePath("ServiceAutobot.xml")));
    QVERIFY(parser.extractMetadata());
    QVERIFY(registerService(parser.parseResults()));

    //try to register an already registered service
    QVERIFY(!registerService(parser.parseResults()));
    QCOMPARE(database.lastError().code(), DBError::LocationAlreadyRegistered);

    //try to register a service with a dll that provides interface implementations
    //that are already provided by a currently registered service
    parser.setDevice(new QFile(testdir.absoluteFilePath("ServicePrimatech2error.xml")));
    QVERIFY(parser.extractMetadata());
    QVERIFY(!registerService(parser.parseResults()));
    QCOMPARE(database.lastError().code(), DBError::IfaceImplAlreadyRegistered);

    parser.setDevice(new QFile(testdir.absoluteFilePath("ServiceYamagatoError.xml")));
    QVERIFY(parser.extractMetadata());
    QVERIFY(!registerService(parser.parseResults()));
    QCOMPARE(database.lastError().code(), DBError::LocationAlreadyRegistered);

    //make sure errors above are corectly rolled back by
    //registering a valid service
    parser.setDevice(new QFile(testdir.absoluteFilePath("ServiceDecepticon.xml")));
    QVERIFY(parser.extractMetadata());
    QVERIFY(registerService(parser.parseResults()));

    QStringList xmlFiles;
    xmlFiles << "ServiceDharma_Swan.xml"
             << "ServiceDharma_Pearl.xml"
             << "ServiceDharma_Flame.xml";

    foreach(const QString &file, xmlFiles) {
        //Check that when we register a service we reseed
        //the random number generator so we don't have
        //any service id clashes.  To verify this is
        //happening we set the seed here to 1.
        qsrand(1);
        parser.setDevice(new QFile(testdir.absoluteFilePath(file)));
        QVERIFY(parser.extractMetadata());
        QVERIFY(registerService(parser.parseResults()));
    }

    QVERIFY(database.close());
    QVERIFY(database.close());  //verify we can close multiple times
                                //without side effects
}

void ServiceDatabaseUnitTest::getInterfaces()
{
    QServiceFilter filter;
    QList<QServiceInterfaceDescriptor> interfaces;

    filter.setServiceName("acme");
    interfaces = database.getInterfaces(filter);
    QCOMPARE(database.lastError().code(), DBError::DatabaseNotOpen);
    QVERIFY(database.close());  //check we can close the database even if it
                                //has not been opened

    QVERIFY(database.open());
    interfaces = database.getInterfaces(filter);
    QCOMPARE(database.lastError().code(), DBError::NoError);
    QCOMPARE(interfaces.count(), 5);

    QHash<QString,QString> customs;
    QStringList capabilities;
    QVERIFY(compareDescriptor(interfaces[0], "com.acme.service.downloader", "acme", 1, 0, capabilities, customs, "C:/TestData/testservice.dll",
            "Acme services", "Interface that provides download support"));

    QServiceInterfaceDescriptor interface;
    customs["coordinate"] = QString("global");
    QVERIFY(compareDescriptor(interfaces[1], "com.acme.service.location", "acme", 1,0, capabilities,customs,  "C:/TestData/testservice.dll"));

    customs.clear();
    customs["cpu"]=QString("");
    capabilities.append(QString("ReadUserData"));
    QVERIFY(compareDescriptor(interfaces[2], "com.acme.device.sysinfo", "acme", 2, 3, capabilities, customs, "C:/TestData/testservice.dll"));

    customs.clear();
    customs["smtp"]=QString("smtp.mail.com");
    capabilities.clear();
    capabilities.append("ReadUserData");
    capabilities.append("WriteUserData");
    QVERIFY(compareDescriptor(interfaces[3], "com.acme.device.sendMessage", "acme", 3, 0, capabilities,customs,  "C:/TestData/testservice.dll"));

    customs.clear();
    customs["imap"]=QString("imap.mail.com");
    customs["pop"]=QString("pop.mail.com");
    capabilities.clear();
    capabilities.append("ReadUserData");
    capabilities.append("WriteUserData");
    capabilities.append("ExecUserData");
    QVERIFY(compareDescriptor(interfaces[4], "com.acme.device.receiveMessage", "acme", 1, 1, capabilities, customs, "C:/TestData/testservice.dll"));

    //check that searching is case insensitive
    filter.setServiceName("OmNi");
    interfaces = database.getInterfaces(filter);
    QCOMPARE(database.lastError().code(), DBError::NoError);
    QCOMPARE(interfaces.count(), 3);

    customs.clear();
    capabilities.clear();
    capabilities << "SurroundingsDD";
    QVERIFY(compareDescriptor(interfaces[0], "com.omni.device.Accelerometer", "OMNI", 1, 1, capabilities, customs, "C:/OmniInc/omniinc.dll",
                "Omni mobile", "Interface that provides accelerometer readings(omni)"));

    capabilities.clear();
    QVERIFY(compareDescriptor(interfaces[1], "com.omni.device.Lights","OMNI", 9, 0, capabilities, customs, "C:/OmniInc/omniinc.dll"));

    capabilities.clear();
    capabilities << "MultimediaDD" << "NetworkServices" << "ReadUserData" << "WriteUserData";
    QVERIFY(compareDescriptor(interfaces[2], "com.omni.service.Video", "OMNI", 1, 4, capabilities, customs, "C:/OmniInc/omniinc.dll"));

    QVERIFY(database.close());
}

void ServiceDatabaseUnitTest::searchByInterfaceName()
{
    QServiceFilter filter;
    QList<QServiceInterfaceDescriptor> interfaces;

    QVERIFY(database.open());

    QString iface = "com.omni.device.Accelerometer";
    // == search via interface name only ==
    filter.setInterface(iface);
    interfaces = database.getInterfaces(filter);

    QCOMPARE(interfaces.count(), 5);
    QHash<QString,QString> customs;
    QStringList capabilities;
    capabilities << "SurroundingsDD";
    QVERIFY(compareDescriptor(interfaces[0], iface, "OMNI", 1, 1, capabilities,customs,"C:/OmniInc/omniinc.dll",
               "Omni mobile", "Interface that provides accelerometer readings(omni)"));
    QVERIFY(compareDescriptor(interfaces[1], iface, "LuthorCorp", 1, 2, capabilities,customs,"C:/Metropolis/kryptonite.dll",
           "", "Interface that provides accelerometer readings")); //service description is empty tag in xml
    QVERIFY(compareDescriptor(interfaces[2], iface, "WayneEnt", 2, 0, capabilities,customs,"C:/Gotham/knight.dll",
            "", "Interface that provides accelerometer readings"));//no service description tag in xml
    QVERIFY(compareDescriptor(interfaces[3], iface, "Primatech", 1, 4, capabilities,customs,"C:/NewYork/kensei.dll",
                "Primatech Cellular Services", "Interface that provides accelerometer readings"));
    QVERIFY(compareDescriptor(interfaces[4], iface, "Primatech", 1, 2, capabilities,customs,"C:/NewYork/kensei.dll",
                "Primatech Cellular Services", "Interface that provides accelerometer readings"));

    //search with non-existent interface name
    filter.setInterface("com.omni.device.FluxCapacitor");
    interfaces = database.getInterfaces(filter);

    QCOMPARE(interfaces.count(), 0);

    // == search for an exact version match ==
    filter.setInterface(iface, "1.4", QServiceFilter::ExactVersionMatch);
    interfaces = database.getInterfaces(filter);

    QCOMPARE(interfaces.count(), 1);
    QVERIFY(compareDescriptor(interfaces[0], iface, "Primatech", 1, 4));

    //try exact version match but with multiple expected instances returned.
    filter.setInterface("com.oMNi.device.accelerometer", "1.2", QServiceFilter::ExactVersionMatch); //also test case insensitivity
    interfaces = database.getInterfaces(filter);

    QCOMPARE(interfaces.count(), 2);
    QVERIFY(compareDescriptor(interfaces[0], iface, "LuthorCorp", 1, 2));
    QVERIFY(compareDescriptor(interfaces[1], iface, "Primatech", 1, 2));

    //try exact match for an interface that exists but a version that doesn't
    filter.setInterface(iface,"1.3", QServiceFilter::ExactVersionMatch);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 0);

    // == search for a minimum version match ==
    filter.setInterface(iface,"1.2", QServiceFilter::MinimumVersionMatch);//use existent version
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 4);

    QVERIFY(compareDescriptor(interfaces[0], iface, "LuthorCorp", 1, 2));
    QVERIFY(compareDescriptor(interfaces[1], iface, "WayneEnt", 2, 0));
    QVERIFY(compareDescriptor(interfaces[2], iface, "Primatech", 1, 4));
    QVERIFY(compareDescriptor(interfaces[3], iface, "Primatech", 1, 2));

    filter.setInterface(iface, "1.3", QServiceFilter::MinimumVersionMatch);//use non-existent version
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 2);
    QVERIFY(compareDescriptor(interfaces[0], iface, "WayneEnt", 2, 0));
    QVERIFY(compareDescriptor(interfaces[1], iface, "Primatech", 1, 4));

    //try minimum version match that will find all available versions
    filter.setInterface(iface,"1.0", QServiceFilter::MinimumVersionMatch);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 5);

    //try minimum version match but no implementations are available
    filter.setInterface(iface,"9.0", QServiceFilter::MinimumVersionMatch);//use non-existent version
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 0);

    //try setting an invalid version (should default to interface name search)
    filter.setInterface(iface,"-1.0", QServiceFilter::MinimumVersionMatch);//use non-existent version
    interfaces =database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 0);

    // == search for an interface spread over multiple plugins by a single service==
    iface = "com.dharma.electro.discharge";
    filter.setInterface(iface);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 4);
    capabilities.clear();
    QVERIFY(compareDescriptor(interfaces[0], iface, "DharmaInitiative", 4, 0, capabilities,customs, "C:/island/swan.dll"));
    QVERIFY(compareDescriptor(interfaces[1], iface, "DharmaInitiative", 8, 0, capabilities, customs,"C:/island/pearl.dll"));
    QVERIFY(compareDescriptor(interfaces[2], iface, "DharmaInitiative", 15, 0, capabilities, customs,"C:/island/flame.dll"));
    QVERIFY(compareDescriptor(interfaces[3], iface, "DharmaInitiative", 16, 0, capabilities, customs,"C:/island/flame.dll"));

    //try searching by minimum version for interface implementation spread over multiple plugins
    filter.setInterface(iface, "5.0", QServiceFilter::MinimumVersionMatch);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 3);
    QVERIFY(compareDescriptor(interfaces[0], iface, "DharmaInitiative", 8, 0, capabilities, customs,"C:/island/pearl.dll"));
    QVERIFY(compareDescriptor(interfaces[1], iface, "DharmaInitiative", 15, 0, capabilities, customs,"C:/island/flame.dll"));
    QVERIFY(compareDescriptor(interfaces[2], iface, "DharmaInitiative", 16, 0, capabilities, customs,"C:/island/flame.dll"));

    //try searching for a single version of an interface implementation
    filter.setInterface(iface, "8.0", QServiceFilter::ExactVersionMatch);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 1);
    QVERIFY(compareDescriptor(interfaces[0], iface, "DharmaInitiative", 8, 0, capabilities, customs,"C:/island/pearl.dll"));

    QVERIFY(database.close());
}

void ServiceDatabaseUnitTest::searchByInterfaceAndService()
{
    QServiceFilter filter;
    QList<QServiceInterfaceDescriptor> interfaces;

    QVERIFY(database.open());

    // == search using only interface and service name ==
    filter.setServiceName("Omni");
    filter.setInterface("com.omni.device.Lights");
    interfaces = database.getInterfaces(filter);
    QVERIFY(compareDescriptor(interfaces[0], "com.omni.device.Lights", "OMNI", 9, 0));

    QCOMPARE(interfaces.count(), 1);

    //try searching with service that implements the same interface
    //more than once
    filter.setServiceName("Primatech");
    filter.setInterface("com.omni.device.Accelerometer");
    interfaces = database.getInterfaces(filter);

    QCOMPARE(interfaces.count(), 2);
    QVERIFY(compareDescriptor(interfaces[0], "com.omni.device.Accelerometer", "Primatech", 1, 4));
    QVERIFY(compareDescriptor(interfaces[1], "com.omni.device.Accelerometer", "Primatech", 1, 2));

    //try and existing service but non-existent interface
    filter.setServiceName("Primatech");
    filter.setInterface("com.omni.device.FluxCapacitor");

    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 0);

    //try an non-existing service but an existing interface
    filter.setServiceName("StarkInd");
    filter.setInterface("com.omni.device.Accelerometer");

    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 0);

    // == search using interface and service name and exact version match ==
    filter.setServiceName("Primatech");
    filter.setInterface("com.omni.device.Accelerometer", "1.2", QServiceFilter::ExactVersionMatch);
    interfaces = database.getInterfaces(filter);

    QCOMPARE(interfaces.count(), 1);
    QVERIFY(compareDescriptor(interfaces[0], "com.omni.device.Accelerometer", "Primatech", 1, 2));

    //try an exact match for and non-existent interface version
    filter.setInterface("com.omni.device.Accelerometer","1.3", QServiceFilter::ExactVersionMatch);
    interfaces = database.getInterfaces(filter);

    // == search using interface and service name and minimum version match ==
    filter.setServiceName("Cyberdyne");
    filter.setInterface("com.cyberdyne.terminator","1.6", QServiceFilter::MinimumVersionMatch);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 4);

    QVERIFY(compareDescriptor(interfaces[0], "com.cyberdyne.terminator", "Cyberdyne", 2, 1));
    QVERIFY(compareDescriptor(interfaces[1], "com.cyberdyne.terminator", "Cyberdyne", 2, 0));
    QVERIFY(compareDescriptor(interfaces[2], "com.cyberdyne.terminator", "Cyberdyne", 1, 7));
    QVERIFY(compareDescriptor(interfaces[3], "com.cyberdyne.terminator", "Cyberdyne", 1, 6));

    //try again with the same interface but a different service
    filter.setServiceName("Skynet");
    filter.setInterface("com.cyberdyne.terminator", "1.6", QServiceFilter::MinimumVersionMatch);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 4);

    QVERIFY(compareDescriptor(interfaces[0], "com.cyberdyne.terminator", "skynet", 3, 6));
    QVERIFY(compareDescriptor(interfaces[1], "com.cyberdyne.terminator", "skynet", 2, 0));
    QVERIFY(compareDescriptor(interfaces[2], "com.cyberdyne.terminator", "skynet", 1, 8));
    QVERIFY(compareDescriptor(interfaces[3], "com.cyberdyne.terminator", "skynet", 1, 6));

    //try with a non-existent interface version (but later versions exist)
    filter.setServiceName("Skynet");
    filter.setInterface("com.cyberdyne.terminator","1.9", QServiceFilter::MinimumVersionMatch);
    interfaces = database.getInterfaces(filter);
    QVERIFY(compareDescriptor(interfaces[0], "com.cyberdyne.terminator", "skynet", 3, 6));
    QVERIFY(compareDescriptor(interfaces[1], "com.cyberdyne.terminator", "skynet", 2, 0));
    QCOMPARE(interfaces.count(), 2);

    // == using wildcard matching when searching, ie service and/or interface field is empty
    filter.setServiceName("");
    filter.setInterface("");
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 36);

    // == searches on a service which is made up of multiple plugins
    // try searching for all interfaces offered by the service
    filter.setServiceName("DharmaInitiative");
    filter.setInterface("");
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 5);

    QHash<QString,QString> customs;
    QStringList capabilities;
    QVERIFY(compareDescriptor(interfaces[0], "com.dharma.electro.discharge", "DharmaInitiative", 4, 0, capabilities, customs, "C:/island/swan.dll",
                "Department of Heuristics And Research on Material Applications(S)",
                "Releases electromagnetic energy buildup every 108 minutes"));
    QVERIFY(compareDescriptor(interfaces[1], "com.dharma.electro.discharge", "DharmaInitiative", 8, 0, capabilities, customs, "C:/island/pearl.dll",
                "Department of Heuristics And Research on Material Applications(P)",
                "Releases electromagnetic energy buildup every 108 minutes"));
    QVERIFY(compareDescriptor(interfaces[2], "com.dharma.electro.discharge", "DharmaInitiative", 15, 0, capabilities, customs, "C:/island/flame.dll",
                "Department of Heuristics And Research on Material Applications(F)",
                "Releases electromagnetic energy buildup every 108 minutes"));
    QVERIFY(compareDescriptor(interfaces[3], "com.dharma.radio", "DharmaInitiative", 8, 15, capabilities, customs, "C:/island/flame.dll",
                "Department of Heuristics And Research on Material Applications(F)",
                "Enables communication off island"));
    QVERIFY(compareDescriptor(interfaces[4], "com.dharma.electro.discharge", "DharmaInitiative", 16, 0, capabilities, customs, "C:/island/flame.dll",
                "Department of Heuristics And Research on Material Applications(F)",
                "Releases electromagnetic energy buildup every 108 minutes"));

    // try searching for all implementations of a specific interface offered by the service
    filter.setServiceName("DharmaInitiative");
    filter.setInterface("com.dharma.electro.discharge");
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 4);
    QVERIFY(compareDescriptor(interfaces[0], "com.dharma.electro.discharge", "DharmaInitiative", 4, 0, capabilities, customs, "C:/island/swan.dll"));
    QVERIFY(compareDescriptor(interfaces[1], "com.dharma.electro.discharge", "DharmaInitiative", 8, 0, capabilities, customs, "C:/island/pearl.dll"));
    QVERIFY(compareDescriptor(interfaces[2], "com.dharma.electro.discharge", "DharmaInitiative", 15, 0, capabilities, customs, "C:/island/flame.dll"));
    QVERIFY(compareDescriptor(interfaces[3], "com.dharma.electro.discharge", "DharmaInitiative", 16, 0, capabilities, customs, "C:/island/flame.dll"));

    //try doing a minimum version search
    filter.setServiceName("DharmaInitiative");
    filter.setInterface("com.dharma.electro.discharge", "7.9", QServiceFilter::MinimumVersionMatch);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 3);
    QVERIFY(compareDescriptor(interfaces[0], "com.dharma.electro.discharge", "DharmaInitiative", 8, 0, capabilities, customs, "C:/island/pearl.dll"));
    QVERIFY(compareDescriptor(interfaces[1], "com.dharma.electro.discharge", "DharmaInitiative", 15, 0, capabilities, customs, "C:/island/flame.dll"));
    QVERIFY(compareDescriptor(interfaces[2], "com.dharma.electro.discharge", "DharmaInitiative", 16, 0, capabilities, customs, "C:/island/flame.dll"));

    //try doing a exact version search
    filter.setServiceName("DharmaInitiative");
    filter.setInterface("com.dharma.electro.discharge", "15.0", QServiceFilter::ExactVersionMatch);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 1);
    QVERIFY(compareDescriptor(interfaces[0], "com.dharma.electro.discharge", "DharmaInitiative", 15, 0, capabilities, customs, "C:/island/flame.dll"));

    //trying setting invalid interface parameters, supply a version without an interface
    filter.setInterface("", "3.0", QServiceFilter::MinimumVersionMatch); //this call should be ignored
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 1);
    QVERIFY(compareDescriptor(interfaces[0], "com.dharma.electro.discharge", "DharmaInitiative", 15, 0, capabilities, customs, "C:/island/flame.dll"));

    QVERIFY(database.close());
}

void ServiceDatabaseUnitTest::searchByCapability()
{
    QServiceFilter filter;
    QList<QServiceInterfaceDescriptor> interfaces;
    QHash<QString,QString> customs;
    QHash<QString,QString> customsW = customs;
    customsW["weapon"] = "";

    QVERIFY(database.open());

    filter.setServiceName("Decepticon");
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(),4);

    filter.setCapabilities(QServiceFilter::MatchLoadable, QStringList());
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(),1);
    QVERIFY(compareDescriptor(interfaces[0], "com.cybertron.transform", "Decepticon", 5, 3,
                QStringList(), customs, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));

    QStringList caps;
    caps << "hunt" << "spy";
    filter.setCapabilities(QServiceFilter::MatchMinimum, caps);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(),2);
    QVERIFY(compareDescriptor(interfaces[0], "com.cybertron.transform", "Decepticon", 2, 0,
                (QStringList()<<"hunt" << "spy" << "kill"), customs, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));
    QVERIFY(compareDescriptor(interfaces[1], "com.cybertron.transform", "Decepticon", 2, 5,
                (QStringList()<<"hunt" << "spy"), customs, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));

    filter.setCapabilities(QServiceFilter::MatchLoadable, caps);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(),3);
    QVERIFY(compareDescriptor(interfaces[0], "com.cybertron.transform", "Decepticon", 1, 1,
                QStringList()<<"hunt", customsW, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));
    QVERIFY(compareDescriptor(interfaces[1], "com.cybertron.transform", "Decepticon", 5, 3,
                QStringList() , customs, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));
    QVERIFY(compareDescriptor(interfaces[2], "com.cybertron.transform", "Decepticon", 2, 5,
                (QStringList()<<"hunt" << "spy"), customs, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));

    caps.clear();
    caps << "hunt";
    filter.setCapabilities(QServiceFilter::MatchMinimum, caps);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(),3);
    QVERIFY(compareDescriptor(interfaces[0], "com.cybertron.transform", "Decepticon", 2, 0,
                (QStringList()<<"hunt" << "spy" << "kill"), customs, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));
    QVERIFY(compareDescriptor(interfaces[1], "com.cybertron.transform", "Decepticon", 1, 1,
                QStringList()<<"hunt" , customsW, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));
    QVERIFY(compareDescriptor(interfaces[2], "com.cybertron.transform", "Decepticon", 2, 5,
                (QStringList()<<"hunt" << "spy"), customs, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));


    filter.setCapabilities(QServiceFilter::MatchLoadable, caps);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(),2);
    QVERIFY(compareDescriptor(interfaces[0], "com.cybertron.transform", "Decepticon", 1, 1,
                QStringList()<<"hunt", customsW, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));
    QVERIFY(compareDescriptor(interfaces[1], "com.cybertron.transform", "Decepticon", 5, 3,
                QStringList(), customs, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));

    caps.clear();
    filter.setCapabilities(QServiceFilter::MatchMinimum, caps);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(),4);
    QVERIFY(compareDescriptor(interfaces[0], "com.cybertron.transform", "Decepticon", 2, 0,
                (QStringList()<<"hunt" << "spy" << "kill"), customs, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));
    QVERIFY(compareDescriptor(interfaces[1], "com.cybertron.transform", "Decepticon", 1, 1,
                QStringList()<<"hunt", customsW, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));
    QVERIFY(compareDescriptor(interfaces[2], "com.cybertron.transform", "Decepticon", 5, 3,
                QStringList(), customs, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));
    QVERIFY(compareDescriptor(interfaces[3], "com.cybertron.transform", "Decepticon", 2, 5,
                (QStringList()<<"hunt" << "spy"), customs, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));


    filter.setCapabilities(QServiceFilter::MatchLoadable, caps);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(),1);
    QVERIFY(compareDescriptor(interfaces[0], "com.cybertron.transform", "Decepticon", 5, 3,
                QStringList(), customs, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));

    QServiceFilter emptyFilter;
    emptyFilter.setCapabilities(QServiceFilter::MatchLoadable);
    interfaces = database.getInterfaces(emptyFilter);
    QCOMPARE(interfaces.count(), 14); //show all services which don't require any caps
}

void ServiceDatabaseUnitTest::searchByCustomAttribute()
{
    QServiceFilter filter;
    QList<QServiceInterfaceDescriptor> interfaces;
    QHash<QString,QString> customs;
    QStringList capabilities;

    QVERIFY(database.open());

    filter.setServiceName("Autobot");
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(),5);

    filter.setCustomAttribute("bot", "automatic");
    QCOMPARE(filter.customAttribute("bot"), QString("automatic"));
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(),3);

    customs["bot"] = "automatic";
    customs["extension"] = "multidrive";
    QVERIFY(compareDescriptor(interfaces[0], "com.cybertron.transform", "Autobot", 2, 7,
                capabilities, customs, "C:/Ark/matrix.dll", "Autobot Protection Services", "Transformation interface"));

    customs.clear();
    customs["bot"] = "automatic";
    QVERIFY(compareDescriptor(interfaces[1], "com.cybertron.transform", "Autobot", 2, 5,
                capabilities, customs, "C:/Ark/matrix.dll", "Autobot Protection Services", "Transformation interface"));

    customs.clear();
    customs["bot"] = "automatic";
    customs["weapon"] = "";
    QVERIFY(compareDescriptor(interfaces[2], "com.cybertron.transform", "Autobot", 1, 9,
                capabilities, customs, "C:/Ark/matrix.dll", "Autobot Protection Services", "Transformation interface"));

    filter.setCustomAttribute("extension","multidrive");
    QCOMPARE(filter.customAttribute("extension"), QString("multidrive"));
    QCOMPARE(filter.customAttribute("bot"), QString("automatic"));
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(),1);

    customs.clear();
    customs["bot"] = "automatic";
    customs["extension"] = "multidrive";
    QVERIFY(compareDescriptor(interfaces[0], "com.cybertron.transform", "Autobot", 2, 7,
                capabilities, customs, "C:/Ark/matrix.dll", "Autobot Protection Services", "Transformation interface"));

    QServiceFilter manualFilter;
    manualFilter.setCustomAttribute("bot", "manual");
    interfaces = database.getInterfaces(manualFilter);
    QCOMPARE(interfaces.count(),2);

    customs.clear();
    customs["bot"]="manual";
    customs["weapon"]="laser";
    QVERIFY(compareDescriptor(interfaces[0], "com.cybertron.transform", "Autobot", 1, 0,
                capabilities, customs, "C:/Ark/matrix.dll", "Autobot Protection Services", "Transformation interface"));
    customs.clear();
    customs["bot"] = "manual";
    QVERIFY(compareDescriptor(interfaces[1], "com.cybertron.transform", "Autobot", 2, 0,
                capabilities, customs, "C:/Ark/matrix.dll", "Autobot Protection Services", "Transformation interface"));

    QServiceFilter multidriveFilter;
    multidriveFilter.setCustomAttribute("extension", "multidrive");
    interfaces = database.getInterfaces(multidriveFilter);
    QCOMPARE(interfaces.count(),1);

    customs.clear();
    customs["bot"] = "automatic";
    customs["extension"] = "multidrive";
    QVERIFY(compareDescriptor(interfaces[0], "com.cybertron.transform", "Autobot", 2, 7,
                capabilities, customs, "C:/Ark/matrix.dll", "Autobot Protection Services", "Transformation interface"));

    //test whether querying a custom property will affect the filter
    filter.setServiceName("");
    filter.setInterface("");
    filter.clearCustomAttribute();
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 36);
    QString customProperty = filter.customAttribute("spark");
    QVERIFY(customProperty.isEmpty());
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 36);

    //test the removal of a custom property from the filter
    filter.setCustomAttribute("bot", "automatic");
    filter.setCustomAttribute("extension", "multidrive");
    QCOMPARE(filter.customAttributes().length(), 2);
    filter.clearCustomAttribute("bot");
    QCOMPARE(filter.customAttributes().length(), 1);
    filter.clearCustomAttribute("extension");
    QCOMPARE(filter.customAttributes().length(), 0);

    //test clearing of custom attributes
    filter.setCustomAttribute("bot", "automatic");
    filter.setCustomAttribute("extension", "multidrive");
    QCOMPARE(filter.customAttributes().length(),2);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 1);
    filter.clearCustomAttribute();
    QCOMPARE(filter.customAttributes().length(), 0);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 36);

    //test searching for an empty custom property
    filter.setCustomAttribute("weapon", "");
    interfaces = database.getInterfaces(filter);
    customs.clear();
    customs["bot"] = "automatic";
    customs["weapon"] = "";
    QCOMPARE(interfaces.length(), 2);
    QVERIFY(compareDescriptor(interfaces[0], "com.cybertron.transform", "Autobot", 1, 9,
                capabilities, customs, "C:/Ark/matrix.dll", "Autobot Protection Services", "Transformation interface"));
    customs.clear();
    capabilities.clear();
    capabilities << "hunt";
    customs["weapon"]= "";
    QVERIFY(compareDescriptor(interfaces[1], "com.cybertron.transform", "Decepticon", 1, 1,
                capabilities, customs, "C:/Cybertron/unicron.dll", "Decepticon Elimination Services", "Transformation interface"));


    filter.clearCustomAttribute();

    //test searching against a non-existent custom property
    filter.setCustomAttribute("fluxcapacitor", "fluxing");
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 0);
    QCOMPARE(database.lastError().code(), DBError::NoError);

    //try searching for custom property with service name and interface constraints
    filter.clearCustomAttribute();
    filter.setServiceName("autobot");
    filter.setInterface("com.cybertron.transform", "2.0");
    filter.setCustomAttribute("bot", "automatic");
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(),2);
    QVERIFY(interfaces[0].majorVersion() == 2 && interfaces[0].minorVersion() == 7);
    QVERIFY(interfaces[1].majorVersion() == 2 && interfaces[1].minorVersion() == 5);

    //test that there is a difference between querying a custom
    //property with an empty value and a custom property that has not been set
    filter.clearCustomAttribute();
    filter.setCustomAttribute("AllSpark", "");
    QVERIFY(!filter.customAttribute("AllSpark").isNull());
    QVERIFY(filter.customAttribute("AllSpark").isEmpty());
    QVERIFY(filter.customAttribute("Non-existentProperty").isNull());

    QVERIFY(database.close());
}

void ServiceDatabaseUnitTest::attributes()
{
    QVERIFY(database.open());

    // == Capability property ==
    //get empty list of capabilities from an interface registered with capabilities=""
    QServiceFilter filter;
    filter.setServiceName("acme");
    filter.setInterface("com.acme.service.location", "1.0", QServiceFilter::ExactVersionMatch);
    QList<QServiceInterfaceDescriptor> interfaces = database.getInterfaces(filter);
    QCOMPARE(database.lastError().code(), DBError::NoError);
    QCOMPARE(interfaces.count(), 1);
    QServiceInterfaceDescriptor interface = interfaces[0];
    QCOMPARE(interface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList(), QStringList());

    //get empty list of capabilites from an interface registered without a
    //capabilities attribute
    filter.setServiceName("DHarmaInitiative");
    filter.setInterface("Com.dharma.electro.discharge", "16.0", QServiceFilter::ExactVersionMatch);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 1);
    interface = interfaces[0];
    QCOMPARE(interface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList(), QStringList());

    //get a list of capabilites from an interface with only 1 capability
    filter.setServiceName("Omni");
    filter.setInterface("com.omni.device.Accelerometer", "1.1", QServiceFilter::ExactVersionMatch);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 1);
    interface = interfaces[0];
    QStringList capabilities;
    capabilities  << "SurroundingsDD";
    QCOMPARE(interface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList(), capabilities);

    //get a list of capabilites from an interface with multiple capabilities
    filter.setServiceName("Omni");
    filter.setInterface("com.omni.service.Video", "1.4", QServiceFilter::ExactVersionMatch);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 1);
    interface = interfaces[0];
    capabilities.clear();
    capabilities  << "MultimediaDD" << "NetworkServices" << "ReadUserData" << "WriteUserData";
    QCOMPARE(interface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList(), capabilities);

    //get a list of capabilities from a default service
    interface = database.interfaceDefault("com.cyberdyne.terminator");
    capabilities.clear();
    capabilities << "NetworkServices";
    QCOMPARE(interface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList(), capabilities);

    // == Interface Description Property ==
    //get the interface description off an interface registered without a description tag
    filter.setServiceName("Skynet");
    filter.setInterface("com.cyberdyne.terminator", "1.8", QServiceFilter::ExactVersionMatch);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 1);
    interface = interfaces[0];
    QCOMPARE(interface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString(), QString());

    //get the interface description off an interface registered with an
    //empty description tag
    filter.setServiceName("Skynet");
    filter.setInterface("com.cyberdyne.terminator", "1,6", QServiceFilter::ExactVersionMatch);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 1);
    interface = interfaces[0];
    QCOMPARE(interface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString(), QString());

    //get the interface description of an interface
    filter.setServiceName("Skynet");
    filter.setInterface("com.cyberdyne.terminator", "1.5", QServiceFilter::ExactVersionMatch);
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 1);
    interface = interfaces[0];
    QCOMPARE(interface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString(),
            QString("Remote communications interface for the T-800v1.5"));

    //get a description from a default service
    interface = database.interfaceDefault("com.omni.device.Accelerometer");
    QCOMPARE(interface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString(),
            QString("Interface that provides accelerometer readings(omni)"));

    QVERIFY(database.close());
}

void ServiceDatabaseUnitTest::getServiceNames()
{
    QStringList services;
    services = database.getServiceNames("com.acme.device.sysinfo");
    QCOMPARE(database.lastError().code(), DBError::DatabaseNotOpen);
    QCOMPARE(services.count(), 0);
    QVERIFY(database.open());

    //try wildcard match to get all services
    services = database.getServiceNames("" );
    QCOMPARE(database.lastError().code(), DBError::NoError);
    QCOMPARE(services.count(), 10);

    //try an interface that is implemented by only one service
    services = database.getServiceNames("com.acme.device.sysinfo");
    QCOMPARE(database.lastError().code(), DBError::NoError);
    QCOMPARE(services.count(), 1);
    QCOMPARE(services[0], QString("acme"));

    //try an interface which  is implmented by multiple services
    services = database.getServiceNames("COM.omni.device.ACCELerometer"); //also test case insensitivity
    QCOMPARE(database.lastError().code(), DBError::NoError);
    QCOMPARE(services.count(), 4);
    QVERIFY(services.contains("LuthorCorp"));
    QVERIFY(services.contains("OMNI"));
    QVERIFY(services.contains("Primatech"));
    QVERIFY(services.contains("WayneEnt"));

    //try again but with services that have multiple implementation versions of a particular implementation
    services = database.getServiceNames("com.cyberdyne.terminator");
    QCOMPARE(database.lastError().code(), DBError::NoError);
    QCOMPARE(services.count(), 2);
    QVERIFY(services.contains("Cyberdyne"));
    QVERIFY(services.contains("skynet"));

    //try with an interface implemented in multiple plugins
    services = database.getServiceNames("com.dharma.electro.discharge");
    QCOMPARE(database.lastError().code(), DBError::NoError);
    QCOMPARE(services.count(), 1);
    QVERIFY(services.contains("DharmaInitiative", Qt::CaseInsensitive));

    //try with a non-existing interface
    services = database.getServiceNames("com.omni.device.FluxCapacitor");
    QCOMPARE(database.lastError().code(), DBError::NoError);
    QCOMPARE(services.count(), 0);

    QVERIFY(database.close());
}

bool ServiceDatabaseUnitTest::compareDescriptor(QServiceInterfaceDescriptor interface,
    QString interfaceName, QString serviceName, int majorVersion, int minorVersion)
{
    interface.d->attributes[QServiceInterfaceDescriptor::Capabilities] = QStringList();

    QHash<QString,QString> customs;
    return compareDescriptor(interface, interfaceName, serviceName, majorVersion, minorVersion,
            QStringList(), customs);
}

bool ServiceDatabaseUnitTest::compareDescriptor(QServiceInterfaceDescriptor interface,
    QString interfaceName, QString serviceName, int majorVersion, int minorVersion,
    QStringList capabilities, const QHash<QString,QString> customProps, QString filePath, QString serviceDescription,
    QString interfaceDescription)
{

    if(interface.interfaceName().compare(interfaceName, Qt::CaseInsensitive) !=0) {
        qWarning() << "Interface name mismatch: expected =" << interfaceName
                    << " actual =" << interface.interfaceName();
        return false;
    }

    if (interface.serviceName().compare(serviceName, Qt::CaseInsensitive) != 0) {
        qWarning() << "Service name mismatch: expected =" << serviceName
                    << " actual =" << interface.serviceName();
        return false;
    }

    if (interface.majorVersion() != majorVersion) {
        qWarning() << "Major version mismatch: expected =" << majorVersion
                        << " actual =" << interface.majorVersion();
        return false;
    }

    if (interface.minorVersion() != minorVersion) {
        qWarning() << "Minor version mismatch: expected =" << minorVersion
                    << " actual =" << interface.minorVersion();
        return false;
    }

    if (capabilities.count() != 0 || interface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList().count() != 0 ) {
        QStringList securityCapabilities;
        securityCapabilities = interface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList();

        if(securityCapabilities.count() != capabilities.count()) {
            qWarning() << "Capabilities count mismatch: expected =" << capabilities.count()
                        << " actual="<< securityCapabilities.count()
                        << "\texpected capabilities =" << capabilities
                        << "actual capabilities =" << securityCapabilities;
            return false;
        }

        for (int i = 0; i < securityCapabilities.count(); ++i) {
            if (securityCapabilities[i] != capabilities[i]) {
                qWarning() << "Capability mismatch: expected =" << capabilities[i]
                            << " actual =" << securityCapabilities[i];
                return false;
            }
        }
    }

    if (!filePath.isEmpty()) {
        if (interface.attribute(QServiceInterfaceDescriptor::Location).toString() != filePath) {
            qWarning() << "File path mismatch: expected =" << filePath
                << " actual =" << interface.attribute(QServiceInterfaceDescriptor::Location).toString();
            return false;
        }
    }
    if (!serviceDescription.isEmpty()) {
        if (interface.attribute(QServiceInterfaceDescriptor::ServiceDescription).toString() != serviceDescription) {
            qWarning() << "Service Description mismatch: expected =" << serviceDescription
                        << " actual=" << interface.attribute(QServiceInterfaceDescriptor::ServiceDescription).toString();
            return false;
        }
    }
    if (!interfaceDescription.isEmpty()) {
        if (interface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString() != interfaceDescription) {
            qWarning() << "Interface Description mismatch: expected =" << interfaceDescription
                        << " actual =" << interface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString();
            return false;
        }

    }

    if (interface.d->customAttributes.size() != customProps.size()) {
        qWarning() << "Number of Interface custom attributes don't match. expected: "
            <<customProps.size() << "actual: " << interface.d->customAttributes.size();
            ;
        qWarning() << "expected:" << customProps << "actual:" << interface.d->customAttributes;
        return false;
    }

    QHash<QString, QString>::const_iterator i;
    for (i = customProps.constBegin(); i!=customProps.constEnd(); i++) {
        if (interface.customAttribute(i.key()) != i.value()) {
            qWarning() << "Interface custom property mismatch: expected =" << i.key() <<"("<<i.value()<<")"
                        << " actual =" << i.key() << "(" << interface.customAttribute(i.key()) << ")";
            return false;
        }
    }
    return true;
}

void ServiceDatabaseUnitTest::defaultExternalIfaceIDs()
{
    database.open();
    QServiceInterfaceDescriptor interface;
    interface.d = new QServiceInterfaceDescriptorPrivate;
    interface.d->serviceName = "StargateCommand";
    interface.d->interfaceName = "gov.usa.stargate";
    interface.d->major = 13;
    interface.d->minor = 37;

    //see if we can set a "cross-reference" default interface
    //ie user db referencing an interfaceID belonging to the system db
    QVERIFY(database.setInterfaceDefault(interface, "FAKE-INTERFACE-ID"));
    QString interfaceID;
    QServiceInterfaceDescriptor descriptor = database.interfaceDefault("gov.usa.stargate", &interfaceID);
    QCOMPARE(database.lastError().code(), DBError::ExternalIfaceIDFound);
    QCOMPARE(interfaceID, QString("FAKE-INTERFACE-ID"));

    interface.d->interfaceName = "gov.ru.stargate";
    QVERIFY(database.setInterfaceDefault(interface, "FAKE-INTERFACE-ID2"));
    QList<QPair<QString,QString> > externalDefaultsInfo = database.externalDefaultsInfo();
    QCOMPARE(externalDefaultsInfo[0].second, QString("FAKE-INTERFACE-ID"));
    QCOMPARE(externalDefaultsInfo[1].second, QString("FAKE-INTERFACE-ID2"));

    //see if we can remove the "cross-reference" default interface
    QVERIFY(database.removeExternalDefaultServiceInterface("FAKE-INTERFACE-ID"));
    interfaceID.clear();
    descriptor = database.interfaceDefault("gov.usa.stargate", &interfaceID);
    QVERIFY(database.lastError().code() == DBError::NotFound);
    QVERIFY(interfaceID.isEmpty());

    //try to delete an interfaceID that's actually a local interfaceID
    QServiceFilter filter;
    filter.setServiceName("omnI");
    filter.setInterface("com.omni.Device.Lights");
    QList<QServiceInterfaceDescriptor> interfaces;
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 1);
    interfaceID = database.getInterfaceID(interfaces[0]);
    QVERIFY(!database.removeExternalDefaultServiceInterface(interfaceID));
    QVERIFY(database.lastError().code()  ==  DBError::IfaceIDNotExternal);

    database.close();
}

void ServiceDatabaseUnitTest::interfaceDefault()
{
    QServiceInterfaceDescriptor interface;
    bool ok;

    //try getting the default service interface implementation when database is not open
    interface = database.interfaceDefault("com.cyberdyne.terminator");
    QCOMPARE(database.lastError().code(), DBError::DatabaseNotOpen);
    QVERIFY(!interface.isValid());

    //try getting a valid default, in this case only one implementation exists
    QVERIFY(database.open());
    interface = database.interfaceDefault("com.omni.device.Lights");
    QVERIFY(database.lastError().code() == DBError::NoError);
    QVERIFY(interface.isValid());
    QStringList capabilities;
    QHash<QString,QString> customs;
    QVERIFY(compareDescriptor(interface, "com.omni.device.Lights",
                                "OMNI", 9, 0, capabilities, customs,
                                "C:/OmniInc/omniinc.dll",
                                "Omni mobile",
                                "Interface that provides access to device lights"));

    //try getting a valid default, in this case two services implement the interface
    ok = false;
    interface = database.interfaceDefault("com.CyBerDynE.Terminator");
    QVERIFY(database.lastError().code() == DBError::NoError);
    QVERIFY(interface.isValid());

    capabilities << "NetworkServices";
    QVERIFY(compareDescriptor(interface, "com.cyberdyne.terminator",
                                    "Cyberdyne", 2,1, capabilities, customs,
                                    "C:/California/connor.dll",
                                    "Cyberdyne Termination Services",
                                    "Remote communications interface for the T-800"));

    //try getting a valid default, in this case multiple services implement the interface
    ok = false;
    interface = database.interfaceDefault("com.omni.device.Accelerometer");
    QVERIFY(database.lastError().code() == DBError::NoError);
    QVERIFY(interface.isValid());
    capabilities.clear();
    capabilities << "SurroundingsDD";
    QVERIFY(compareDescriptor(interface, "com.omni.device.Accelerometer",
                                    "OMNI", 1, 1, capabilities, customs,
                                    "C:/OmniInc/omniinc.dll",
                                    "Omni mobile",
                                    "Interface that provides accelerometer readings(omni)"));

    //try searching for an interface that isn't registered
    interface = database.interfaceDefault("com.omni.device.FluxCapacitor");
    QVERIFY(database.lastError().code() == DBError::NotFound);
    QVERIFY(!interface.isValid());

    //try getting the default interface impl for a service that is made up of multiple
    //plugins
    interface = database.interfaceDefault("com.dharma.electro.discharge");
    QVERIFY(database.lastError().code() == DBError::NoError);
    capabilities.clear();
    QVERIFY(compareDescriptor(interface, "com.dharma.electro.discharge",
                                "DharmaInitiative", 4, 0,
                                capabilities, customs, "C:/island/swan.dll"));

    //trying getting the default using an empty interface name
    interface = database.interfaceDefault("");
    QVERIFY(database.lastError().code() == DBError::NotFound);
    QVERIFY(!interface.isValid());
    QVERIFY(database.close());
}

void ServiceDatabaseUnitTest::setInterfaceDefault()
{
    QServiceInterfaceDescriptor interface;
    interface.d = new QServiceInterfaceDescriptorPrivate;
    interface.d->serviceName = "cyberdyne";
    interface.d->interfaceName = "com.cyberdyne.terminator";
    interface.d->major = 1;
    interface.d->minor = 6;

    QVERIFY(!database.setInterfaceDefault(interface));
    QCOMPARE(database.lastError().code(), DBError::DatabaseNotOpen);

    QVERIFY(database.open());

    //try setting the default to a older version provided by the same
    //service
    QServiceInterfaceDescriptor defaultInterface;
    defaultInterface = database.interfaceDefault("com.cyberdyne.terminator");
    QVERIFY(compareDescriptor(defaultInterface, "com.cyberdyne.terminator",
                                        "Cyberdyne", 2, 1));

    QVERIFY(database.setInterfaceDefault(interface));
    defaultInterface = database.interfaceDefault("com.cyberdyne.terminator");
    QVERIFY(compareDescriptor(defaultInterface, "com.cyberdyne.terminator",
                                        "Cyberdyne", 1, 6));

    //try setting the default to another service
    interface.d->serviceName = "SKYnet"; //check that behaviour is case insensitive
    interface.d->interfaceName = "COM.cyberdyne.terminaTOR";
    interface.d->major = 1;
    interface.d->minor = 5;

    QVERIFY(database.setInterfaceDefault(interface));
    defaultInterface = database.interfaceDefault("com.CYBERDYNE.terminator");
    QVERIFY(compareDescriptor(defaultInterface, "com.cyberdyne.terminator",
                                        "skynet", 1, 5));

    //try setting the default of a service that is made up of multiple plugins
    interface.d->serviceName = "DharmaInitiative";
    interface.d->interfaceName = "com.dharma.electro.discharge";
    interface.d->major = 8;
    interface.d->minor = 0;
    QVERIFY(database.setInterfaceDefault(interface));
    QStringList capabilities;
    QHash<QString,QString> customs;
    defaultInterface = database.interfaceDefault("com.dharma.electro.discharge");
    QVERIFY(compareDescriptor(defaultInterface, "com.dharma.electro.discharge",
                                "DharmaInitiative", 8, 0, capabilities, customs,
                                "C:/island/pearl.dll"));

    //try setting the default to a implementation verison not supplied
    //by the service
    interface.d->serviceName = "SKYnet"; //check that behaviour is case insensitive
    interface.d->interfaceName = "COM.cyberdyne.terminaTOR";
    interface.d->major = 1;
    interface.d->minor = 9;

    QVERIFY(!database.setInterfaceDefault(interface));
    QCOMPARE(database.lastError().code(), DBError::NotFound);
    defaultInterface = database.interfaceDefault("com.CYBERDYNE.terminator");
    QVERIFY(compareDescriptor(defaultInterface, "com.cyberdyne.terminator",
                                        "skynet", 1, 5));

    //try setting the default of an interface that doesn't exist
    interface.d->serviceName = "SKYnet"; //check that behaviour is case insensitive
    interface.d->interfaceName = "com.omni.device.FluxCapacitor";
    interface.d->major = 1;
    interface.d->minor = 5;

    QVERIFY(!database.setInterfaceDefault(interface));
    QCOMPARE(database.lastError().code(), DBError::NotFound);
    defaultInterface = database.interfaceDefault("com.CYBERDYNE.terminator");
    QVERIFY(compareDescriptor(defaultInterface, "com.cyberdyne.terminator",
                                        "skynet", 1, 5));

    //try setting an interface default to a service that does not implement the interface
    interface.d->serviceName = "Primatech";
    interface.d->interfaceName = "com.cyberdyne.terminator";
    interface.d->major = 1;
    interface.d->minor = 4;
    QVERIFY(!database.setInterfaceDefault(interface));
    QCOMPARE(database.lastError().code(), DBError::NotFound);
    defaultInterface = database.interfaceDefault("com.CYBERDYNE.terminator");
    QVERIFY(compareDescriptor(defaultInterface, "com.cyberdyne.terminator",
                                        "skynet", 1, 5));

    //try setting an interface default to a non-existent service
    interface.d->serviceName = "StarkInd"; //check that behaviour is case insensitive
    interface.d->interfaceName = "COM.cyberdyne.terminaTOR";
    interface.d->major = 1;
    interface.d->minor = 5;
    QVERIFY(!database.setInterfaceDefault(interface));
    QCOMPARE(database.lastError().code(), DBError::NotFound);
    defaultInterface = database.interfaceDefault("com.CYBERDYNE.terminator");
    QVERIFY(compareDescriptor(defaultInterface, "com.cyberdyne.terminator",
                                        "skynet", 1, 5));

    //try setting the default using an invalid interface
    QServiceInterfaceDescriptor invalidInterface;
    QVERIFY(!invalidInterface.isValid());
    QVERIFY(!database.setInterfaceDefault(invalidInterface));
    QCOMPARE(database.lastError().code(), DBError::NotFound);
    defaultInterface = database.interfaceDefault("com.CYBERDYNE.terminator");
    QVERIFY(compareDescriptor(defaultInterface, "com.cyberdyne.terminator",
                                        "skynet", 1, 5));

    QVERIFY(database.close());
}

void ServiceDatabaseUnitTest::unregister()
{
    QVERIFY(!unregisterService("acme"));
    QCOMPARE(database.lastError().code(), DBError::DatabaseNotOpen);

    QVERIFY(database.open());

    //try unregister a non-existing service
    QVERIFY(!unregisterService("StarkInd"));
    QCOMPARE(database.lastError().code(), DBError::NotFound);
    QServiceFilter filter;

    // == check that the service to delete is already in the database ==
    //try a search for descriptors by service name
    filter.setServiceName("omni");
    QList<QServiceInterfaceDescriptor> interfaces;
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 3);

    //search for service via interface name
    QStringList services;
    services = database.getServiceNames("com.omni.device.Accelerometer");
    QCOMPARE(services.count(), 4);
    QVERIFY(services.contains("OMNI"));

    //search for descriptors via interface name
    filter.setServiceName("");
    filter.setInterface("com.omni.device.Accelerometer");
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 5);
    bool serviceFound = false;
    foreach (QServiceInterfaceDescriptor interface, interfaces) {
        if(interface.serviceName() == "OMNI")
            serviceFound = true;
    }
    QVERIFY(serviceFound);

    //confirm that it is the default service for a couple of interfaces
    QServiceInterfaceDescriptor interface;
    interface = database.interfaceDefault("com.omni.device.Accelerometer");
    QVERIFY(interface.serviceName() == "OMNI");//other services implement this interface

    interface = database.interfaceDefault("com.omni.service.Video");
    QVERIFY(interface.serviceName() == "OMNI");//no other services implmement this interface

    //confirm that interface and service attributes exist for the service
    QStringList serviceIDs = getServiceIDs("omni");
    foreach(const QString &serviceID, serviceIDs)
        QVERIFY(existsInServicePropertyTable(serviceID));

    QStringList interfaceIDs = getInterfaceIDs("omni");
    QCOMPARE(interfaceIDs.count(), 3);
    foreach(const QString &interfaceID, interfaceIDs)
        QVERIFY(existsInInterfacePropertyTable(interfaceID));

    QVERIFY(unregisterService("oMni")); //ensure case insensitive behaviour

    //  == check that deleted service and associated interfaces cannot be found ==
    //try a search for descriptors by service name
    QServiceFilter serviceFilter;
    serviceFilter.setServiceName("omni");
    interfaces = database.getInterfaces(serviceFilter);
    QCOMPARE(interfaces.count(), 0);

    //search for service via interface name
    services = database.getServiceNames("com.omni.device.Accelerometer");
    QCOMPARE(services.count(), 3);
    QVERIFY(!services.contains("OMNI"));

    //search for descriptors via interface name
    filter.setServiceName("");
    filter.setInterface("com.omni.device.Accelerometer");
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 4);
    serviceFound = false;
    foreach (QServiceInterfaceDescriptor interface, interfaces) {
        if(interface.serviceName() == "OMNI")
            serviceFound = true;
    }
    QVERIFY(!serviceFound);

    //ensure a new default interface has been assigned for the following interface
    interface = database.interfaceDefault("com.omni.device.Accelerometer");
    QVERIFY(interface.isValid());
    QCOMPARE(interface.serviceName(), QString("WayneEnt"));
    QCOMPARE(interface.majorVersion(), 2);
    QCOMPARE(interface.minorVersion(), 0);

    //ensure there is no longer a default for the following interface
    interface = database.interfaceDefault("com.omni.service.Video");
    QVERIFY(!interface.isValid());

    //ensure the associated interfaceIDs no longer exist in the Property
    //and Defaults tables
    foreach (const QString &serviceID, serviceIDs)
        QVERIFY(!existsInServicePropertyTable(serviceID));
    foreach (const QString &interfaceID, interfaceIDs)
        QVERIFY(!existsInInterfacePropertyTable(interfaceID));
    foreach(const QString &interfaceID, interfaceIDs)
        QVERIFY(!existsInDefaultsTable(interfaceID));

    //  == unregister an service that implements a default interface
    //     but whose default interface name differs in case
    //     from the other implementations it provides ==
    interface.d = new QServiceInterfaceDescriptorPrivate;
    interface.d->serviceName = "Cyberdyne";
    interface.d->interfaceName = "com.cyberdyne.terminator";
    interface.d->major = 2;
    interface.d->minor = 0;
    QVERIFY(database.setInterfaceDefault(interface));

    interface = database.interfaceDefault("com.cyberdyne.terminator");
    QVERIFY(interface.isValid());
    QVERIFY(unregisterService("cyberDYNE"));
    interface = database.interfaceDefault("com.cyberdyne.terminatOR");
    QVERIFY(interface.isValid());
    QVERIFY(compareDescriptor(interface, "com.cyberdyne.terminator",
                                "skynet", 3, 6));

    // == unregister a service that is made up of multiple plugins ==
    // get the serviceIDs and interfaceIDs to make sure it's been deleted later on
    serviceIDs = getServiceIDs("DharmaInitiative");
    QVERIFY(serviceIDs.count() > 0);
    interfaceIDs = getInterfaceIDs("DharmaInitiative");
    QVERIFY(interfaceIDs.count() > 0);

    foreach(const QString &interfaceID, interfaceIDs)
        QVERIFY(existsInInterfacePropertyTable(interfaceID));
    foreach(const QString &serviceID, serviceIDs)
        QVERIFY(existsInServicePropertyTable(serviceID));

    QVERIFY(unregisterService("DHARMAInitiative"));
    interface = database.interfaceDefault("com.dharma.electro.discharge");
    QVERIFY(!interface.isValid());
    QCOMPARE(database.lastError().code(), DBError::NotFound);
    filter.setServiceName("DharmaInitiative");
    filter.setInterface("");
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 0);

    foreach(const QString &serviceID, serviceIDs)
        QVERIFY(!existsInServicePropertyTable(serviceID));
    foreach(const QString &interfaceID, interfaceIDs)
        QVERIFY(!existsInInterfacePropertyTable(interfaceID));
    foreach(const QString &interfaceID, interfaceIDs)
        QVERIFY(!existsInDefaultsTable(interfaceID));

    //  == check that the service can be registered again
    //     after it has been unregistered ==
    QDir testdir = QDir(QFINDTESTDATA("testdata"));
    ServiceMetaData parser(testdir.absoluteFilePath("ServiceDharma_Flame.xml"));
    QVERIFY(parser.extractMetadata());
    QVERIFY(registerService(parser.parseResults()));
    interface = database.interfaceDefault("com.dharma.electro.discharge");
    QVERIFY(interface.isValid());
    filter.setServiceName("DharmaInitiative");
    filter.setInterface("");
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 3);

    parser.setDevice(new QFile(testdir.absoluteFilePath("ServiceDharma_Swan.xml")));
    QVERIFY(parser.extractMetadata());
    QVERIFY(registerService(parser.parseResults()));
    filter.setServiceName("DharmaInitiative");
    filter.setInterface("");
    interfaces = database.getInterfaces(filter);
    QCOMPARE(interfaces.count(), 4);
    interface = database.interfaceDefault("com.dharma.electro.discharge");
    QVERIFY(interface.isValid());
    QStringList capabilities;
    QHash<QString,QString> customs;
    QVERIFY(compareDescriptor(interface, "com.dharma.electro.discharge",
                    "DharmaInitiative", 16, 0, capabilities, customs, "C:/island/flame.dll"));
}

QStringList ServiceDatabaseUnitTest::getInterfaceIDs(const QString &serviceName) {
    QSqlDatabase sqlDatabase  = QSqlDatabase::database(database.m_connectionName);
    QSqlQuery query(sqlDatabase);
    QString statement("Select Interface.ID FROM Interface, Service "
                        "WHERE Service.ID = Interface.ServiceID "
                        "AND Service.Name = ? COLLATE NOCASE");
    QList<QVariant> bindValues;
    bindValues.append(serviceName);
    database.executeQuery(&query, statement, bindValues);

    QStringList ids;
    while   (query.next()) {
        ids << query.value(0).toString();
    }
    return ids;
}

QStringList ServiceDatabaseUnitTest::getServiceIDs(const QString &serviceName) {
    QSqlDatabase sqlDatabase = QSqlDatabase::database(database.m_connectionName);
    QSqlQuery query(sqlDatabase);
    QString statement("SELECT Service.ID from Service "
                        "WHERE Service.Name = ? COLLATE NOCASE");
    QList<QVariant> bindValues;
    bindValues.append(serviceName);
    database.executeQuery(&query, statement, bindValues);

    QStringList ids;
    while(query.next()) {
        ids << query.value(0).toString();
    }
    return ids;
}

bool ServiceDatabaseUnitTest::existsInInterfacePropertyTable(const QString &interfaceID)
{
    QSqlDatabase sqlDatabase  = QSqlDatabase::database(database.m_connectionName);
    QSqlQuery query(sqlDatabase);

    QString statement("SELECT InterfaceID FROM InterfaceProperty "
                "WHERE InterfaceID = ?");
    QList<QVariant> bindValues;
    bindValues.append(interfaceID);
    database.executeQuery(&query, statement, bindValues);
    if (query.next())
        return true;
    else
        return false;
}

bool ServiceDatabaseUnitTest::existsInServicePropertyTable(const QString &serviceID)
{
    QSqlDatabase sqlDatabase  = QSqlDatabase::database(database.m_connectionName);
    QSqlQuery query(sqlDatabase);

    QString statement("SELECT ServiceID from ServiceProperty "
                        "WHERE ServiceID = ?");
    QList<QVariant> bindValues;
    bindValues.append(serviceID);
    database.executeQuery(&query, statement, bindValues);
    if (query.next())
        return true;
    else
        return false;
}

bool ServiceDatabaseUnitTest::existsInDefaultsTable(const QString &interfaceID)
{
    QSqlDatabase sqlDatabase = QSqlDatabase::database(database.m_connectionName);
    QSqlQuery query(sqlDatabase);
    QString statement("SELECT InterfaceID From Defaults "
                    "WHERE InterfaceID = ?");
    QList<QVariant> bindValues;
    bindValues.append(interfaceID);
    database.executeQuery(&query, statement, bindValues);

    if (query.next())
        return true;
    else
        return false;
}

// Two small helper functions to assist with passing the security token to the servicedatabase.
// Do not use this function if you intentionally want to provide emtpy securityToken,
// but rather call database directly.
bool ServiceDatabaseUnitTest::registerService(const ServiceMetaDataResults &service, const QString &securityToken)
{
#ifdef QT_SFW_SERVICEDATABASE_USE_SECURITY_TOKEN
    if (securityToken.isEmpty()) {
        return database.registerService(service, securityTokenOwner);
    } else {
        return database.registerService(service, securityToken);
    }
#else
    Q_UNUSED(securityToken);
    return database.registerService(service);
#endif
}

bool ServiceDatabaseUnitTest::unregisterService(const QString &serviceName, const QString &securityToken)
{
#ifdef QT_SFW_SERVICEDATABASE_USE_SECURITY_TOKEN
    if (securityToken.isEmpty()) {
        return database.unregisterService(serviceName, securityTokenOwner);
    } else {
        return database.unregisterService(serviceName, securityToken);
    }
#else
    Q_UNUSED(securityToken);
    return database.unregisterService(serviceName);
#endif
}

void ServiceDatabaseUnitTest::securityTokens() {
#ifndef QT_SFW_SERVICEDATABASE_USE_SECURITY_TOKEN
    QSKIP("Security tokens are not enabled.");
#endif
    // Clear databases just in case
    database.close();
    QFile::remove(database.databasePath());
    QDir testdir = QDir(QFINDTESTDATA("testdata"));
    QVERIFY(database.open());
    database.m_databasePath = QDir::toNativeSeparators(QDir::currentPath().append("/services.db"));

    // Declare and setup testdata
    ServiceMetaData parser(testdir.absoluteFilePath("ServiceAcme.xml"));
    QVERIFY(parser.extractMetadata());

    // Actual teststeps
    qDebug("---------- 1. Add and remove with same security token. (OK)");
    QVERIFY(registerService(parser.parseResults(), securityTokenOwner));
    QVERIFY(unregisterService("acme", securityTokenOwner));

    qDebug("---------- 2. Add and remove with empty security token. (NOK)");
    QVERIFY(!database.registerService(parser.parseResults()));
    QCOMPARE(database.lastError().code(), DBError::NoWritePermissions);
    QVERIFY(!database.unregisterService("acme"));
    QCOMPARE(database.lastError().code(), DBError::NoWritePermissions);

    qDebug("---------- 3. Add, then remove with different security token. (NOK)");
    QVERIFY(registerService(parser.parseResults(), securityTokenOwner));
    QVERIFY(!unregisterService("acme", securityTokenStranger));
    QCOMPARE(database.lastError().code(), DBError::NoWritePermissions);
    QVERIFY(unregisterService("acme", securityTokenOwner));

    qDebug("---------- 4. Add namesake but but differently located service with same security token. (OK)");
    QStringList xmlFilesNamesakeServices;
    xmlFilesNamesakeServices << "ServiceDharma_Swan.xml" << "ServiceDharma_Pearl.xml";
    foreach(const QString &file, xmlFilesNamesakeServices) {
        parser.setDevice(new QFile(testdir.absoluteFilePath(file)));
        QVERIFY(parser.extractMetadata());
        QVERIFY(registerService(parser.parseResults(), securityTokenOwner));
    }
    QVERIFY(!database.getServiceNames(QString()).isEmpty());
    QVERIFY(unregisterService("DharmaInitiative", securityTokenOwner));
    QVERIFY(database.getServiceNames(QString()).isEmpty());

    qDebug("---------- 5. Add namesake but differently located services with different security token. (NOK)");
    parser.setDevice(new QFile(testdir.absoluteFilePath("ServiceDharma_Swan.xml")));
    QVERIFY(parser.extractMetadata());
    QVERIFY(registerService(parser.parseResults(), securityTokenOwner));
    parser.setDevice(new QFile(testdir.absoluteFilePath("ServiceDharma_Pearl.xml")));
    QVERIFY(parser.extractMetadata());
    QVERIFY(!registerService(parser.parseResults(), securityTokenStranger));
    QCOMPARE(database.lastError().code(), DBError::NoWritePermissions);
    QVERIFY(unregisterService("DharmaInitiative", securityTokenOwner));
}

void ServiceDatabaseUnitTest::cleanupTestCase()
{
    database.close();
    QFile::remove(database.databasePath());
}
QTEST_MAIN(ServiceDatabaseUnitTest)

#include "tst_servicedatabase.moc"
