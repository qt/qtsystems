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

#include <QtTest/QtTest>
#include "qvaluespace.h"
#include "qvaluespacepublisher.h"
#include "qvaluespacesubscriber.h"

QT_USE_NAMESPACE

class tst_QValueSpace : public QObject
{
    Q_OBJECT

private slots:
    void tst_availableLayers();

    void tst_PublisherPath_data();
    void tst_PublisherPath();

    void tst_PublishSubscribe_data();
    void tst_PublishSubscribe();
};

void tst_QValueSpace::tst_availableLayers()
{
    QList<QUuid> layers = QValueSpace::availableLayers();

    if (layers.size() == 0)
        QSKIP("No value space layer available, thus skip all the test cases.");

#if defined(Q_OS_LINUX)
#if !defined(QT_NO_GCONFLAYER)
    QVERIFY(layers.contains(QVALUESPACE_GCONF_LAYER));
#endif
#elif defined(Q_OS_WIN)
    QVERIFY(layers.contains(QVALUESPACE_VOLATILEREGISTRY_LAYER));
    QVERIFY(layers.contains(QVALUESPACE_NONVOLATILEREGISTRY_LAYER));
#endif
}

void tst_QValueSpace::tst_PublisherPath_data()
{
    QTest::addColumn<QString>("path");

    QTest::newRow("root") << QString(QStringLiteral("/"));
    QTest::newRow("non existing path") << QString(QStringLiteral("/a/path/that/doesnt/exist"));
}

void tst_QValueSpace::tst_PublisherPath()
{
    if (QValueSpace::availableLayers().size() == 0)
        QSKIP("No value space layer available, thus skip all the test cases.");

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

    QTest::newRow("root") << QString(QStringLiteral("/")) << QString(QStringLiteral("myName")) << QVariant::fromValue(QString(QStringLiteral("myValue")));
    QTest::newRow("non existing path") << QString(QStringLiteral("/a/path/that/doesnt/exist")) << QString(QStringLiteral("propertyName")) << QVariant::fromValue(QString(QStringLiteral("propertyValue")));
}

void tst_QValueSpace::tst_PublishSubscribe()
{
    if (QValueSpace::availableLayers().size() == 0)
        QSKIP("No value space layer available, thus skip all the test cases.");

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

    subscriber.setPath(path + QStringLiteral("/") + name);
    QCOMPARE(subscriber.value(), value);

    publisher.resetValue(name);
    publisher.sync();
}

QTEST_MAIN(tst_QValueSpace)
#include "tst_qvaluespace.moc"
