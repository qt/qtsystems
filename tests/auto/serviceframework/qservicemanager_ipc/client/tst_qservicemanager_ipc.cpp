
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
#include <QTimer>
#include <QMetaObject>
#include <QMetaMethod>
#include <QThread>
#include <QtTest/QtTest>
#include <qservice.h>
#include <qremoteserviceregister.h>
#include <qservicereply.h>

#include <signal.h>

#ifdef SFW_USE_DBUS_BACKEND
#include <QtDBus/QtDBus>
#if QT_VERSION < 0x040800
inline QDBusArgument &operator<<(QDBusArgument &arg, const QVariantHash &map)
{
    arg.beginMap(QVariant::String, qMetaTypeId<QDBusVariant>());
    QVariantHash::ConstIterator it = map.constBegin();
    QVariantHash::ConstIterator end = map.constEnd();
    for ( ; it != end; ++it) {
        arg.beginMapEntry();
        arg << it.key() << QDBusVariant(it.value());
        arg.endMapEntry();
    }
    arg.endMap();
    return arg;
}
#endif
#endif


#ifdef Q_OS_MAC
#include <stdlib.h>
#include <sys/resource.h>
#endif

QT_USE_NAMESPACE
Q_DECLARE_METATYPE(QServiceFilter);
Q_DECLARE_METATYPE(QVariant);
Q_DECLARE_METATYPE(QList<QString>);
Q_DECLARE_METATYPE(QVariantHash);

char serviceReuqestXml[] =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<SFW version=\"1.1\">"
        "<service>"
            "<name>com.nokia.mt.processmanager</name>"
            "<ipcaddress>com.nokia.mt.processmanager.ServiceRequestSocket</ipcaddress>"
            "<interface>"
                "<name>com.nokia.mt.processmanager.ProcessManagerController</name>"
                "<version>1.0</version>"
            "</interface>"
        "</service>"
        "</SFW>";

char serviceReuqestXmlFake[] =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<SFW version=\"1.1\">"
        "<service>"
            "<name>com.nokia.qt.does.not.exist</name>"
            "<ipcaddress>com.nokia.qt.does.not.exist</ipcaddress>"
            "<interface>"
                "<name>com.nokia.qt.interface.does.not.exist</name>"
                "<version>1.0</version>"
            "</interface>"
        "</service>"
        "</SFW>";

char serviceReuqestXmlStarted[] =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<SFW version=\"1.1\">"
        "<service>"
            "<name>com.nokia.qt.tests.autostart</name>"
            "<ipcaddress>com.nokia.qt.tests.autostart</ipcaddress>"
            "<interface>"
                "<name>com.nokkia.qt.tests.autostarted</name>"
                "<version>1.0</version>"
            "</interface>"
        "</service>"
        "</SFW>";


class ServiceRequest : public QObject
{
    Q_OBJECT
public:
    ServiceRequest(QObject *o = 0) : QObject(o)
    {
    }

    Q_INVOKABLE void startService(const QString location)
    {
        qDebug() << "Got startService";
        if (location == "com.nokia.qt.tests.autostart") {
            QMetaObject::invokeMethod(this, "sendOk", Qt::DirectConnection, Q_ARG(QString, location));
        } else {
            QMetaObject::invokeMethod(this, "sendError", Qt::DirectConnection, Q_ARG(QString, location));
        }
    }

protected slots:
    void sendError(const QString &location)
    {
        qDebug() << "Send fail";
        emit failed(QStringLiteral("Test code failing creation"), location);
    }

    void sendOk(const QString &location)
    {
        emit launched(location);
        QMetaObject::invokeMethod(this, "startNewService", Qt::DirectConnection);
    }

    void startNewService()
    {
        QRemoteServiceRegister *r = new QRemoteServiceRegister(this);

        QRemoteServiceRegister::Entry pmEntry =
                r->createEntry<ServiceRequest>(
                    QStringLiteral("com.nokia.qt.tests.autostart"),
                    QStringLiteral("com.nokkia.qt.tests.autostarted"),
                    QStringLiteral("1.0"));

        r->publishEntries(QStringLiteral("com.nokia.qt.tests.autostart"));
    }

signals:

    void launched(QString process);
    void failed(QString error, QString process);

};

class tst_QServiceManager_IPC: public QObject
{
    Q_OBJECT
public:
    tst_QServiceManager_IPC();

protected slots:
    void ipcError(QService::UnrecoverableIPCError error);
    void ipcErrorNonTest(QService::UnrecoverableIPCError error);
    void unblockRemote();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void verifyIsServiceRunning();

    void verifySharedServiceObject(); //rough count
    void verifySharedMethods();
    void verifySharedMethods_data();
    void verifySharedProperties();
    void verifySharedProperties_data();

    void verifyUniqueServiceObject(); //rough count
    void verifyUniqueMethods();
    void verifyUniqueMethods_data();

    void verifyUniqueProperties();
    void verifyUniqueProperties_data();

    void verifyUniqueClassInfo();
    void verifyUniqueClassInfo_data();

    void verifyUniqueEnumerator();
    void verifyUniqueEnumerator_data();

    void verifyLargeDataTransfer();
    void verifyLargeDataTransfer_data();

    void verifyRemoteBlockingFunctions();

    void verifyThreadSafety();
    void verifyThreadSafety_data();

    void sharedTestService();
    void uniqueTestService();

    void testInvokableFunctions();
    void testSlotInvokation();
    void testSignalling();

    void testSignalSlotOrdering();

    void verifyServiceClass();
    void verifyFailures();

    void verifyAsyncLoading();
    void verifyAsyncLoading_data();

    void testServiceSecurity();

    void testProcessLaunch();

    void testIpcFailure();

signals:
    void threadReady();
    void threadRun();
    void threadFinished();
    void threadError();
    void threadRemaining();

private:
    QObject* serviceUnique;
    QObject* serviceUniqueOther;
    QObject* serviceShared;
    QObject* serviceSharedOther;
    QObject* miscTest;
    QServiceManager* manager;
    bool verbose;
    bool ipcfailure;
};

tst_QServiceManager_IPC::tst_QServiceManager_IPC()
    : serviceUnique(0)
    , serviceUniqueOther(0)
    , serviceShared(0)
    , serviceSharedOther(0)
    , miscTest(0)
    , manager(0)
    , verbose(false)
    , ipcfailure(false)
{
}

#ifdef Q_OS_WIN
static const char serviceBinaryC[] = "qt_sfw_example_ipc_unittest.exe";
#else
static const char serviceBinaryC[] = "qt_sfw_example_ipc_unittest";
#endif

void tst_QServiceManager_IPC::initTestCase()
{
    const QString serviceBinary = QFINDTESTDATA(serviceBinaryC);
    QVERIFY(!serviceBinary.isEmpty());
    const QFileInfo serviceBinaryInfo(serviceBinary);
    QVERIFY(serviceBinaryInfo.isExecutable());
    const QString serviceBinaryAbsPath = serviceBinaryInfo.absoluteFilePath();

    const QString path = QFINDTESTDATA("xmldata/ipcexampleservice.xml");
    QVERIFY(!path.isEmpty());

    //verbose = true;
    ipcfailure = false;
    verbose = false;
    serviceUnique = 0;
    serviceUniqueOther = 0;
    serviceSharedOther = 0;
    serviceShared = 0;
    serviceSharedOther = 0;
    miscTest = 0;
    qRegisterMetaType<QServiceFilter>("QServiceFilter");
    qRegisterMetaTypeStreamOperators<QServiceFilter>("QServiceFilter");
    qRegisterMetaType<QList<QString> >("QList<QString>");
    qRegisterMetaTypeStreamOperators<QList<QString> >("QList<QString>");
    qRegisterMetaTypeStreamOperators<QVariantHash>("QVariantHash");
    qRegisterMetaType<QVariantHash>("QVariantHash");
#ifdef SFW_USE_DBUS_BACKEND
    qDBusRegisterMetaType<QVariantHash>();
#endif

#ifdef Q_OS_MAC
    struct rlimit rlp;
    int ret = getrlimit(RLIMIT_NOFILE, &rlp);
    if (ret == -1) {
        qWarning() << "Failed to get fd limit on mac, expect the threading test to fail";
    } else {
        rlp.rlim_cur = 512;
        ret = setrlimit(RLIMIT_NOFILE, &rlp);
        if (ret == -1) {
            qWarning() << "Failed to raise the number of open files to 512, expect the threading test to fail";
        }
    }
#endif

    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());

    QServiceManager* manager = new QServiceManager(this);

    const bool r = manager->addService(path);
    if (!r)
        qWarning() << "Cannot register IPCExampleService" << path;

