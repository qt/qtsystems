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
#ifndef QT_NO_DEBUG_STREAM
#include <QDebug>
#endif
#include <qserviceinterfacedescriptor.h>
#include <private/qserviceinterfacedescriptor_p.h>

QT_USE_NAMESPACE
class tst_QServiceInterfaceDescriptor: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void comparison();
#ifndef QT_NO_DATASTREAM
    void testStreamOperators();
#endif
#ifndef QT_NO_DEBUG_STREAM
    void testDebugStream();
#endif
    void destructor();
};

void tst_QServiceInterfaceDescriptor::initTestCase()
{
}

void tst_QServiceInterfaceDescriptor::comparison()
{
    QServiceInterfaceDescriptor desc;
    QVERIFY(desc.majorVersion() == -1);
    QVERIFY(desc.minorVersion() == -1);
    QVERIFY(desc.serviceName().isEmpty());
    QVERIFY(desc.interfaceName().isEmpty());
    QVERIFY(!desc.attribute(QServiceInterfaceDescriptor::Capabilities).isValid());
    QVERIFY(!desc.attribute(QServiceInterfaceDescriptor::Location).isValid());
    QVERIFY(!desc.attribute(QServiceInterfaceDescriptor::InterfaceDescription).isValid());
    QVERIFY(!desc.attribute(QServiceInterfaceDescriptor::ServiceDescription).isValid());
    QVERIFY(desc.scope() == QService::UserScope);
    QVERIFY(!desc.isValid());

    QServiceInterfaceDescriptor copy(desc);
    QVERIFY(copy.majorVersion() == -1);
    QVERIFY(copy.minorVersion() == -1);
    QVERIFY(copy.serviceName().isEmpty());
    QVERIFY(copy.interfaceName().isEmpty());
    QVERIFY(!copy.attribute(QServiceInterfaceDescriptor::Capabilities).isValid());
    QVERIFY(!copy.attribute(QServiceInterfaceDescriptor::Location).isValid());
    QVERIFY(!copy.attribute(QServiceInterfaceDescriptor::InterfaceDescription).isValid());
    QVERIFY(!copy.attribute(QServiceInterfaceDescriptor::ServiceDescription).isValid());
    QVERIFY(copy.scope() == QService::UserScope);
    QVERIFY(!copy.isValid());

    QVERIFY(desc == copy);

    QServiceInterfaceDescriptor valid;
    QServiceInterfaceDescriptorPrivate *d = new QServiceInterfaceDescriptorPrivate();
    QServiceInterfaceDescriptorPrivate::setPrivate(&valid, d);
    d->serviceName = "name";
    d->interfaceName = "interface";
    d->major = 3;
    d->minor = 1;
    d->attributes.insert(QServiceInterfaceDescriptor::ServiceDescription, QString("mydescription"));
    d->customAttributes.insert(QString("ckey"), QString("cvalue"));
    d->scope = QService::SystemScope;

    QCOMPARE(valid.interfaceName(), QString("interface"));
    QCOMPARE(valid.serviceName(), QString("name"));
    QCOMPARE(valid.majorVersion(), 3);
    QCOMPARE(valid.minorVersion(), 1);
    QCOMPARE(valid.customAttribute("ckey"), QString("cvalue"));
    QCOMPARE(valid.attribute(QServiceInterfaceDescriptor::ServiceDescription).toString(), QString("mydescription"));
    QCOMPARE(valid.attribute(QServiceInterfaceDescriptor::Location).toString(), QString(""));
    QCOMPARE(valid.scope(), QService::SystemScope);
    QVERIFY(valid.isValid());

    QVERIFY(valid != desc);
    QVERIFY(desc != valid);

    //test copy constructor
    QServiceInterfaceDescriptor validCopy(valid);
    QVERIFY(valid==validCopy);
    QVERIFY(validCopy==valid);

    QServiceInterfaceDescriptorPrivate::getPrivate(&validCopy)->attributes.insert(QServiceInterfaceDescriptor::Location, QString("myValue"));
    QVERIFY(valid!=validCopy);
    QVERIFY(validCopy!=valid);

    QCOMPARE(validCopy.interfaceName(), QString("interface"));
    QCOMPARE(validCopy.serviceName(), QString("name"));
    QCOMPARE(validCopy.majorVersion(), 3);
    QCOMPARE(validCopy.minorVersion(), 1);
    QCOMPARE(validCopy.attribute(QServiceInterfaceDescriptor::Location).toString(), QString("myValue"));
    QCOMPARE(validCopy.attribute(QServiceInterfaceDescriptor::ServiceDescription).toString(), QString("mydescription"));
    QCOMPARE(validCopy.customAttribute("ckey"),QString("cvalue"));
    QCOMPARE(validCopy.scope(), QService::SystemScope);
    QVERIFY(validCopy.isValid());

    //test assignment operator
    QServiceInterfaceDescriptor validCopy2 = valid;
    QVERIFY(valid==validCopy2);
    QVERIFY(validCopy2==valid);

    QServiceInterfaceDescriptorPrivate::getPrivate(&validCopy2)->attributes.insert(QServiceInterfaceDescriptor::Location, QString("myValue2"));
    QVERIFY(valid!=validCopy2);
    QVERIFY(validCopy2!=valid);

    QCOMPARE(validCopy2.interfaceName(), QString("interface"));
    QCOMPARE(validCopy2.serviceName(), QString("name"));
    QCOMPARE(validCopy2.majorVersion(), 3);
    QCOMPARE(validCopy2.minorVersion(), 1);
    QCOMPARE(validCopy2.attribute(QServiceInterfaceDescriptor::Location).toString(), QString("myValue2"));
    QCOMPARE(validCopy2.customAttribute("ckey"),QString("cvalue"));
    QCOMPARE(validCopy2.attribute(QServiceInterfaceDescriptor::ServiceDescription).toString(), QString("mydescription"));
    QCOMPARE(validCopy2.scope(), QService::SystemScope);
    QVERIFY(validCopy2.isValid());

    //test customAttributes
    d->customAttributes.insert(QString("ckey"), QString("cvalue"));
    d->customAttributes.insert(QString("ckey1"), QString("cvalue1"));
    d->customAttributes.insert(QString("ckey2"), QString("cvalue2"));
    QStringList customAttributes = valid.customAttributes();
    QVERIFY(customAttributes.contains("ckey"));
    QVERIFY(customAttributes.contains("ckey1"));
    QVERIFY(customAttributes.contains("ckey2"));
    QCOMPARE(customAttributes.count(), 3);
}

