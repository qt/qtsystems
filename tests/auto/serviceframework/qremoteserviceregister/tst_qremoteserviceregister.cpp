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

#include <QCoreApplication>
#include "qservicemanager.h"
#include "qservicefilter.h"
#include "service.h"
#include <QTimer>
#include <QMetaObject>
#include <QMetaMethod>
#include <QtTest/QtTest>
#include <qservice.h>
#include <qremoteserviceregister.h>
#include <QDebug>
#include <QByteArray>
#include <QDataStream>

QT_USE_NAMESPACE
Q_DECLARE_METATYPE(QServiceFilter);
Q_DECLARE_METATYPE(QVariant);
Q_DECLARE_METATYPE(QList<QString>);


class tst_QRemoteServiceRegister: public QObject
{
    Q_OBJECT
public:

private slots:
    void initTestCase();
    void cleanupTestCase();
    void checkCreateEntryWithEmptyServiceName();
    void checkOperators();
    void checkPublish();
    void tst_instanceClosed();

private:
    QRemoteServiceRegister* serviceRegister;
    QRemoteServiceRegister::Entry uniqueEntry;
    QRemoteServiceRegister::Entry uniqueEntry2;

    QObject *connectToService(const QString &serviceName);
    bool servicePublished;
};

void mySecurityFilterFunction(QServiceClientCredentials *cred)
{
    // allow the superuser
    if (cred->getUserIdentifier() == 0) {
        cred->setClientAccepted(false);
    }

    cred->setClientAccepted(true);
}

void alwaysPass(QServiceClientCredentials *cred)
{
    cred->setClientAccepted(true);
}

void tst_QRemoteServiceRegister::initTestCase()
{
    qRegisterMetaType<QServiceFilter>("QServiceFilter");
    qRegisterMetaTypeStreamOperators<QServiceFilter>("QServiceFilter");
    qRegisterMetaType<QList<QString> >("QList<QString>");
    qRegisterMetaTypeStreamOperators<QList<QString> >("QList<QString>");

    serviceRegister = new QRemoteServiceRegister();

    //Check setting of close on last instance
//    serviceRegister->setQuitOnLastInstanceClosed(false);
//    QVERIFY(serviceRegister->quitOnLastInstanceClosed() == false);
    serviceRegister->setQuitOnLastInstanceClosed(true);
    QVERIFY(serviceRegister->quitOnLastInstanceClosed() == true);

    //check setting a security filter
    serviceRegister->setSecurityFilter(mySecurityFilterFunction);

    QServiceManager* manager = new QServiceManager(this);
    const QString path = QFINDTESTDATA("/xmldata/rsrexampleservice.xml");
    bool r = manager->addService(path);
    QVERIFY2(r, qPrintable(QString("Cannot register RSRExampleService - %1").arg(path)));

    // D-Bus auto registration
#ifndef QT_NO_DBUS
    const QString &file = QDir::homePath() + "/.local/share/dbus-1/services/" +
                                             "com.nokia.qt.rsrunittest.service";
    QFile data(file);
    if (data.open(QFile::WriteOnly)) {
        QTextStream out(&data);
        out << "[D-BUS Service]\n"
            << "Name=com.nokia.qtmobility.sfw.RSRExampleService" << '\n'
            << "Exec=" << QFileInfo("./qt_sfw_example_rsr_unittest").absoluteFilePath();
        data.close();
    }
#endif

    //register the unique service
    uniqueEntry = serviceRegister->createEntry<QRemoteServiceRegisterService>(
                "RSRExampleService", "com.nokia.qt.rsrunittest", "1.0");

    bool valid = uniqueEntry.isValid();
    QVERIFY(valid == true);

    uniqueEntry2 = serviceRegister->createEntry<QRemoteServiceRegisterService>(
                "RSRExampleService", "com.nokia.qt.rsrunittest", "1.0");

    valid = uniqueEntry2.isValid();
    QVERIFY(valid == true);

    servicePublished = false;
}

void tst_QRemoteServiceRegister::cleanupTestCase()
{
#ifndef QT_NO_DBUS
    const QString &file = QDir::homePath() + "/.local/share/dbus-1/services/" +
                                             "com.nokia.qt.rsrunittest.service";
    QFile::remove(file);
#endif

    // clean up the unit, don't leave it registered
    QServiceManager m;
    m.removeService("RSRExampleService");
    delete serviceRegister;
}