#ifdef SFW_USE_DBUS_BACKEND
    // D-Bus auto registration
    const QString &file = QDir::homePath() + "/.local/share/dbus-1/services/" +
                                   "com.nokia.qt.ipcunittest.service";

    QDir dir(QDir::homePath());
    dir.mkpath(".local/share/dbus-1/services/");
    QFile data(file);
    if (data.open(QFile::WriteOnly)) {
        QTextStream out(&data);
        out << "[D-BUS Service]\n"
            << "Name=com.nokia.qtmobility.sfw.IPCExampleService" << '\n'
            << "Exec=" << serviceBinaryAbsPath << '\n';
        data.close();
    }
    QVERIFY(data.exists());
#endif // SFW_USE_DBUS_BACKEND

#ifdef Q_OS_UNIX
    char *p = getenv("PATH");
    char newpath[strlen(p)+3];
    strcpy(newpath, p);
    strcat(newpath, ":.");
    setenv("PATH", newpath, 1);
#endif

    //test that the service is installed
    QList<QServiceInterfaceDescriptor> list = manager->findInterfaces("IPCExampleService");
    QVERIFY2(list.count() >= 7,"unit test specific IPCExampleService not registered/found" );
    QServiceInterfaceDescriptor d;
    foreach (d, list) {
        if (d.majorVersion() == 3 && d.minorVersion() == 5) {
            serviceUnique = manager->loadInterface(d);
            QVERIFY(serviceUnique);
            serviceUniqueOther = manager->loadInterface(d);
            QVERIFY(serviceUniqueOther);
            connect(serviceUnique, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)),
                    this, SLOT(ipcErrorNonTest(QService::UnrecoverableIPCError)));
            connect(serviceUniqueOther, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)),
                    this, SLOT(ipcErrorNonTest(QService::UnrecoverableIPCError)));

        }
        if (d.majorVersion() == 3 && d.minorVersion() == 4) {
            serviceShared = manager->loadInterface(d);
            QVERIFY(serviceShared);
            serviceSharedOther = manager->loadInterface(d);
            connect(serviceShared, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)),
                    this, SLOT(ipcErrorNonTest(QService::UnrecoverableIPCError)));
            QVERIFY(serviceSharedOther);
            connect(serviceSharedOther, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)),
                    this, SLOT(ipcErrorNonTest(QService::UnrecoverableIPCError)));

        }
        if (d.majorVersion() == 3 && d.minorVersion() == 8) {
            miscTest = manager->loadInterface(d);
            QVERIFY(miscTest);
            connect(miscTest, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)),
                    this, SLOT(ipcErrorNonTest(QService::UnrecoverableIPCError)));
        }

    }

    QString errorCode = "Cannot find service. Error: %1";
    errorCode = errorCode.arg(manager->error());
    QVERIFY2(serviceUnique,errorCode.toLatin1());
    QVERIFY2(serviceUniqueOther,errorCode.toLatin1());
    QVERIFY2(serviceShared,errorCode.toLatin1());
    QVERIFY2(serviceSharedOther,errorCode.toLatin1());

    // all objects come from the same service, just need to connect to 1 signal
    connect(serviceSharedOther, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)), this, SLOT(ipcError(QService::UnrecoverableIPCError)));

    QRemoteServiceRegister *reg = new QRemoteServiceRegister();

    QRemoteServiceRegister::Entry pmEntry =
            reg->createEntry<ServiceRequest>(
                QStringLiteral("com.nokia.mt.processmanager"),
                QStringLiteral("com.nokia.mt.processmanager.ServiceRequest"),
                QStringLiteral("1.0"));

    reg->publishEntries("com.nokia.mt.processmanager.ServiceRequestSocket");

    QByteArray ba(serviceReuqestXml);
    QBuffer b(&ba);
    manager->removeService(QStringLiteral("com.nokia.mt.processmanager"));
    manager->addService(&b);

    QByteArray baFake(serviceReuqestXmlFake);
    QBuffer bFake(&baFake);
    manager->removeService(QStringLiteral("com.nokia.qt.does.not.exists"));
    manager->addService(&bFake);

    QByteArray baAutoStart(serviceReuqestXmlStarted);
    QBuffer bAutoStart(&baAutoStart);
    manager->removeService(QStringLiteral("com.nokia.qt.tests.autostart"));
    manager->addService(&bAutoStart);

}

void tst_QServiceManager_IPC::ipcError(QService::UnrecoverableIPCError err)
{
    Q_UNUSED(err);
    ipcfailure = true;
}

// unexpected ipc failures
void tst_QServiceManager_IPC::ipcErrorNonTest(QService::UnrecoverableIPCError err)
{
    Q_UNUSED(err);
}

void tst_QServiceManager_IPC::cleanupTestCase()
{
#ifdef SFW_USE_DBUS_BACKEND
    const QString &file = QDir::homePath() + "/.local/share/dbus-1/services/" +
                                             "com.nokia.qt.ipcunittest.service";
    QFile::remove(file);
#endif

    if (serviceUnique) {
        delete serviceUnique;
    }

    if (serviceUniqueOther) {
        delete serviceUniqueOther;
    }

    if (serviceShared) {
        delete serviceShared;
    }

    if (serviceSharedOther) {
        delete serviceSharedOther;
    }

    // clean up the unit, don't leave it registered
    QServiceManager m;
    m.removeService("IPCExampleService");
    m.removeService(QStringLiteral("com.nokia.mt.processmanager"));
    m.removeService(QStringLiteral("com.nokia.qt.does.not.exists"));
    m.removeService(QStringLiteral("com.nokia.qt.tests.autostart"));
}

void tst_QServiceManager_IPC::init()
{
}

void tst_QServiceManager_IPC::cleanup()
{
}

void tst_QServiceManager_IPC::verifyIsServiceRunning()
{
#if defined(Q_OS_UNIX) && !defined(SFW_USE_DBUS_BACKEND)

   QServiceManager m;

   QCOMPARE(m.isInterfaceRunning("com.nokia.qt.ipcunittest"), true);
   QCOMPARE(m.isInterfaceRunning("com.nokis.qt.ipcunittest.that.does.not.exsist.and.should.not.be.created"), false);

   QList<QServiceInterfaceDescriptor> list = m.findInterfaces("IPCExampleService");
   QServiceInterfaceDescriptor d;
   foreach (d, list) {
       QCOMPARE(m.isInterfaceRunning(d), true);
   }
#else
    QSKIP("isInterfaceRunning needs more testing on supported platforms");
#endif
}

void tst_QServiceManager_IPC::sharedTestService()
{
    const QMetaObject *meta = serviceShared->metaObject();
    const QMetaObject *metaOther = serviceSharedOther->metaObject();

    // test changes on the property 'value'
    QMetaProperty prop = meta->property(1);
    QMetaProperty propOther = metaOther->property(1);

    // check that the property values are the same before and after an edit
    QCOMPARE(prop.read(serviceShared), propOther.read(serviceSharedOther));
    prop.write(serviceShared, "Shared");
    // write does not block so the QCOMPARE below may result in serviceSharedOther being read before
    // the new value has been sent to the service. To avoid this, read the property back now
    (void)prop.read(serviceShared); // flush
    QCOMPARE(prop.read(serviceShared), propOther.read(serviceSharedOther));
    prop.reset(serviceShared);

    // test changes through a method call
    uint hash, hashOther;
    QMetaObject::invokeMethod(serviceShared, "setConfirmationHash",
                              Q_ARG(uint, 0));
    QMetaObject::invokeMethod(serviceShared, "slotConfirmation",
                              Q_RETURN_ARG(uint, hash));
    QMetaObject::invokeMethod(serviceSharedOther, "setConfirmationHash",
                              Q_ARG(uint, 0));
    QMetaObject::invokeMethod(serviceSharedOther, "slotConfirmation",
                              Q_RETURN_ARG(uint, hashOther));
    QCOMPARE(hash, (uint)0);
    QCOMPARE(hashOther, (uint)0);

    // check that the method values are the same after an edit
    QMetaObject::invokeMethod(serviceShared, "setConfirmationHash",
                              Q_ARG(uint, 1));
    QMetaObject::invokeMethod(serviceShared, "slotConfirmation",
                              Q_RETURN_ARG(uint, hash));
    QMetaObject::invokeMethod(serviceSharedOther, "slotConfirmation",
                              Q_RETURN_ARG(uint, hashOther));
    QCOMPARE(hash, (uint)1);
    QCOMPARE(hashOther, (uint)1);
}