#ifndef QT_NO_DATASTREAM
void tst_QServiceInterfaceDescriptor::testStreamOperators()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::ReadWrite);
    QDataStream stream(&buffer);


    QServiceInterfaceDescriptor empty;
    QVERIFY(!empty.isValid());

    //stream invalid into invalid
    QServiceInterfaceDescriptor invalid;
    QVERIFY(!invalid.isValid());
    QVERIFY(invalid == empty);
    buffer.seek(0);
    stream << empty;
    buffer.seek(0);
    stream >> invalid;
    QVERIFY(invalid == empty);

    //stream invalid into valid
    QServiceInterfaceDescriptor valid;
    QServiceInterfaceDescriptorPrivate *d = new QServiceInterfaceDescriptorPrivate();
    QServiceInterfaceDescriptorPrivate::setPrivate(&valid, d);
    d->serviceName = "name";
    d->interfaceName = "interface";
    d->major = 3;
    d->minor = 1;
    d->attributes.insert(QServiceInterfaceDescriptor::Location, QString("myValue"));
    d->attributes.insert(QServiceInterfaceDescriptor::Capabilities, QStringList() << "val1" << "val2");
    d->attributes.insert(QServiceInterfaceDescriptor::ServiceDescription, QString("This is the service description"));
    d->attributes.insert(QServiceInterfaceDescriptor::InterfaceDescription, QString("This is the interface description"));
    d->customAttributes.insert(QString("key1"), QString("value1"));
    d->customAttributes.insert(QString("abcd"), QString("efgh"));
    d->customAttributes.insert(QString("empty"), QString(""));
    d->scope = QService::SystemScope;
    QVERIFY(valid.isValid());
    QServiceInterfaceDescriptor validref = valid;
    QVERIFY(validref == valid);
    QVERIFY(!(validref!=valid));
    QVERIFY(empty!=validref);

    buffer.seek(0);
    stream << empty;
    buffer.seek(0);
    stream >> validref;
    QVERIFY(empty == validref);
    QVERIFY(!(empty!=validref));
    QVERIFY(validref != valid);

    //stream valid into invalid
    QServiceInterfaceDescriptor invalid2;
    QVERIFY(!invalid2.isValid());
    validref = valid;
    QVERIFY(validref == valid);
    QVERIFY(validref != invalid2);

    buffer.seek(0);
    stream << validref;
    buffer.seek(0);
    stream >> invalid2;
    QVERIFY(invalid2 == validref);
    QVERIFY(!(invalid2 != validref));
    QVERIFY(invalid2.isValid());
    QVERIFY(invalid2.interfaceName() == QString("interface"));
    QVERIFY(invalid2.serviceName() == QString("name"));
    QVERIFY(invalid2.majorVersion() == 3);
    QVERIFY(invalid2.minorVersion() == 1);
    QVERIFY(invalid2.attribute(QServiceInterfaceDescriptor::Location).toString() == QString("myValue"));
    QVERIFY(invalid2.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList() == (QStringList() << "val1" << "val2"));
    QVERIFY(invalid2.attribute(QServiceInterfaceDescriptor::ServiceDescription).toString() == QString("This is the service description"));
    QVERIFY(invalid2.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString() == QString("This is the interface description"));
    QCOMPARE(invalid2.customAttribute("key1"), QString("value1"));
    QCOMPARE(invalid2.customAttribute("abcd"), QString("efgh"));
    QCOMPARE(invalid2.customAttribute("notvalid"), QString());
    QVERIFY(invalid2.customAttribute("notvalid").isEmpty());
    QVERIFY(invalid2.customAttribute("notvalid").isNull());
    QCOMPARE(invalid2.customAttribute("empty"), QString(""));
    QVERIFY(invalid2.customAttribute("empty").isEmpty());
    QVERIFY(!invalid2.customAttribute("empty").isNull());
    QCOMPARE(invalid2.scope(), QService::SystemScope);

    //stream valid into valid
    QServiceInterfaceDescriptor valid2;
    QServiceInterfaceDescriptorPrivate *d2 = new QServiceInterfaceDescriptorPrivate();
    QServiceInterfaceDescriptorPrivate::setPrivate(&valid2, d2);
    d2->serviceName = "name2";
    d2->interfaceName = "interface2";
    d2->major = 5;
    d2->minor = 6;
    d2->attributes.insert(QServiceInterfaceDescriptor::Location, QString("myValue1"));
    d2->attributes.insert(QServiceInterfaceDescriptor::Capabilities, QStringList() << "val3" << "val4");
    d2->attributes.insert(QServiceInterfaceDescriptor::ServiceDescription, QString("This is the second service description"));
    d2->attributes.insert(QServiceInterfaceDescriptor::InterfaceDescription, QString("This is the second interface description"));
    d2->customAttributes.insert(QString("key1"), QString("value2"));
    d2->customAttributes.insert(QString("abcd1"), QString("efgh"));
    d2->customAttributes.insert(QString("empty"), QString(""));
    d2->scope = QService::UserScope;
    QVERIFY(valid2.isValid());
    QCOMPARE(valid2.customAttribute("key1"), QString("value2"));
    QCOMPARE(valid2.customAttribute("abcd1"), QString("efgh"));
    QCOMPARE(valid2.customAttribute("abcd"), QString());
    QVERIFY(valid2.customAttribute("abcd").isEmpty());
    QVERIFY(valid2.customAttribute("abcd").isNull());
    QCOMPARE(valid2.customAttribute("empty"), QString(""));
    QVERIFY(valid2.customAttribute("empty").isEmpty());
    QVERIFY(!valid2.customAttribute("empty").isNull());


    QVERIFY(valid2 != valid);
    QVERIFY(!(valid2 == valid));

    buffer.seek(0);
    stream << valid;
    buffer.seek(0);
    stream >> valid2;
    QVERIFY(valid2 == valid);
    QVERIFY(!(valid2 != valid));
    QVERIFY(valid2.isValid());
    QVERIFY(valid2.interfaceName() == QString("interface"));
    QVERIFY(valid2.serviceName() == QString("name"));
    QVERIFY(valid2.majorVersion() == 3);
    QVERIFY(valid2.minorVersion() == 1);
    QVERIFY(valid2.attribute(QServiceInterfaceDescriptor::Location).toString() == QString("myValue"));
    QVERIFY(valid2.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList() == (QStringList() << "val1" << "val2"));
    QVERIFY(valid2.attribute(QServiceInterfaceDescriptor::ServiceDescription).toString() == QString("This is the service description"));
    QVERIFY(valid2.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString() == QString("This is the interface description"));
    QCOMPARE(valid2.customAttribute("key1"), QString("value1"));
    QCOMPARE(valid2.customAttribute("abcd"), QString("efgh"));
    QCOMPARE(valid2.customAttribute("notvalid"), QString());
    QVERIFY(valid2.customAttribute("notvalid").isEmpty());
    QVERIFY(valid2.customAttribute("notvalid").isNull());
    QCOMPARE(valid2.customAttribute("empty"), QString(""));
    QVERIFY(valid2.customAttribute("empty").isEmpty());
    QVERIFY(!valid2.customAttribute("empty").isNull());
    QCOMPARE(valid2.customAttribute("abcd1"), QString());
    QCOMPARE(valid2.scope(), QService::SystemScope);
}
#endif //QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
static QString msg;
static QtMsgType type;

