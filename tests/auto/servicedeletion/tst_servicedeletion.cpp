/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include <qservicemanager.h>
#include <qremoteserviceregister.h>

#include <QtTest/QtTest>
#include <QtCore>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QPair>
#include <QTimer>

#define QTRY_COMPARE(a,e)                       \
    for (int _i = 0; _i < 5000; _i += 100) {    \
        if ((a) == (e)) break;                  \
        QTest::qWait(100);                      \
    }                                           \
    QCOMPARE(a, e)

#define QTRY_VERIFY(a)                       \
    for (int _i = 0; _i < 5000; _i += 100) {    \
        if (a) break;                  \
        QTest::qWait(100);                      \
    }                                           \
    QVERIFY(a)

#define PRINT_ERR(a) qPrintable(QString("error = %1").arg(a.error()))
QT_USE_NAMESPACE

class TestService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 time READ time WRITE setTime NOTIFY timeChanged)

public:
    explicit TestService(QObject *parent = 0)
        : QObject(parent), m_time(42)
    {
    }

    ~TestService()
    {
    }

    Q_INVOKABLE void setTime(qint64 time) { m_time = time; emit timeChanged(); }
    qint64 time() const {
        return m_time;
    }

signals:
    void timeChanged();

private:
    qint64 m_time;
};

static int sigs = 0;

class tst_QServiceDeletion: public QObject
{
    Q_OBJECT

private:
    QObject *connectToService(const QString &serviceName);

private slots:
    void initTestCase();

    void publishService();

    void cleanupTestCase();

public slots:
    void error(QService::UnrecoverableIPCError);
    void timeChanged();

};

void tst_QServiceDeletion::initTestCase()
{

}

void tst_QServiceDeletion::cleanupTestCase()
{

}

void tst_QServiceDeletion::publishService()
{
    QRemoteServiceRegister *serviceRegister = new QRemoteServiceRegister(this);

    QString serviceName = "TestService";
    QString xmlFilename = QString(SRCDIR) + "/xmldata/testdeletion.xml";
    QString interfaceName = "com.nokia.test.services.TestService";
    QString interfaceVersion = "1.0";
    QString ipcAddress = "test_service";

    QServiceManager serviceManager(QService::UserScope);
    serviceManager.removeService(serviceName);

    QRemoteServiceRegister::Entry entry =
            serviceRegister->createEntry<TestService>(
                serviceName,
                interfaceName,
                interfaceVersion);
    entry.setInstantiationType(QRemoteServiceRegister::GlobalInstance);

    QVERIFY(serviceManager.addService(xmlFilename));

    serviceRegister->publishEntries(ipcAddress);

    // Two connections are created and one of them is deleted later
    QObject *conn1 = connectToService(serviceName);
    QVERIFY(conn1);
    QObject *conn2 = connectToService(serviceName);
    QVERIFY(conn2);

    connect(conn1, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)),
            this, SLOT(error(QService::UnrecoverableIPCError)));
    connect(conn2, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)),
            this, SLOT(error(QService::UnrecoverableIPCError)));
    connect(conn1, SIGNAL(timeChanged()),
            this, SLOT(timeChanged()));
    connect(conn2, SIGNAL(timeChanged()),
             this, SLOT(timeChanged()));

    QCOMPARE(conn1->property("time").toInt(), 42);
    QCOMPARE(conn2->property("time").toInt(), 42);

    QVERIFY(QMetaObject::invokeMethod(conn2, "setTime", Q_ARG(qint64, 21)));

    QTRY_VERIFY(sigs == 2);
    QCOMPARE(conn2->property("time").toInt(), 21);

    delete conn2;
    QCOMPARE(conn1->property("time").toInt(), 21);

    sigs = 0;
    QVERIFY(QMetaObject::invokeMethod(conn1, "setTime", Q_ARG(qint64, 42)));
    QTRY_VERIFY(sigs == 1);
    QCOMPARE(conn1->property("time").toInt(), 42);


    delete conn1;
    serviceManager.removeService(serviceName);
}

QObject *tst_QServiceDeletion::connectToService(const QString &serviceName)
{
    QServiceManager manager;
    QList<QServiceInterfaceDescriptor> list = manager.findInterfaces(serviceName);
    if (list.isEmpty()) {
        qWarning() << "Couldn't find service" << serviceName;
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

void tst_QServiceDeletion::error(QService::UnrecoverableIPCError)
{
    qDebug() << "Error received from IPC";
}

void tst_QServiceDeletion::timeChanged()
{
    qDebug() << "time changed";
    sigs++;
}

QTEST_MAIN(tst_QServiceDeletion)

#include "tst_servicedeletion.moc"
