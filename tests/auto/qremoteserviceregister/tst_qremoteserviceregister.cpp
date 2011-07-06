/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#define QTRY_VERIFY(a)                       \
    for (int _i = 0; _i < 5000; _i += 100) {    \
        if (a) break;                  \
        QTest::qWait(100);                      \
    }                                           \
    QVERIFY(a)

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

bool mySecurityFilterFunction(const void *p)
{
    const QRemoteServiceRegisterCredentials *cred = (const struct QRemoteServiceRegisterCredentials *)p;

    // allow the superuser
    if (cred->uid == 0)
        return true;

    return false;
}

bool alwaysPass(const void * /*p*/)
{
    return true;
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

    // Symbian has auto registration
#ifndef Q_OS_SYMBIAN
    const QString path = QString(SRCDIR) + "/xmldata/rsrexampleservice.xml";
    bool r = manager->addService(path);
    QVERIFY2(r, qPrintable(QString("Cannot register RSRExampleService - %1").arg(path)));
#endif

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
    QSKIP("This test does not pass yet", SkipAll);

    qRegisterMetaType<QRemoteServiceRegister::Entry>("QRemoteServiceRegister::Entry");
    if(!servicePublished)
        serviceRegister->publishEntries("qt_sfw_example_rsr_unittest");

    serviceRegister->setSecurityFilter(alwaysPass);
    QSignalSpy spy(serviceRegister,SIGNAL(instanceClosed(QRemoteServiceRegister::Entry)));
    QSignalSpy spyAll(serviceRegister,SIGNAL(allInstancesClosed()));

    QObject *o = connectToService("RSRExampleService");
    QVERIFY(o);

    delete o;

    QCOMPARE(spy.count(), 1);
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
