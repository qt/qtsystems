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

#include "../qsfwtestutil.h"

#include <QtTest/QtTest>
#include <QtCore>
#include <qservicemanager.h>
#include <qabstractsecuritysession.h>

QT_USE_NAMESPACE

#define PRINT_ERR(a) qPrintable(QString("error = %1").arg(a.error()))

class TestSession : public QAbstractSecuritySession
{
public:
    TestSession() {}
    virtual ~TestSession(){}

    virtual bool isAllowed(const QStringList& serviceCaps) 
    {
        //client must have at least service caps;
        QSet<QString> sub = serviceCaps.toSet() - clientCaps;;
        if (sub.isEmpty())
            return true;
        else
            return false;
    }
    
    void setClientCaps(const QStringList& capabilities)
    {
        clientCaps = capabilities.toSet();
    }
    
private:

    QSet<QString> clientCaps;

};

class tst_QAbstractSecuritySession: public QObject
{
    Q_OBJECT
    
private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void testSecSessionHandling();
    
private:
   QString path; 
};

void tst_QAbstractSecuritySession::initTestCase()
{
    path = QCoreApplication::applicationDirPath() + "/xml/";

    QSfwTestUtil::setupTempUserDb();
    QSfwTestUtil::setupTempSystemDb();

    QSfwTestUtil::removeTempUserDb();
    QSfwTestUtil::removeTempSystemDb();
#if defined(Q_OS_SYMBIAN)
    QSfwTestUtil::removeDatabases_symbian();
#endif
}

void tst_QAbstractSecuritySession::cleanup()
{
    //use QEventLopp::DeferredDeletion
    //QServiceManager::loadInterface makes use of deleteLater() when
    //cleaning up service objects and their respective QPluginLoader
    //we want to force the testcase to run the cleanup code
    QCoreApplication::processEvents(QEventLoop::AllEvents|QEventLoop::DeferredDeletion);
}

void tst_QAbstractSecuritySession::testSecSessionHandling()
{
    QFile file(QDir::toNativeSeparators(path+"testserviceplugin.xml"));
    QServiceManager mgr;
    QVERIFY(mgr.findServices().isEmpty());
    QVERIFY2(mgr.addService(&file), PRINT_ERR(mgr));
    QVERIFY(mgr.findServices() == (QStringList()<< "TestService"));

    QServiceFilter simpleFilter;
    simpleFilter.setInterface("com.nokia.qt.ISimpleTypeTest");
    QList<QServiceInterfaceDescriptor> list = mgr.findInterfaces(simpleFilter);
    QVERIFY(list.count() == 1);
    QServiceInterfaceDescriptor simpleDesc = list.at(0);
    QVERIFY(simpleDesc.isValid());
    QVERIFY(simpleDesc.majorVersion() == 1);
    QVERIFY(simpleDesc.minorVersion() == 0);
    QVERIFY(simpleDesc.interfaceName() == QString("com.nokia.qt.ISimpleTypeTest"));
    QCOMPARE(simpleDesc.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList(),
             QStringList() << "simple");

    QServiceFilter complexFilter;
    complexFilter.setInterface("com.nokia.qt.IComplexTypeTest");
    list = mgr.findInterfaces(complexFilter);
    QVERIFY(list.count() == 1);
    QServiceInterfaceDescriptor complexDesc = list.at(0);
    QVERIFY(complexDesc.isValid());
    QVERIFY(complexDesc.majorVersion() == 2);
    QVERIFY(complexDesc.minorVersion() == 3);
    QVERIFY(complexDesc.interfaceName() == QString("com.nokia.qt.IComplexTypeTest"));
    QCOMPARE(complexDesc.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList(),
             QStringList() << "complex" << "simple");

    //no QAbstractSecuritySession object
    QObject* o = mgr.loadInterface(simpleDesc, 0, 0 );
    QVERIFY(o);
    delete o;
    o = mgr.loadInterface(complexDesc, 0, 0);
    QVERIFY(o);
    delete o;

    //client does not have any permission
    TestSession* secSession = new TestSession();
    secSession->setClientCaps(QStringList());

    o = mgr.loadInterface(simpleDesc, 0, secSession );
    QVERIFY(!o);
    o = mgr.loadInterface(complexDesc, 0, secSession);
    QVERIFY(!o);

    //client has simple permission
    secSession->setClientCaps(QStringList() << "simple");
    o = mgr.loadInterface(simpleDesc, 0, secSession );
    QVERIFY(o);
    delete o;
    o = mgr.loadInterface(complexDesc, 0, secSession);
    QVERIFY(!o);
    
    //client has simple and complex permission
    secSession->setClientCaps(QStringList() << "simple" << "complex");
    o = mgr.loadInterface(simpleDesc, 0, secSession );
    QVERIFY(o);
    delete o;
    o = mgr.loadInterface(complexDesc, 0, secSession);
    QVERIFY(o);
    delete o;
    
    //client has simple, complex and advanced permission
    secSession->setClientCaps(QStringList() << "simple" << "complex" << "advanced");
    o = mgr.loadInterface(simpleDesc, 0, secSession );
    QVERIFY(o);
    delete o;
    o = mgr.loadInterface(complexDesc, 0, secSession);
    QVERIFY(o);
    delete o;

    //client has unknown capability
    secSession->setClientCaps(QStringList() << "unknown");
    o = mgr.loadInterface(simpleDesc, 0, secSession );
    QVERIFY(!o);
    o = mgr.loadInterface(complexDesc, 0, secSession);
    QVERIFY(!o);

    delete secSession;
}

void tst_QAbstractSecuritySession::cleanupTestCase()
{
    QSfwTestUtil::removeTempUserDb();
    QSfwTestUtil::removeTempSystemDb();
#if defined(Q_OS_SYMBIAN)
    QSfwTestUtil::removeDatabases_symbian();
#endif
}

QTEST_MAIN(tst_QAbstractSecuritySession)
#include "tst_qabstractsecuritysession.moc"
