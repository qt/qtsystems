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

#include <qservicemanager.h>
#include <qremoteserviceregister.h>

#include <QtTest/QtTest>
#include <QtCore>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QPair>
#include <QTimer>

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
    QString xmlFilename = QFINDTESTDATA("xmldata/testdeletion.xml");
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