static void customMsgHandler(QtMsgType t, const QMessageLogContext &, const QString &m)
{
    msg = m;
    type = t;

}

void tst_QServiceInterfaceDescriptor::testDebugStream()
{
    QServiceInterfaceDescriptor valid2;
    QServiceInterfaceDescriptorPrivate *d2 = new QServiceInterfaceDescriptorPrivate();
    QServiceInterfaceDescriptorPrivate::setPrivate(&valid2, d2);
    d2->serviceName = "name2";
    d2->interfaceName = "interface2";
    d2->major = 5;
    d2->minor = 6;
    d2->attributes.insert(QServiceInterfaceDescriptor::Location, QString("myValue1"));
    d2->attributes.insert(QServiceInterfaceDescriptor::Capabilities, QStringList() << "val3" << "val4");
    d2->attributes.insert(QServiceInterfaceDescriptor::ServiceDescription, QString("This is the second service description"));
    d2->attributes.insert(QServiceInterfaceDescriptor::InterfaceDescription, QString("This is the second interface description"));
    QVERIFY(valid2.isValid());

    QServiceInterfaceDescriptor invalid;

    qInstallMessageHandler(customMsgHandler);
    qDebug() << valid2 << invalid;
    QCOMPARE(type, QtDebugMsg);
    QCOMPARE(msg,QString::fromLatin1("QServiceInterfaceDescriptor(service=\"name2\", interface=\"interface2 5.6\") QServiceInterfaceDescriptor(invalid)"));
    qInstallMessageHandler(0);
}
#endif