void tst_QServiceManager_IPC::uniqueTestService()
{
    const QMetaObject *meta = serviceUnique->metaObject();
    const QMetaObject *metaOther = serviceUniqueOther->metaObject();

    // test changes on the property 'value'
    QMetaProperty prop = meta->property(1);
    QMetaProperty propOther = metaOther->property(1);

    // check that the property values are not the same after an edit
    QCOMPARE(prop.read(serviceUnique), propOther.read(serviceUniqueOther));
    prop.write(serviceUnique, "Unique");
    QVERIFY(prop.read(serviceUnique) != propOther.read(serviceUniqueOther));
    prop.reset(serviceUnique);

    // test changes through a method call
    uint hash, hashOther;
    QMetaObject::invokeMethod(serviceUnique, "setConfirmationHash",
                              Q_ARG(uint, 0));
    QMetaObject::invokeMethod(serviceUnique, "slotConfirmation",
                              Q_RETURN_ARG(uint, hash));
    QMetaObject::invokeMethod(serviceUniqueOther, "setConfirmationHash",
                              Q_ARG(uint, 0));
    QMetaObject::invokeMethod(serviceUniqueOther, "slotConfirmation",
                              Q_RETURN_ARG(uint, hashOther));
    QCOMPARE(hash, (uint)0);
    QCOMPARE(hashOther, (uint)0);

    // check that the method values are not the same after an edit
    QMetaObject::invokeMethod(serviceUnique, "setConfirmationHash",
                              Q_ARG(uint, 1));
    QMetaObject::invokeMethod(serviceUnique, "slotConfirmation",
                              Q_RETURN_ARG(uint, hash));
    QMetaObject::invokeMethod(serviceUniqueOther, "slotConfirmation",
                              Q_RETURN_ARG(uint, hashOther));
    QCOMPARE(hash, (uint)1);
    QCOMPARE(hashOther, (uint)0);
}

void tst_QServiceManager_IPC::verifySharedServiceObject()
{
    QVERIFY(serviceShared != 0);
    const QMetaObject* mo = serviceShared->metaObject();
    QCOMPARE(mo->className(), "SharedTestService");
    QVERIFY(mo->superClass());
    QCOMPARE(mo->superClass()->className(), "QServiceProxyBase");
    QCOMPARE(mo->methodCount()-mo-> methodOffset(), 19);
    QCOMPARE(mo->methodCount(), 25); //20 meta functions available
    //actual function presence will be tested later

//    for (int i = 0; i < mo->methodCount(); i++) {
//        qDebug() << "Methods" << i << mo->method(i).methodSignature();
//    }

    //test properties
    QCOMPARE(mo->propertyCount()-mo->propertyOffset(), 1);
    QCOMPARE(mo->propertyCount(), 2);

    QCOMPARE(mo->enumeratorCount()-mo->enumeratorOffset(), 0);
    QCOMPARE(mo->enumeratorCount(), 0);

    QCOMPARE(mo->classInfoCount()-mo->classInfoOffset(), 0);
    QCOMPARE(mo->classInfoCount(), 0);

    if (verbose) {
        qDebug() << "ServiceObject class: " << mo->className() << mo->superClass() << mo->superClass()->className();
        qDebug() << "------------------- Meta Methods -----------";
        qDebug() << "Methods:" << mo->methodCount()- mo->methodOffset() << "(" << mo->methodCount() << ")";
        for (int i=0; i< mo->methodCount(); i++) {
            QMetaMethod method = mo->method(i);
            QString type;
            switch (method.methodType()) {
            case QMetaMethod::Signal:
                type = "signal"; break;
            case QMetaMethod::Slot:
                type = "slot"; break;
            case QMetaMethod::Constructor:
                type = "constrcutor"; break;
            case QMetaMethod::Method:
                type = "method"; break;
            }
            qDebug() << "    " << i << "." << method.methodSignature() << type;
        }

        qDebug() << "------------------- Meta Properties --------";
        qDebug() << "Properties:" << mo->propertyCount()- mo->propertyOffset() << "(" << mo->propertyCount() << ")";
        for (int i=0; i< mo->propertyCount(); i++) {
            QMetaProperty property = mo->property(i);
            QString info = "Readable: %1 Resettable: %2 Writeable: %3 Designable: %4 Scriptable: %5 User: %6 Stored: %7 Constant: %8 Final: %9 HasNotify: %10 EnumType: %11 FlagType: %12";
            info = info.arg(property.isReadable()).arg(property.isResettable()).arg(property.isWritable());
            info = info.arg(property.isDesignable()).arg(property.isScriptable()).arg(property.isUser());
            info = info.arg(property.isStored()).arg(property.isConstant()).arg(property.isFinal());
            info = info.arg(property.hasNotifySignal()).arg(property.isEnumType()).arg(property.isFlagType());

            qDebug() << "    " << i << "." << property.name() << "Type:" << property.typeName() << info;
        }

        qDebug() << "------------------- Meta Enumerators --------";
        qDebug() << "Enums:" << mo->enumeratorCount()- mo->enumeratorOffset() << "(" << mo->enumeratorCount() << ")";

        qDebug() << "------------------- Meta class info ---------";
        qDebug() << "ClassInfos:" << mo->classInfoCount()- mo->classInfoOffset() << "(" << mo->classInfoCount() << ")";
    }
}

void tst_QServiceManager_IPC::verifySharedMethods_data()
{
    QTest::addColumn<QByteArray>("signature");
    QTest::addColumn<int>("metaMethodType");
    QTest::addColumn<QByteArray>("returnType");

    //list of all slots, signals and invokable functions
    QTest::newRow("signalWithIntParam(int)")
        << QByteArray("signalWithIntParam(int)") <<  (int)( QMetaMethod::Signal) << QByteArray("void");
    QTest::newRow("signalWithVariousParam(QVariant,QString,QServiceFilter)")
        << QByteArray("signalWithVariousParam(QVariant,QString,QServiceFilter)") <<  (int)( QMetaMethod::Signal) << QByteArray("void");
    QTest::newRow("valueChanged()")
        << QByteArray("valueChanged()") <<  (int)( QMetaMethod::Signal) << QByteArray("void");
    QTest::newRow("triggerSignalWithIntParam()")
        << QByteArray("triggerSignalWithIntParam()") <<  (int)( QMetaMethod::Slot) << QByteArray("void");
    QTest::newRow("triggerSignalWithVariousParam()")
        << QByteArray("triggerSignalWithVariousParam()") <<  (int)( QMetaMethod::Slot) << QByteArray("void");
    QTest::newRow("triggerSignalWithIntParamExecute()")
        << QByteArray("triggerSignalWithIntParamExecute()") <<  (int)( QMetaMethod::Slot) << QByteArray("void");
    QTest::newRow("triggerSignalWithVariousParamExecute()")
        << QByteArray("triggerSignalWithVariousParamExecute()") <<  (int)( QMetaMethod::Slot) << QByteArray("void");
    QTest::newRow("testSlot()")
        << QByteArray("testSlot()") <<  (int)( QMetaMethod::Slot) << QByteArray("void");
    QTest::newRow("testSlotWithArgs(QByteArray,int,QVariant)")
        << QByteArray("testSlotWithArgs(QByteArray,int,QVariant)") <<  (int)( QMetaMethod::Slot) << QByteArray("void");
    QTest::newRow("testSlotWithCustomArg(QServiceFilter)")
        << QByteArray("testSlotWithCustomArg(QServiceFilter)") <<  (int)( QMetaMethod::Slot) << QByteArray("void");
    QTest::newRow("testSlotWidthComplexArg(QVariantHash)")
            << QByteArray("testSlotWithComplexArg(QVariantHash)") <<  (int)( QMetaMethod::Slot) << QByteArray("void");

    //QServiceInterfaceDescriptor has not been declared as meta type
    QTest::newRow("testSlotWithUnknownArg(QServiceInterfaceDescriptor)")
        << QByteArray("testSlotWithUnknownArg(QServiceInterfaceDescriptor)") <<  (int)( QMetaMethod::Slot) << QByteArray("void");
    QTest::newRow("testFunctionWithReturnValue(int)")
        << QByteArray("testFunctionWithReturnValue(int)") <<  (int)( QMetaMethod::Method) << QByteArray("QString");
    QTest::newRow("testFunctionWithVariantReturnValue(QVariant)")
        << QByteArray("testFunctionWithVariantReturnValue(QVariant)") <<  (int)( QMetaMethod::Method) << QByteArray("QVariant");
    QTest::newRow("testFunctionWithCustomReturnValue()")
        << QByteArray("testFunctionWithCustomReturnValue()") <<  (int)( QMetaMethod::Method) << QByteArray("QServiceFilter");
    QTest::newRow("slotConfirmation()")
        << QByteArray("slotConfirmation()") <<  (int)( QMetaMethod::Method) << QByteArray("uint");
    QTest::newRow("setConfirmationHash(uint)")
        << QByteArray("setConfirmationHash(uint)") <<  (int)( QMetaMethod::Method) << QByteArray("void");
}

void tst_QServiceManager_IPC::verifySharedMethods()
{
    QVERIFY(serviceShared);
    QFETCH(QByteArray, signature);
    QFETCH(int, metaMethodType);
    QFETCH(QByteArray, returnType);

    const QMetaObject* meta = serviceShared->metaObject();
    const int index = meta->indexOfMethod(signature.constData());
    QVERIFY(index>=0);
    QMetaMethod method = meta->method(index);
    QCOMPARE(metaMethodType, (int)method.methodType());
    QCOMPARE(returnType, QByteArray(method.typeName()));
}

