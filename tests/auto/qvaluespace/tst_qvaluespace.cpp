/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include "qvaluespace.h"
#include "qvaluespacepublisher.h"
#include "qvaluespacesubscriber.h"

QT_USE_NAMESPACE

class tst_QValueSpace : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void tst_availableLayers();

    void tst_PublisherPath_data();
    void tst_PublisherPath();

    void tst_PublishSubscribe_data();
    void tst_PublishSubscribe();
};

void tst_QValueSpace::initTestCase()
{
    QValueSpace::initValueSpaceServer();
}

void tst_QValueSpace::tst_availableLayers()
{
    QList<QUuid> layers = QValueSpace::availableLayers();

    if (QValueSpace::availableLayers() == 0)
        QSKIP("No value space layer available, thus skip all the test cases.", SkipAll);

#if defined(Q_OS_LINUX)
#if !defined(QT_NO_GCONFLAYER)
    QVERIFY(layers.contains(QVALUESPACE_GCONF_LAYER));
#endif
#if !defined(QT_NO_CONTEXTKIT)
    QVERIFY(layers.contains(QVALUESPACE_CONTEXTKITNONCORE_LAYER));
    QVERIFY(layers.contains(QVALUESPACE_CONTEXTKITCORE_LAYER));
#endif
#elif defined(Q_OS_WIN)
    QVERIFY(layers.contains(QVALUESPACE_VOLATILEREGISTRY_LAYER));
    QVERIFY(layers.contains(QVALUESPACE_NONVOLATILEREGISTRY_LAYER));
#else
    QVERIFY(layers.contains(QVALUESPACE_SHAREDMEMORY_LAYER));
#endif
}

void tst_QValueSpace::tst_PublisherPath_data()
{
    QTest::addColumn<QString>("path");

    QTest::newRow("root") << QString::fromAscii("/");
    QTest::newRow("non existing path") << QString::fromAscii("/a/path/that/doesnt/exist");
}

void tst_QValueSpace::tst_PublisherPath()
{
    QFETCH(QString, path);

    QValueSpacePublisher publisher(path);
    QVERIFY(publisher.isConnected());
    QCOMPARE(publisher.path(), path);
}

void tst_QValueSpace::tst_PublishSubscribe_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QVariant>("value");

    QTest::newRow("root") << QString::fromAscii("/") << QString::fromAscii("myName") << QVariant::fromValue(QString::fromAscii("myValue"));
    QTest::newRow("non existing path") << QString::fromAscii("/a/path/that/doesnt/exist") << QString::fromAscii("propertyName") << QVariant::fromValue(QString::fromAscii("propertyValue"));
}

void tst_QValueSpace::tst_PublishSubscribe()
{
    QFETCH(QString, path);
    QFETCH(QString, name);
    QFETCH(QVariant, value);

    QValueSpacePublisher publisher(path);
    QVERIFY(publisher.isConnected());
    publisher.setValue(name, value);
    publisher.sync();

    QValueSpaceSubscriber subscriber(path);
    QVERIFY(subscriber.isConnected());
    QCOMPARE(subscriber.value(name), value);

    subscriber.cd(name);
    QCOMPARE(subscriber.value(), value);

    publisher.resetValue(name);
    publisher.sync();
}

QTEST_MAIN(tst_QValueSpace)
#include "tst_qvaluespace.moc"