void tst_QServiceInterfaceDescriptor::destructor()
{
    //test destructor if descriptor is invalid
    QServiceInterfaceDescriptor* invalid = new QServiceInterfaceDescriptor();
    delete invalid;

    //test destructor if descriptor is valid
    QServiceInterfaceDescriptor* valid = new QServiceInterfaceDescriptor();
    QServiceInterfaceDescriptorPrivate *d = new QServiceInterfaceDescriptorPrivate();
    QServiceInterfaceDescriptorPrivate::setPrivate(valid, d);
    d->serviceName = "name";
    d->interfaceName = "interface";
    d->major = 3;
    d->minor = 1;
    d->attributes.insert(QServiceInterfaceDescriptor::Location, QString("myValue"));
    d->attributes.insert(QServiceInterfaceDescriptor::Capabilities, QStringList() << "val1" << "val2");
    d->attributes.insert(QServiceInterfaceDescriptor::ServiceDescription, QString("This is the service description"));
    d->attributes.insert(QServiceInterfaceDescriptor::InterfaceDescription, QString("This is the interface description"));
    d->customAttributes.insert("ckey", "cvalue");
    QVERIFY(valid->isValid());
    delete valid;
}

void tst_QServiceInterfaceDescriptor::cleanupTestCase()
{
}

QTEST_MAIN(tst_QServiceInterfaceDescriptor)
#include "tst_qserviceinterfacedescriptor.moc"