void tst_QServiceManager_IPC::verifySharedProperties_data()
{
    QTest::addColumn<QByteArray>("signature");
    QTest::addColumn<QString>("typeName");
    QTest::addColumn<qint16>("flags");
    QTest::addColumn<QVariant>("defaultValue");
    QTest::addColumn<QVariant>("writeValue");
    QTest::addColumn<QVariant>("expectedReturnValue");

    //
    //    bit order:
    //        0 isWritable - 1 isUser - 2 isStored - 3 isScriptable
    //        4 isResettable - 5 isReadable - 6 isFlagType - 7 isFinal
    //        8 isEnumType - 9 isDesignable - 10 isConstant - 11 hasNotifySgnal
    //
    //        for more details see verifyProperties()
    //

    QTest::newRow("value property") << QByteArray("value")
            << QString("QString") << qint16(0x0A3D) << QVariant(QString("FFF")) << QVariant(QString("GGG")) << QVariant(QString("GGG"));
}

void tst_QServiceManager_IPC::verifySharedProperties()
{
    QVERIFY(serviceShared);
    QFETCH(QByteArray, signature);
    QFETCH(QString, typeName);
    QFETCH(qint16, flags);
    QFETCH(QVariant, defaultValue);
    QFETCH(QVariant, writeValue);
    QFETCH(QVariant, expectedReturnValue);

    const QMetaObject* meta = serviceShared->metaObject();
    const int index = meta->indexOfProperty(signature.constData());
    QVERIFY(index>=0);
    QMetaProperty property = meta->property(index);
    QVERIFY(property.isValid());
    QCOMPARE(QString(property.typeName()), typeName);

    QVERIFY( property.isWritable() == (bool) (flags & 0x0001) );
    QVERIFY( property.isUser() == (bool) (flags & 0x0002) );
    QVERIFY( property.isStored() == (bool) (flags & 0x0004) );
    QVERIFY( property.isScriptable() == (bool) (flags & 0x0008) );
    QVERIFY( property.isResettable() == (bool) (flags & 0x0010) );
    QVERIFY( property.isReadable() == (bool) (flags & 0x0020) );
    QVERIFY( property.isFlagType() == (bool) (flags & 0x0040) );
    QVERIFY( property.isFinal() == (bool) (flags & 0x0080) );
    QVERIFY( property.isEnumType() == (bool) (flags & 0x0100) );
    QVERIFY( property.isDesignable() == (bool) (flags & 0x0200) );
    QVERIFY( property.isConstant() == (bool) (flags & 0x0400) );
    QVERIFY( property.hasNotifySignal() == (bool) (flags & 0x0800) );

    if (property.isReadable()) {
        QCOMPARE(defaultValue, serviceShared->property(signature));
        if (property.isWritable()) {
            serviceShared->setProperty(signature, writeValue);
            QCOMPARE(expectedReturnValue, serviceShared->property(signature));
            if (property.isResettable()) {
                property.reset(serviceShared);
                QCOMPARE(defaultValue, serviceShared->property(signature));
            }
            serviceShared->setProperty(signature, defaultValue);
            QCOMPARE(defaultValue, serviceShared->property(signature));
        }
    }
}

void tst_QServiceManager_IPC::verifyUniqueServiceObject()
{
    QVERIFY(serviceUnique != 0);
    const QMetaObject* mo = serviceUnique->metaObject();
    QCOMPARE(mo->className(), "UniqueTestService");
    QVERIFY(mo->superClass());
    QCOMPARE(mo->superClass()->className(), "QServiceProxyBase");
    // TODO adding the ipc failure signal seems to break these
    QCOMPARE(mo->methodCount()-mo-> methodOffset(), 27); // 28+1 added signal for error signal added by library
    QCOMPARE(mo->methodCount(), 33); //33 meta functions available + 1 signal
    //actual function presence will be tested later

//    for (int i = 0; i < mo->methodCount(); i++) {
//        qDebug() << "Methods" << i << mo->method(i).methodSignature();
//    }

    //test properties
    QCOMPARE(mo->propertyCount()-mo->propertyOffset(), 5);
    QCOMPARE(mo->propertyCount(), 6);

    QCOMPARE(mo->enumeratorCount()-mo->enumeratorOffset(), 3);
    QCOMPARE(mo->enumeratorCount(), 3);

    QCOMPARE(mo->classInfoCount()-mo->classInfoOffset(), 2);
    QCOMPARE(mo->classInfoCount(), 2);

    if (verbose) {
        qDebug() << "ServiceObject class: " << mo->className() << mo->superClass() << mo->superClass()->className();
        qDebug() << "------------------- Meta Methods -----------";
        qDebug() << "Methods:" << mo->methodCount()- mo->methodOffset() << "(" << mo->methodCount() << ")";
        for (int i=0; i< mo->methodCount(); i++) {
            QMetaMethod method = mo->method(i);
            QString type;
            switch (method.methodType()) {
            case QMetaMethod::Signal:
                type = "signal"; break;
            case QMetaMethod::Slot:
                type = "slot"; break;
            case QMetaMethod::Constructor:
                type = "constrcutor"; break;
            case QMetaMethod::Method:
                type = "method"; break;
            }
            qDebug() << "    " << i << "." << method.methodSignature() << type;
        }

        qDebug() << "------------------- Meta Properties --------";
        qDebug() << "Properties:" << mo->propertyCount()- mo->propertyOffset() << "(" << mo->propertyCount() << ")";
        for (int i=0; i< mo->propertyCount(); i++) {
            QMetaProperty property = mo->property(i);
            QString info = "Readable: %1 Resettable: %2 Writeable: %3 Designable: %4 Scriptable: %5 User: %6 Stored: %7 Constant: %8 Final: %9 HasNotify: %10 EnumType: %11 FlagType: %12";
            info = info.arg(property.isReadable()).arg(property.isResettable()).arg(property.isWritable());
            info = info.arg(property.isDesignable()).arg(property.isScriptable()).arg(property.isUser());
            info = info.arg(property.isStored()).arg(property.isConstant()).arg(property.isFinal());
            info = info.arg(property.hasNotifySignal()).arg(property.isEnumType()).arg(property.isFlagType());

            qDebug() << "    " << i << "." << property.name() << "Type:" << property.typeName() << info;
        }

        qDebug() << "------------------- Meta Enumerators --------";
        qDebug() << "Enums:" << mo->enumeratorCount()- mo->enumeratorOffset() << "(" << mo->enumeratorCount() << ")";
        for (int i=0; i< mo->enumeratorCount(); i++) {
            QMetaEnum e = mo->enumerator(i);
            qDebug() << "    " << i << "." << e.name() << "Scope:" << e.scope() << "KeyCount: " << e.keyCount();
            for (int j = 0; j<e.keyCount(); j++)
                qDebug() << "         " << e.key(j) << " - " << e.value(j);
        }

        qDebug() << "------------------- Meta class info ---------";
        qDebug() << "ClassInfos:" << mo->classInfoCount()- mo->classInfoOffset() << "(" << mo->classInfoCount() << ")";
        for (int i=0; i< mo->classInfoCount(); i++) {
            QMetaClassInfo info = mo->classInfo(i);
            qDebug() << "    " << i << "." << info.name() << "Value:" << info.value();
        }
    }
}