void tst_QRemoteServiceRegister::checkCreateEntryWithEmptyServiceName()
{
    QRemoteServiceRegister::Entry emptyservicename =
                serviceRegister->createEntry<QRemoteServiceRegisterService>(
                "", "", "");
    QVERIFY(emptyservicename.serviceName() == "");
    bool valid = emptyservicename.isValid();
    QVERIFY(valid == false);
}

void tst_QRemoteServiceRegister::checkOperators()
{
    //== operator
    bool equal = (uniqueEntry == uniqueEntry2 ? true : false);
    QVERIFY(equal == true);

    //!= operator
    bool notequal = (uniqueEntry != uniqueEntry2 ? true : false);
    QVERIFY(notequal == false);

    //= operator
    QRemoteServiceRegister::Entry assignval;
    assignval = uniqueEntry;
    equal = (assignval == uniqueEntry ? true : false);
    QVERIFY(equal == true);

    //QDataStream << >>
#ifndef QT_NO_DATASTREAM
    QByteArray barray = QByteArray();
    QDataStream streamOut(&barray, QIODevice::WriteOnly);
    streamOut.setVersion(QDataStream::Qt_4_6);
    streamOut << uniqueEntry;
    QDataStream streamIn(&barray, QIODevice::ReadOnly);
    streamOut.setVersion(QDataStream::Qt_4_6);
    QRemoteServiceRegister::Entry streamedentry;
    streamIn >> streamedentry;
    QVERIFY(uniqueEntry.serviceName() == streamedentry.serviceName());
    QVERIFY(uniqueEntry.interfaceName() == streamedentry.interfaceName());
    QVERIFY(uniqueEntry.version() == streamedentry.version());
#endif
}

void tst_QRemoteServiceRegister::checkPublish()
{
    //publish the registered services
    serviceRegister->publishEntries("qt_sfw_example_rsr_unittest");
    servicePublished = true;

    //check instantiation type
    //- default value
    QRemoteServiceRegister::InstanceType type = uniqueEntry.instantiationType();
    QRemoteServiceRegister::InstanceType type2 = uniqueEntry2.instantiationType();
    QVERIFY(type == QRemoteServiceRegister::PrivateInstance);
    QVERIFY(type2 == QRemoteServiceRegister::PrivateInstance);
    //check setting the type
    uniqueEntry2.setInstantiationType(QRemoteServiceRegister::GlobalInstance);
    type2 = uniqueEntry2.instantiationType();
    QVERIFY(type2 == QRemoteServiceRegister::GlobalInstance);
}

Q_DECLARE_METATYPE(QRemoteServiceRegister::Entry);

void tst_QRemoteServiceRegister::tst_instanceClosed()
{
    qRegisterMetaType<QRemoteServiceRegister::Entry>("QRemoteServiceRegister::Entry");
    if(!servicePublished)
        serviceRegister->publishEntries("qt_sfw_example_rsr_unittest");

    serviceRegister->setSecurityFilter(alwaysPass);
    QSignalSpy spy(serviceRegister,SIGNAL(instanceClosed(QRemoteServiceRegister::Entry)));
    QSignalSpy spyAll(serviceRegister,SIGNAL(allInstancesClosed()));

    QObject *o = connectToService("RSRExampleService");
    QVERIFY(o);

    delete o;

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyAll.count(), 1);

}

QObject *tst_QRemoteServiceRegister::connectToService(const QString &serviceName)
{
    QServiceManager manager;

    QList<QServiceInterfaceDescriptor> list = manager.findInterfaces(serviceName);
    if (list.isEmpty()) {
        qWarning() << "Couldn't find service" << serviceName << manager.findServices("qt_sfw_example_rsr_unittest");
        return 0;
    }

    // Get the interface descriptor
    QServiceInterfaceDescriptor desc = list.at(0);
    if (!desc.isValid()) {
        qWarning() << "Warning: Invalid service interface descriptor for" << serviceName;
        return 0;
    }

    QObject* service = manager.loadInterface(desc);
    if (!service) {
        qWarning() << "Couldn't load service interface for" << serviceName;
        return 0;
    }
    return service;
}


QTEST_MAIN(tst_QRemoteServiceRegister);
#include "tst_qremoteserviceregister.moc"
