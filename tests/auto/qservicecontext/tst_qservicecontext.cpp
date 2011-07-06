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

#include <QtTest/QtTest>
#include <QtCore>
#include <qservicecontext.h>

QT_USE_NAMESPACE
class MyServiceContext : public QServiceContext
{
    Q_OBJECT
public:
    void notify(ContextType type, const QVariant &variant)
    {
        contextType = type;
        contextVariant = variant;
    }

    ContextType contextType;
    QVariant contextVariant;
};


class tst_QServiceContext: public QObject
{
    Q_OBJECT

private:
    void addStringData();

private slots:
    void clientId();
    void setClientId();
    void setClientId_data();

    void clientName();
    void setClientName();
    void setClientName_data();

    void notify();
    void clientData();
};

void tst_QServiceContext::addStringData()
{
    QTest::addColumn<QString>("string");

    QTest::newRow("empty string") << QString();
    QTest::newRow("basic") << QString("abc");
    QTest::newRow("spaces") << QString("abc def");
    QTest::newRow("numbers") << QString("12312313 12312332");
}

void tst_QServiceContext::clientId()
{
    MyServiceContext c;
    QCOMPARE(c.clientId(), QString());
}

void tst_QServiceContext::setClientId()
{
    QFETCH(QString, string);

    MyServiceContext c;
    c.setClientId(string);
    QCOMPARE(c.clientId(), string);
}

void tst_QServiceContext::setClientId_data()
{
    addStringData();
}

void tst_QServiceContext::clientName()
{
    MyServiceContext c;
    QCOMPARE(c.clientName(), QString());
}

void tst_QServiceContext::setClientName()
{
    QFETCH(QString, string);

    MyServiceContext c;
    c.setClientName(string);
    QCOMPARE(c.clientName(), string);
}

void tst_QServiceContext::setClientName_data()
{
    addStringData();
}

void tst_QServiceContext::notify()
{
    MyServiceContext c;

    c.notify(QServiceContext::DisplayContext, "abc");
    QCOMPARE(c.contextType, QServiceContext::DisplayContext);
    QCOMPARE(c.contextVariant, qVariantFromValue<QString>("abc"));

    c.notify(QServiceContext::UserDefined, "123");
    QCOMPARE(c.contextType, QServiceContext::UserDefined);
    QCOMPARE(c.contextVariant, qVariantFromValue<QString>("123"));
}

void tst_QServiceContext::clientData()
{
#ifdef QT_NO_USERDATA
    QWARN("QServiceContext::clientData() depends on QObjectUserData which is disabled in the current Qt build");
#endif
    MyServiceContext c1;
    MyServiceContext c2;
    QStringList keys;
    keys << QString("data1")<<QString("data2")<<QString("data3")<<QString("data4")<<QString("data5");
    QList<QVariant> values;
    values << QVariant(QString("value1")) << QVariant(QString("value2"))
        << QVariant(QString("value3")) << QVariant(QString("value4")) << QVariant(QString("value5"));

    const int dataCount = 5;

    for (int i = 0; i < dataCount; i++) {
        c1.setClientData(keys[i], values[i]);
        c2.setClientData(keys[i], values[(i+1)%dataCount]);
    }

    // check that stored values are properly saved and can be retrieved
    for (int i = 0; i < dataCount; i++) {
#ifndef QT_NO_USERDATA
        QCOMPARE(c1.clientData(keys[i]), values[i]);
        QCOMPARE(c1.clientData(keys[i]), c2.clientData(keys[(i+dataCount-1)%dataCount]));
#else
        QCOMPARE(c1.clientData(keys[i]), QVariant());
        QCOMPARE(c1.clientData(keys[i]), QVariant());
#endif
    }

    // reset values and recheck
    c2.resetClientData();
    for (int i = 0; i < dataCount; i++) {
#ifndef QT_NO_USERDATA
        QCOMPARE(c1.clientData(keys[i]), values[i]);
#else
        QCOMPARE(c1.clientData(keys[i]), QVariant());
#endif
        QCOMPARE(c2.clientData(keys[i]), QVariant());
    }

    //edit first three existing values
    for (int i = 0; i < dataCount-2; i++) {
        c1.setClientData(keys[i],QVariant(QString("custom")));
    }

    for (int i = 0; i < dataCount; i++) {
#ifndef QT_NO_USERDATA
        if (i<dataCount-2)
            QCOMPARE(c1.clientData(keys[i]), QVariant(QString("custom")));
        else
            QCOMPARE(c1.clientData(keys[i]), values[i]);
#else
        QCOMPARE(c1.clientData(keys[i]), QVariant());
#endif
    }
}

QTEST_MAIN(tst_QServiceContext)

#include "tst_qservicecontext.moc"