void tst_QServiceManager_IPC::verifyUniqueMethods_data()
{
    QTest::addColumn<QByteArray>("signature");
    QTest::addColumn<int>("metaMethodType");
    QTest::addColumn<QByteArray>("returnType");

    //list of all slots, signals and invokable functions
    QTest::newRow("signalWithIntParam(int)")
        << QByteArray("signalWithIntParam(int)") <<  (int)( QMetaMethod::Signal) << QByteArray("void");
    QTest::newRow("signalWithVariousParam(QVariant,QString,QServiceFilter,QVariant)")
        << QByteArray("signalWithVariousParam(QVariant,QString,QServiceFilter,QVariant)") <<  (int)( QMetaMethod::Signal) << QByteArray("void");
    QTest::newRow("valueChanged()")
        << QByteArray("valueChanged()") <<  (int)( QMetaMethod::Signal) << QByteArray("void");
    QTest::newRow("priorityChanged()")
        << QByteArray("priorityChanged()") <<  (int)( QMetaMethod::Signal) << QByteArray("void");
    QTest::newRow("triggerSignalWithIntParam()")
        << QByteArray("triggerSignalWithIntParam()") <<  (int)( QMetaMethod::Slot) << QByteArray("void");
    QTest::newRow("triggerSignalWithVariousParam()")
        << QByteArray("triggerSignalWithVariousParam()") <<  (int)( QMetaMethod::Slot) << QByteArray("void");
    QTest::newRow("triggerSignalWithIntParamExecute()")
        << QByteArray("triggerSignalWithIntParamExecute()") <<  (int)( QMetaMethod::Slot) << QByteArray("void");
    QTest::newRow("triggerSignalWithVariousParamExecute()")
        << QByteArray("triggerSignalWithVariousParamExecute()") <<  (int)( QMetaMethod::Slot) << QByteArray("void");
    QTest::newRow("testSlot()")
        << QByteArray("testSlot()") <<  (int)( QMetaMethod::Slot) << QByteArray("void");
    QTest::newRow("testSlotWithArgs(QByteArray,int,QVariant)")
        << QByteArray("testSlotWithArgs(QByteArray,int,QVariant)") <<  (int)( QMetaMethod::Slot) << QByteArray("void");
    QTest::newRow("testSlotWithCustomArg(QServiceFilter)")
        << QByteArray("testSlotWithCustomArg(QServiceFilter)") <<  (int)( QMetaMethod::Slot) << QByteArray("void");

    //QServiceInterfaceDescriptor has not been declared as meta type
    QTest::newRow("testSlotWithUnknownArg(QServiceInterfaceDescriptor)")
        << QByteArray("testSlotWithUnknownArg(QServiceInterfaceDescriptor)") <<  (int)( QMetaMethod::Slot) << QByteArray("void");
    QTest::newRow("testFunctionWithReturnValue(int)")
        << QByteArray("testFunctionWithReturnValue(int)") <<  (int)( QMetaMethod::Method) << QByteArray("QString");
    QTest::newRow("testFunctionWithVariantReturnValue(QVariant)")
        << QByteArray("testFunctionWithVariantReturnValue(QVariant)") <<  (int)( QMetaMethod::Method) << QByteArray("QVariant");
    QTest::newRow("testFunctionWithCustomReturnValue()")
        << QByteArray("testFunctionWithCustomReturnValue()") <<  (int)( QMetaMethod::Method) << QByteArray("QServiceFilter");
    QTest::newRow("slotConfirmation()")
        << QByteArray("slotConfirmation()") <<  (int)( QMetaMethod::Method) << QByteArray("uint");
    QTest::newRow("setConfirmationHash(uint)")
        << QByteArray("setConfirmationHash(uint)") <<  (int)( QMetaMethod::Method) << QByteArray("void");
}

void tst_QServiceManager_IPC::verifyUniqueMethods()
{
    QVERIFY(serviceUnique);
    QFETCH(QByteArray, signature);
    QFETCH(int, metaMethodType);
    QFETCH(QByteArray, returnType);

    const QMetaObject* meta = serviceUnique->metaObject();
    const int index = meta->indexOfMethod(signature.constData());
    QVERIFY(index>=0);
    QMetaMethod method = meta->method(index);
    QCOMPARE((int)method.methodType(), metaMethodType);
    QCOMPARE(QByteArray(method.typeName()), returnType);
}

void tst_QServiceManager_IPC::verifyUniqueProperties_data()
{
    QTest::addColumn<QByteArray>("signature");
    QTest::addColumn<QString>("typeName");
    QTest::addColumn<qint16>("flags");
    QTest::addColumn<QVariant>("defaultValue");
    QTest::addColumn<QVariant>("writeValue");
    QTest::addColumn<QVariant>("expectedReturnValue");

    //
    //    bit order:
    //        0 isWritable - 1 isUser - 2 isStored - 3 isScriptable
    //        4 isResettable - 5 isReadable - 6 isFlagType - 7 isFinal
    //        8 isEnumType - 9 isDesignable - 10 isConstant - 11 hasNotifySgnal
    //
    //        for more details see verifyProperties()
    //

    QTest::newRow("value property") << QByteArray("value")
            << QString("QString") << qint16(0x0A3D) << QVariant(QString("FFF")) << QVariant(QString("GGG")) << QVariant(QString("GGG"));
#ifndef SFW_USE_DBUS_BACKEND
    // ENUMS DONT WORK OVER DBUS
    QTest::newRow("priority property") << QByteArray("priority")
            << QString("Priority") << qint16(0x0B2D) << QVariant((int)0) << QVariant("Low") << QVariant((int)1) ;
    QTest::newRow("serviceFlags property") << QByteArray("serviceFlags")
            << QString("ServiceFlag") << qint16(0x036D) << QVariant((int)4) << QVariant("FirstBit|ThirdBit") << QVariant((int)5);
#endif
}

void tst_QServiceManager_IPC::verifyUniqueProperties()
{
    QVERIFY(serviceUnique);
    QFETCH(QByteArray, signature);
    QFETCH(QString, typeName);
    QFETCH(qint16, flags);
    QFETCH(QVariant, defaultValue);
    QFETCH(QVariant, writeValue);
    QFETCH(QVariant, expectedReturnValue);

    const QMetaObject* meta = serviceUnique->metaObject();
    const int index = meta->indexOfProperty(signature.constData());
    QVERIFY(index>=0);
    QMetaProperty property = meta->property(index);
    QVERIFY(property.isValid());
    QCOMPARE(QString(property.typeName()), typeName);

    QVERIFY( property.isWritable() == (bool) (flags & 0x0001) );
    QVERIFY( property.isUser() == (bool) (flags & 0x0002) );
    QVERIFY( property.isStored() == (bool) (flags & 0x0004) );
    QVERIFY( property.isScriptable() == (bool) (flags & 0x0008) );
    QVERIFY( property.isResettable() == (bool) (flags & 0x0010) );
    QVERIFY( property.isReadable() == (bool) (flags & 0x0020) );
    QVERIFY( property.isFlagType() == (bool) (flags & 0x0040) );
    QVERIFY( property.isFinal() == (bool) (flags & 0x0080) );
    QVERIFY( property.isEnumType() == (bool) (flags & 0x0100) );
    QVERIFY( property.isDesignable() == (bool) (flags & 0x0200) );
    QVERIFY( property.isConstant() == (bool) (flags & 0x0400) );
    QVERIFY( property.hasNotifySignal() == (bool) (flags & 0x0800) );

    if (property.isReadable()) {
        QCOMPARE(defaultValue, serviceUnique->property(signature));
        if (property.isWritable()) {
            serviceUnique->setProperty(signature, writeValue);
            QCOMPARE(expectedReturnValue, serviceUnique->property(signature));
            if (property.isResettable()) {
                property.reset(serviceUnique);
                QCOMPARE(defaultValue, serviceUnique->property(signature));
            }
            serviceUnique->setProperty(signature, defaultValue);
            QCOMPARE(defaultValue, serviceUnique->property(signature));
        }
    }
}

void tst_QServiceManager_IPC::verifyUniqueClassInfo_data()
{
    QTest::addColumn<QString>("classInfoKey");
    QTest::addColumn<QString>("classInfoValue");

    QTest::newRow("UniqueTestService") << QString("UniqueTestService") << QString("First test");
    QTest::newRow("Key") << QString("Key") << QString("Value");

}
void tst_QServiceManager_IPC::verifyUniqueClassInfo()
{
    QFETCH(QString, classInfoKey);
    QFETCH(QString, classInfoValue);

    const QMetaObject* meta = serviceUnique->metaObject();
    const int index = meta->indexOfClassInfo(classInfoKey.toLatin1().constData());

    QMetaClassInfo info = meta->classInfo(index);
    QCOMPARE(classInfoKey, QString(info.name()));
    QCOMPARE(classInfoValue, QString(info.value()));

}

Q_DECLARE_METATYPE(QList<int> )
void tst_QServiceManager_IPC::verifyUniqueEnumerator_data()
{
    QTest::addColumn<QString>("enumName");
    QTest::addColumn<QString>("enumScope");
    QTest::addColumn<int>("enumKeyCount");
    QTest::addColumn<bool>("enumIsFlag");
    QTest::addColumn<QStringList>("enumKeyNames");
    QTest::addColumn<QList<int> >("enumKeyValues");


    QStringList keynames;
    keynames << "FirstBit" << "SecondBit" << "ThirdBit";
    QList<int> values;
    values << 1 << 2 << 4;
    QTest::newRow("ServiceFlag") << QString("ServiceFlag") << QString("UniqueTestService")
        << 3 << true << keynames << values;
    QTest::newRow("ServiceFlags") << QString("ServiceFlags") << QString("UniqueTestService")
        << 3 << true << keynames << values;

    keynames.clear();
    values.clear();

    keynames << "High"  << "Low" << "VeryLow" << "ExtremelyLow";
    values << 0 << 1 << 2 << 3 ;

    QTest::newRow("Priority") << QString("Priority") << QString("UniqueTestService")
        << 4 << false << keynames << values;
}

void tst_QServiceManager_IPC::verifyUniqueEnumerator()
{
    QFETCH(QString, enumName);
    QFETCH(QString, enumScope);
    QFETCH(int, enumKeyCount);
    QFETCH(bool, enumIsFlag);
    QFETCH(QStringList, enumKeyNames);
    QFETCH(QList<int>, enumKeyValues);

    const QMetaObject* meta = serviceUnique->metaObject();
    const int index = meta->indexOfEnumerator(enumName.toLatin1().constData());
    QMetaEnum metaEnum = meta->enumerator(index);

    QVERIFY(metaEnum.isValid());
    QCOMPARE(enumScope, QString(metaEnum.scope()));
    QCOMPARE(enumKeyCount, metaEnum.keyCount());
    QCOMPARE(enumIsFlag, metaEnum.isFlag());
    QCOMPARE(enumKeyNames.count(), enumKeyValues.count());

    for (int i=0; i<enumKeyNames.count(); i++) {
        QCOMPARE(enumKeyNames.at(i), QString(metaEnum.valueToKey(enumKeyValues.at(i))));
        QCOMPARE(enumKeyValues.at(i), metaEnum.keyToValue(enumKeyNames.at(i).toLatin1().constData()));
    }
}

void tst_QServiceManager_IPC::testInvokableFunctions()
{
    // Test invokable method input with calculated return value
    QString temp;
    QString result;
    QString patternShared("%1 + 3 = %2");
    QString patternUnique("%1 x 3 = %2");
    for (int i = -10; i<10; i++) {
        result.clear(); temp.clear();
        QVERIFY(result.isEmpty());
        QVERIFY(temp.isEmpty());

        // Shared service
        QMetaObject::invokeMethod(serviceShared, "testFunctionWithReturnValue",
                                  Q_RETURN_ARG(QString, result),
                                  Q_ARG(int, i));
        temp = patternShared.arg(i).arg(i+3);
        QCOMPARE(temp, result);

        result.clear(); temp.clear();
        QVERIFY(result.isEmpty());
        QVERIFY(temp.isEmpty());

        // Unique service
        QMetaObject::invokeMethod(serviceUnique, "testFunctionWithReturnValue",
                                  Q_RETURN_ARG(QString, result),
                                  Q_ARG(int, i));
        temp = patternUnique.arg(i).arg(i*3);
        QCOMPARE(temp, result);
    }

    // Test invokable method with QVariant return type
    QList<QVariant> variants;
    variants << QVariant("CANT BE NULL") << QVariant(6) << QVariant(QString("testString"));
    for (int i = 0; i<variants.count(); i++) {
        QVariant varResult;

        // Shared service
        QMetaObject::invokeMethod(serviceShared, "testFunctionWithVariantReturnValue",
                                  Q_RETURN_ARG(QVariant, varResult),
                                  Q_ARG(QVariant, variants.at(i)));
        QCOMPARE(variants.at(i), varResult);

        // Unique service
        QMetaObject::invokeMethod(serviceUnique, "testFunctionWithVariantReturnValue",
                                  Q_RETURN_ARG(QVariant, varResult),
                                  Q_ARG(QVariant, variants.at(i)));
        QCOMPARE(variants.at(i), varResult);
    }

    // Test invokable method with custom return type
    QServiceFilter f;
    // Shared service
    QMetaObject::invokeMethod(serviceShared, "testFunctionWithCustomReturnValue",
                              Q_RETURN_ARG(QServiceFilter, f));
    QCOMPARE(f.serviceName(), QString("MySharedService"));
    QCOMPARE(f.interfaceName(), QString("com.nokia.qt.ipcunittest"));
    QCOMPARE(f.majorVersion(), 6);
    QCOMPARE(f.minorVersion(), 2);

    // Unique service
    QMetaObject::invokeMethod(serviceUnique, "testFunctionWithCustomReturnValue",
                              Q_RETURN_ARG(QServiceFilter, f));
    QCOMPARE(f.serviceName(), QString("MyUniqueService"));
    QCOMPARE(f.interfaceName(), QString("com.nokia.qt.ipcunittest"));
    QCOMPARE(f.majorVersion(), 6);
    QCOMPARE(f.minorVersion(), 7);

    // Test invokable method with list return type
    QList<QString> list, retList;
    list << "1" << "2" << "3";
    QMetaObject::invokeMethod(serviceUnique, "testFunctionWithListReturn",
                              Q_RETURN_ARG(QList<QString>, retList));
    QCOMPARE(list, retList);
}

void tst_QServiceManager_IPC::testSignalling()
{
    // Test signalling for simple methods
    QSignalSpy spy(serviceUnique, SIGNAL(signalWithIntParam(int)));
    QMetaObject::invokeMethod(serviceUnique, "triggerSignalWithIntParam");
    QTRY_VERIFY(spy.count() > 0);
    QCOMPARE(spy.at(0).at(0).toInt(), 5);

    // Test signalling for property changes
    QCOMPARE(QString("FFF"), serviceUnique->property("value").toString());
    QSignalSpy propSpy(serviceUnique, SIGNAL(valueChanged()));

    serviceUnique->setProperty("value", QString("GGG"));
    QTRY_VERIFY(propSpy.count() == 1);
    propSpy.clear();
    serviceUnique->setProperty("value", QString("FFF"));
    QTRY_VERIFY(propSpy.count() == 1);
    QCOMPARE(QString("FFF"), serviceUnique->property("value").toString());

    // Test signalling with custom types
    QSignalSpy variousSpy(serviceUnique, SIGNAL(signalWithVariousParam(QVariant,QString,QServiceFilter,QVariant)));
    QMetaObject::invokeMethod(serviceUnique, "triggerSignalWithVariousParam");

    QTRY_VERIFY(variousSpy.count() == 1);
    QCOMPARE(variousSpy.at(0).count(), 4);
    QCOMPARE(variousSpy.at(0).at(0).value<QVariant>(), QVariant("CAN'T BE NULL"));
    QCOMPARE(variousSpy.at(0).at(1).toString(), QString("string-value"));

    QVariant vFilter = variousSpy.at(0).at(2);
    QServiceFilter filter;
    QVERIFY(vFilter.canConvert<QServiceFilter>());
    filter = vFilter.value<QServiceFilter>();
    QCOMPARE(filter.serviceName(), QString("MyService"));
    QCOMPARE(filter.interfaceName(), QString("com.nokia.qt.ipcunittest"));
    QCOMPARE(filter.majorVersion(), 6);
    QCOMPARE(filter.minorVersion(), 7);

    QCOMPARE(variousSpy.at(0).at(3).value<QVariant>(), QVariant(5));
}

void tst_QServiceManager_IPC::testSlotInvokation()
{
    QObject *service = serviceUnique;

    // check the success of our invokable slot by using a hash of the method and its parameters
    uint hash = 1;
    uint expectedHash = 0;
    QMetaObject::invokeMethod(service, "setConfirmationHash",
                              Q_ARG(uint, 0));
    QMetaObject::invokeMethod(service, "slotConfirmation",
                              Q_RETURN_ARG(uint, hash));
    QCOMPARE(hash, (uint)0);

    // Test invokable slot with no arguments
    QMetaObject::invokeMethod(service, "testSlot");
    QMetaObject::invokeMethod(service, "slotConfirmation",
                               Q_RETURN_ARG(uint, hash));
    expectedHash = qHash(QString("testSlot()"));
    QCOMPARE(hash, expectedHash);

    // Test invokable slot with various arguments
    QVariant test("CANT BE NULL");
    QByteArray d = "array";
    int num = 5;
    QString output = QString("%1, %2, %3, %4");
    output = output.arg(d.constData()).arg(num).arg(test.toString()).arg(test.isValid());
    expectedHash = qHash(output);
    QMetaObject::invokeMethod(service, "testSlotWithArgs",
                              Q_ARG(QByteArray, d), Q_ARG(int, num), Q_ARG(QVariant, test));
    QMetaObject::invokeMethod(service, "slotConfirmation",
                              Q_RETURN_ARG(uint, hash));
    QCOMPARE(hash, expectedHash);

    // Test failed invokable slot since QServiceInterfaceDescriptor is not a registered meta type
    // by checking that the service didn't set the hash to -1 due to being called
    QServiceInterfaceDescriptor desc;
    QMetaObject::invokeMethod(service, "testSlotWithUnknownArg",
                              Q_ARG(QServiceInterfaceDescriptor, desc));
    QMetaObject::invokeMethod(service, "slotConfirmation",
                              Q_RETURN_ARG(uint, hash));
    QVERIFY(hash != 1);

    // Test slot function with custom argument
    QServiceFilter f("com.myInterface" , "4.5");
    f.setServiceName("MyService");
    output = QString("%1: %2 - %3.%4");
    output = output.arg(f.serviceName()).arg(f.interfaceName())
            .arg(f.majorVersion()).arg(f.minorVersion());
    expectedHash = qHash(output);
    QMetaObject::invokeMethod(service, "testSlotWithCustomArg",
                              Q_ARG(QServiceFilter, f));
    QMetaObject::invokeMethod(service, "slotConfirmation",
                              Q_RETURN_ARG(uint, hash));
    QCOMPARE(hash, expectedHash);

    // Test invokable method with a QList argument
    QList<QString> list;
    list << "one" << "two" << "three";
    output = "";
    for (int i=0; i<list.size(); i++) {
        output += list[i];
        if (i<list.size()-1)
            output += ", ";
    }
    expectedHash = qHash(output);
    QMetaObject::invokeMethod(serviceUnique, "testSlotWithListArg",
                              Q_ARG(QList<QString>, list));
    QMetaObject::invokeMethod(service, "slotConfirmation",
                              Q_RETURN_ARG(uint, hash));
    QCOMPARE(hash, expectedHash);

    // Test slot with QHash<QString, QVariant>
    QHash<QString, QVariant> hashv;
    hashv["test"] = QVariant::fromValue(1);
    hashv["test2"] = QVariant::fromValue(QString("test2 string"));
    hashv["test3"] = QVariant::fromValue(true);
    QHashIterator<QString, QVariant> i(hashv);
    output.clear();
    QStringList lines;
    while (i.hasNext()) {
        i.next();
        QString line = i.key();
        line += "=";
        line += i.value().toString();
        lines << line;
    }
    lines.sort();
    output = lines.join(QStringLiteral(","));
    expectedHash = qHash(output);

    QMetaObject::invokeMethod(serviceUnique, "testSlotWithComplexArg",
                              Q_ARG(QVariantHash, hashv));
    QMetaObject::invokeMethod(service, "slotConfirmation",
                              Q_RETURN_ARG(uint, hash));
    QCOMPARE(hash, expectedHash);


}

void tst_QServiceManager_IPC::verifyServiceClass()
{
    QRemoteServiceRegister *registerObject = new QRemoteServiceRegister();

    QVERIFY2(registerObject->quitOnLastInstanceClosed() == true, "should default to true, default is to shutdown");
    registerObject->setQuitOnLastInstanceClosed(false);
    QVERIFY2(registerObject->quitOnLastInstanceClosed() == false, "must transition to false");
    registerObject->setQuitOnLastInstanceClosed(true);
    QVERIFY2(registerObject->quitOnLastInstanceClosed() == true, "must transition back to true");

    delete registerObject;
}

void tst_QServiceManager_IPC::verifyLargeDataTransfer()
{
    QFETCH(QByteArray, data);

    QMetaObject::invokeMethod(serviceUnique, "testSlotWithData",
                              Q_ARG(QByteArray, data));
    QByteArray ret_data;
    QMetaObject::invokeMethod(serviceUnique, "testInvoableWithReturnData", Q_RETURN_ARG(QByteArray, ret_data));

    QVERIFY2(data.length() == ret_data.length(), "Returned data from the service is not the same length as data sent");
    QVERIFY2(data == ret_data, "Returned data from service does not match");
}

void tst_QServiceManager_IPC::verifyLargeDataTransfer_data()
{
    QTest::addColumn<QByteArray>("data");

    QTest::newRow("Trivial") << QByteArray("c");
    QTest::newRow("1k") << QByteArray(1024, 'A');
    QTest::newRow("2k") << QByteArray(2048, 'A');
    QTest::newRow("4k") << QByteArray(4096, 'A');
    QTest::newRow("8k") << QByteArray(8192, 'A');
    QTest::newRow("16k") << QByteArray(16384, 'A');
    QTest::newRow("32k") << QByteArray(32768, 'A');
    QTest::newRow("64k") << QByteArray(65536, 'A');
    QTest::newRow("128k") << QByteArray(131072, 'A');
    QTest::newRow("1 megabyte") << QByteArray(1048576, 'A');
    QTest::newRow("Free mem") << QByteArray("");
}

class FetchLotsOfProperties : public QThread
{
    Q_OBJECT
public:
    FetchLotsOfProperties(int runs, bool shared, QObject *parent = 0);
    void run();

public slots:
    void startFetches();
    void printProgress();

private slots:
    void setup();
    void fetchProperty();
    void ipcError();

signals:
    void ready();
    void error();

private:
    int runs;
    int start_runs;
    bool shared;
    QString lastValue;
    QObject *service;
    QServiceManager *mgr;
};

FetchLotsOfProperties::FetchLotsOfProperties(int runs, bool shared, QObject *parent)
    : QThread(parent),
      service(0),
      mgr(0)
{
    this->runs = runs;
    this->shared = shared;
    this->start_runs = runs;
}

void FetchLotsOfProperties::run()
{
    QMetaObject::invokeMethod(this, "setup", Qt::QueuedConnection);
    exec();
}

void FetchLotsOfProperties::setup()
{
    mgr = new QServiceManager(this);

    if (!mgr) {
        qWarning() << "Failed to create a manager";
        emit error();
        exit(-1);
    }

    QList<QServiceInterfaceDescriptor> list = mgr->findInterfaces("IPCExampleService");
    if (list.isEmpty()) {
        qWarning() << "Thread failed to query the database, or the database contained no services";
        emit error();
        exit(-1);
        return;
    }
    foreach (const QServiceInterfaceDescriptor &d, list) {
        if (d.majorVersion() == 3 && d.minorVersion() == 5 && !shared) {
            service = mgr->loadInterface(d);
        }
        if (d.majorVersion() == 3 && d.minorVersion() == 4 && shared) {
            service = mgr->loadInterface(d);
        }
        if (service)
            connect(service, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)), this, SLOT(ipcError()));
    }

    if (!service) {
        qWarning() << "Thread failed to load service";
        emit error();
        exit(-1);
        return;
    }

    connect(this, SIGNAL(destroyed()), service, SLOT(deleteLater()));

    lastValue = service->property("value").toString();

    emit ready();
}

void FetchLotsOfProperties::startFetches()
{
    if (!mgr || !service) {
        exit(-1);
    }

    if (runs > 0) {
        QMetaObject::invokeMethod(this, "fetchProperty", Qt::QueuedConnection);
    }
    else {
        quit();
    }
}

void FetchLotsOfProperties::printProgress()
{
    qWarning() << "Thread has" << runs << "left, started with" << start_runs;
}

void FetchLotsOfProperties::fetchProperty()
{
    if (!service) {
        emit error();
        exit(-1);
    }

    QString value = service->property("value").toString();
    if (value != lastValue) {
        qWarning() << "Value changed on read! Got" << value << "wanted" << lastValue;
        emit error();
        exit(-1);
    }

    if (runs-- > 0) {
        QMetaObject::invokeMethod(this, "fetchProperty", Qt::QueuedConnection);
    }
    else {
        quit();
    }
}

void FetchLotsOfProperties::ipcError()
{

}

void tst_QServiceManager_IPC::verifyThreadSafety()
{
    QFETCH(int, shared);
    QFETCH(int, runs);
    QFETCH(int, threads);

    QSignalSpy ready(this, SIGNAL(threadReady()));
    QSignalSpy finished(this, SIGNAL(threadFinished()));
    QSignalSpy errors(this, SIGNAL(threadError()));
    bool mixed = true;
    for (int i = 0; i < threads; i++) {
        switch (shared) {
        case 0:
            mixed = false;
            break;
        case 1:
            mixed = true;
            break;
        case 2:
            mixed = !mixed;
        }
        FetchLotsOfProperties *fetch = new FetchLotsOfProperties(runs, mixed, this);
        connect(this, SIGNAL(threadRun()), fetch, SLOT(startFetches()));
        connect(fetch, SIGNAL(ready()), this, SIGNAL(threadReady()));
        connect(fetch, SIGNAL(error()), this, SIGNAL(threadError()));
        connect(fetch, SIGNAL(finished()), this, SIGNAL(threadFinished()));
        connect(fetch, SIGNAL(finished()), fetch, SLOT(deleteLater()));
        connect(this, SIGNAL(threadRemaining()), fetch, SLOT(printProgress()));
        fetch->start();
    }


#define THREAD_TIMEOUT 30*1000

    QTimer timer;
    timer.setInterval(THREAD_TIMEOUT);
    timer.setSingleShot(true);
    timer.start();

    QTime start = QTime::currentTime();

    while (((ready.count()) < threads) && (timer.isActive())) {
        QCOMPARE(errors.count(), 0);
        QCoreApplication::processEvents();
        QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    }
    QTRY_COMPARE(ready.count(), threads);
    QVERIFY2(!errors.count(), "Threads reported errors on setup");
    emit threadRun();

    qDebug() << "Waiting on" << threads << "threads to finish";

    while (((finished.count() + errors.count()) < threads) && (timer.isActive())) {
        QCOMPARE(errors.count(), 0);
        QCoreApplication::processEvents();
        QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    }

    qDebug() << "Waited" << start.secsTo(QTime::currentTime()) << "seconds";

    if (finished.count() != threads){
        emit threadRemaining();
    }

    QCOMPARE(finished.count(), threads);
    QVERIFY2(!errors.count(), "Threads reported errors");
}

void tst_QServiceManager_IPC::verifyThreadSafety_data()
{
    QTest::addColumn<int>("shared");
    QTest::addColumn<int>("runs");
    QTest::addColumn<int>("threads");

    QTest::newRow("unique, 2 threads") << 0 << 50 << 2;
    QTest::newRow("unique, 4 threads") << 0 << 25 << 4;
    QTest::newRow("unique, 16 threads") << 0 << 5 << 16;
    QTest::newRow("shared, 2 threads") << 1 << 50 << 2;
    QTest::newRow("shared, 4 threads") << 1 << 25 << 4;
    QTest::newRow("shared, 16 threads") << 1 << 5 << 16;
    QTest::newRow("mixed, 16 threads") << 2 << 5 << 16;
#ifndef Q_OS_WIN // Limit on event notifiers.
    QTest::newRow("unique, 128 threads") << 0 << 5 << 128;
    QTest::newRow("shared, 128 threads") << 1 << 5 << 128;
    QTest::newRow("mixed, 128 threads") << 2 << 5 << 128;
#endif // Q_OS_WIN
}

void tst_QServiceManager_IPC::unblockRemote()
{
    QString ret = serviceUnique->property("releaseBlockingRead").toString();
    QCOMPARE(ret, QStringLiteral("releaseBlockingReadReturned"));
}

void tst_QServiceManager_IPC::verifyRemoteBlockingFunctions()
{
#ifdef SFW_USE_DBUS_BACKEND
    QEXPECT_FAIL("", "Test case known to segfault on dbus, skipping, QTBUG-23168", Abort);
    QVERIFY(false);
#else
    connect(serviceUnique, SIGNAL(blockingValueRead()), this, SLOT(unblockRemote()));

    QString ret = serviceUnique->property("blockingValue").toString();
    QCOMPARE(ret, QStringLiteral("blockingValueReturned"));
#endif
}

void tst_QServiceManager_IPC::testSignalSlotOrdering()
{
    QSignalSpy spy(serviceUnique, SIGNAL(count(int)));

    int value = -1;
    bool ret = QMetaObject::invokeMethod(serviceUnique, "testSignalSlotOrdering", Q_RETURN_ARG(int, value));
    QVERIFY2(ret == true, "Verify slot call worked");
    QVERIFY2(value == 0, "Verify slot return value");
    QVERIFY2(spy.count() == 0, "Verify no signals fired before the invokeMethod returns");
    QTRY_COMPARE(spy.count(), 20);
}

void tst_QServiceManager_IPC::testServiceSecurity()
{

    QServiceManager* manager = new QServiceManager(this);
    QList<QServiceInterfaceDescriptor> list = manager->findInterfaces("IPCExampleService");
    QServiceInterfaceDescriptor d, dPriv, dGlobal;
    foreach (QServiceInterfaceDescriptor d, list){
        if (d.majorVersion() == 3 && d.minorVersion() == 9) {
            dPriv = d;
        }
        if (d.majorVersion() == 3 && d.minorVersion() == 10) {
            dGlobal = d;
        }
    }

    QVERIFY(dPriv.isValid());
    QVERIFY(dGlobal.isValid());

    // Rejects all connections
    QObject *o = manager->loadInterface(dPriv);
    QVERIFY2(o == 0, "Everything should fail security/creation with this interface");

    o = manager->loadInterface(dGlobal);
    QVERIFY2(o, "Global class should allow the first connection");

    bool status;
    // make sync call to ensure remote is in the same state
    QMetaObject::invokeMethod(o, "disableConnections", Q_RETURN_ARG(bool, status), Q_ARG(bool, true));
    QCOMPARE(status, true);

    // expect a null object back
    QObject *second = manager->loadInterface(dGlobal);
    QVERIFY2(second == 0, "Security checks should no longer allow objects to be created");

    // make sync call to ensure remote is in the same state
    QMetaObject::invokeMethod(o, "disableConnections", Q_RETURN_ARG(bool, status), Q_ARG(bool, false));

    second = manager->loadInterface(dGlobal);
    QVERIFY2(second != 0, "Object creation should be allowed");

    delete second;

}

void tst_QServiceManager_IPC::testProcessLaunch()
{
}

void tst_QServiceManager_IPC::verifyAsyncLoading()
{

    QFETCH(QString, serviceInterface);
    QFETCH(int, descriptor);
    QFETCH(bool, errors);
    QFETCH(int, simultaneous);

    QServiceManager mgr;

    QServiceReply *reply = 0;
    QServiceReply *replies[16]; // must be constant for windows, set to max size below

    if (simultaneous > 0) {
        for (int i = 0; i < simultaneous; i++) {
            replies[i] = mgr.loadInterfaceRequest(serviceInterface);
        }
        reply = replies[0];
    } else if (descriptor > 0) {
        QList<QServiceInterfaceDescriptor> list = mgr.findInterfaces("IPCExampleService");
        QServiceInterfaceDescriptor d;
        foreach (d, list) {
            if (d.majorVersion() == 3 && d.minorVersion() == descriptor) {
                break;
            }
        }
        reply = mgr.loadInterfaceRequest(d);
    } else if (!serviceInterface.isEmpty()) {
        reply = mgr.loadInterfaceRequest(serviceInterface);
    } else {
        QFAIL("No descriptor nor interface speficied");
    }


    QSignalSpy startedSpy(reply, SIGNAL(started()));
    QSignalSpy errorSpy(reply, SIGNAL(errorChanged()));
    QSignalSpy finishedSpy(reply, SIGNAL(finished()));

    QCOMPARE(startedSpy.count(), 0);

    QTRY_COMPARE(startedSpy.count(), 1);
    if (!errors)
        QCOMPARE(errorSpy.count(), 0);

    QTRY_COMPARE(finishedSpy.count(), 1);
    QCOMPARE(reply->isFinished(), true);
    if (!errors) {
        QCOMPARE(errorSpy.count(), 0);
        QCOMPARE(reply->error(), QServiceManager::NoError);
    } else {
        QCOMPARE(errorSpy.count(), 1);
        QVERIFY(reply->error() != QServiceManager::NoError);
    }



    if (simultaneous) {
        for (int i = 0; i < simultaneous; i++) {
            QTRY_COMPARE(replies[i]->isFinished(), true);
            QCOMPARE(replies[i]->error(), QServiceManager::NoError);
        }
    }

    delete reply;
}

void tst_QServiceManager_IPC::verifyAsyncLoading_data()
{
    QTest::addColumn<QString>("serviceInterface");
    QTest::addColumn<int>("descriptor");
    QTest::addColumn<bool>("errors");
    QTest::addColumn<int>("simultaneous");


    QTest::newRow("Load minor version 5") << QString() << 5 << false << 0;
    QTest::newRow("Load default interface") << QString("com.nokia.qt.ipcunittest") << 0 << false << 0;
    QTest::newRow("Load invalid interface") << QString("com.nokia.qt.ipcunittest.does.not.exist") << 0 << true << 0;
    QTest::newRow("Load 4 interfaces") << QString("com.nokia.qt.ipcunittest") << 1 << false << 4;
    QTest::newRow("Load 16 interfaces") << QString("com.nokia.qt.ipcunittest") << 1 << false << 16;

}

void tst_QServiceManager_IPC::testIpcFailure()
{
    // test deleting an object doesn't trigger an IPC fault
    ipcfailure = false;
    delete serviceShared;
    QVERIFY2(!ipcfailure, "Deleting an object should not cause an IPC failure message");
    serviceShared = 0;

    ipcfailure = false;
    QMetaObject::invokeMethod(serviceUnique, "testIpcFailure");
    int i = 0;
    while (!ipcfailure && i++ < 50)
        QTest::qWait(50);

    QVERIFY(ipcfailure);

    // TODO restart the connection
    //initTestCase();
}

void tst_QServiceManager_IPC::verifyFailures()
{
    bool result;

    QServiceManager* manager = new QServiceManager(this);
    QList<QServiceInterfaceDescriptor> list = manager->findInterfaces("IPCExampleService");
    QServiceInterfaceDescriptor d;
    foreach (d, list){
        if (d.majorVersion() == 3 && d.minorVersion() == 6) {
            QObject *o = manager->loadInterface(d);
            QVERIFY2(o == 0, "Failure to allocate remote object returns null");
        }
        if (d.majorVersion() == 3 && d.minorVersion() == 7) {
            QObject *o = manager->loadInterface(d);
            QVERIFY2(o == 0, "Failure to allocate remote object returns null");
        }
    }

    QMetaObject::invokeMethod(miscTest, "addTwice",
                              Q_RETURN_ARG(bool, result));
    QVERIFY2(result, "Added the same service twice, returned different entries");

    QMetaObject::invokeMethod(miscTest, "getInvalidEntry",
                              Q_RETURN_ARG(bool, result));
    QVERIFY2(result, "Invalid entry returns invalid meta data");

    delete manager;

}



QTEST_MAIN(tst_QServiceManager_IPC);
#include "tst_qservicemanager_ipc.moc"
