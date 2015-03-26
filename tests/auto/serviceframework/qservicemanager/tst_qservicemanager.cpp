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
#include <private/qserviceinterfacedescriptor_p.h>
#include "sampleservice/sampleserviceplugin.h"
#include "../qsfwtestutil.h"

#include <QtTest/QtTest>
#include <QtCore>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QPair>

#define PRINT_ERR(a) qPrintable(QString("error = %1").arg(a.error()))

typedef QList<QServiceInterfaceDescriptor> ServiceInterfaceDescriptorList;
Q_DECLARE_METATYPE(QServiceFilter)
Q_DECLARE_METATYPE(QServiceInterfaceDescriptor)
Q_DECLARE_METATYPE(ServiceInterfaceDescriptorList)

Q_DECLARE_METATYPE(QSet<QString>)
Q_DECLARE_METATYPE(QList<QByteArray>)
Q_DECLARE_METATYPE(QService::Scope)

QT_BEGIN_NAMESPACE
typedef QHash<QServiceInterfaceDescriptor::Attribute, QVariant> DescriptorAttributes;
QT_END_NAMESPACE

QT_USE_NAMESPACE
static DescriptorAttributes defaultDescriptorAttributes()
{
    DescriptorAttributes props;
    //props[QServiceInterfaceDescriptor::Capabilities] = QStringList();
    props[QServiceInterfaceDescriptor::Location] = "";
    props[QServiceInterfaceDescriptor::ServiceDescription] = "";
    props[QServiceInterfaceDescriptor::InterfaceDescription] = "";
    props[QServiceInterfaceDescriptor::ServiceType] = QService::Plugin;
    return props;
}
static const DescriptorAttributes DEFAULT_DESCRIPTOR_PROPERTIES = defaultDescriptorAttributes();

// Helper function for debugging. Useful e.g. for checking what is difference between
// two descriptors (in addition to attributes printed below, the
// QServiceInterfaceDescriptor::== operator also compares
// attributes.
/*static void printDescriptor (const QServiceInterfaceDescriptor &desc) {
    qDebug("***QServiceInterfaceDescriptor printed:");
    qDebug() << "***majorVersion:" << desc.majorVersion();
    qDebug() << "***minorVersion:" << desc.minorVersion();
    qDebug() << "***interfaceName:" << desc.interfaceName();
    qDebug() << "***serviceName:" << desc.serviceName();
    qDebug() << "***customAttributes:" << desc.customAttributes();
    qDebug() << "***attributes:" << desc.attribute(QServiceInterfaceDescriptor::Capabilities) <<
                desc.attribute(QServiceInterfaceDescriptor::Location) <<
                desc.attribute(QServiceInterfaceDescriptor::ServiceDescription) <<
                desc.attribute(QServiceInterfaceDescriptor::InterfaceDescription) <<
                desc.attribute(QServiceInterfaceDescriptor::ServiceType);
    qDebug() << "***isValid(): " << desc.isValid();
    qDebug() << "***scope (user:0, system:1): " << desc.scope();
}*/

class ServicesListener : public QObject
{
    Q_OBJECT
public slots:
    void serviceAdded(const QString &name , QService::Scope scope) {
        params.append(qMakePair(name, scope));
    }
    void serviceRemoved(const QString &name, QService::Scope scope) {
        params.append(qMakePair(name, scope));
    }
public:
    QList<QPair<QString, QService::Scope> > params;
};


class tst_QServiceManager: public QObject
{
    Q_OBJECT

private:
    inline QString xmlTestDataPath(const QString &xmlFileName) const
        { return m_xmlDirectory + QLatin1Char('/') + xmlFileName; }

    QByteArray createServiceXml(const QString &serviceName, const QByteArray &interfaceXml, const QString &path, const QString &description = QString()) const
    {
        QString xml = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
        xml += "<service>\n";
        xml += "<name>" + serviceName + "</name>\n";
        xml += "<filepath>" + path + "</filepath>\n";
        xml += "<description>" + description + "</description>\n";
        xml += interfaceXml;
        xml += "</service>\n";
        return xml.toLatin1();
    }

    QByteArray createServiceXml(const QString &serviceName, const QList<QServiceInterfaceDescriptor> &descriptors) const
    {
        Q_ASSERT(descriptors.count() > 0);
        return createServiceXml(serviceName, createInterfaceXml(descriptors),
                descriptors[0].attribute(QServiceInterfaceDescriptor::Location).toString(),
                descriptors[0].attribute(QServiceInterfaceDescriptor::ServiceDescription).toString());
    }

    QByteArray createInterfaceXml(const QList<QServiceInterfaceDescriptor> &descriptors) const
    {
        QByteArray interfacesXml;
        foreach (const QServiceInterfaceDescriptor &desc, descriptors) {
            QString version = QString("%1.%2").arg(desc.majorVersion()).arg(desc.minorVersion());
            interfacesXml += createInterfaceXml(desc.interfaceName(), version,
                    desc.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString());
        }
        return interfacesXml;
    }

    QByteArray createInterfaceXml(const QString &name, const QString &version = "1.0", const QString &description = QString()) const
    {
       QString xml = "<interface>\n";
        xml += "<name>" + name + "</name>\n";
        xml += "<version>" + version + "</version>\n";
        xml += " <description>" + description + "</description>\n";
        xml += "</interface>\n";
        return xml.toLatin1();

    }

    QServiceInterfaceDescriptor createDescriptor(const QString &interfaceName, int major, int minor, const QString &serviceName, const DescriptorAttributes &attributes = DescriptorAttributes(), QService::Scope scope = QService::UserScope) const
    {
        QString version = QString("%1.%2").arg(major).arg(minor);

        QServiceInterfaceDescriptorPrivate *priv = new QServiceInterfaceDescriptorPrivate;
        priv->serviceName = serviceName;
        priv->interfaceName = interfaceName;
        priv->major = major;
        priv->minor = minor;
        priv->scope = scope;

        priv->attributes = attributes;
        foreach (QServiceInterfaceDescriptor::Attribute key, DEFAULT_DESCRIPTOR_PROPERTIES.keys()) {
            if (!priv->attributes.contains(key))
                priv->attributes[key] = DEFAULT_DESCRIPTOR_PROPERTIES[key];
        }

        QServiceInterfaceDescriptor desc;
        QServiceInterfaceDescriptorPrivate::setPrivate(&desc, priv);
        return desc;
    }

    void deleteTestDatabasesAndWaitUntilDone()
    {
        QSfwTestUtil::removeTempUserDb();
        QSfwTestUtil::removeTempSystemDb();

        QTRY_VERIFY(!QFile::exists(QSfwTestUtil::tempUserDbDir()));
        QTRY_VERIFY(!QFile::exists(QSfwTestUtil::tempSystemDbDir()));
    }

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

    void constructor();
    void constructor_scope();
    void constructor_scope_data();

    void findServices();
    void findServices_data();

    void findServices_scope();
    void findServices_scope_data();

    void findInterfaces_filter();
    void findInterfaces_filter_data();

    void findInterfaces_scope();
    void findInterfaces_scope_data();

    void loadInterface_string();

    void loadInterface_descriptor();
    void loadInterface_descriptor_data();

    void loadInterface_testLoadedObjectAttributes();

    void loadLocalTypedInterface();

    void addService();
    void addService_data();

    void addService_testInvalidServiceXml();
    void addService_testPluginLoading();
    void addService_testPluginLoading_data();
    void addService_testInstallService();

    void removeService();

    void setInterfaceDefault_strings();
    void setInterfaceDefault_strings_multipleInterfaces();

    void setInterfaceDefault_descriptor();
    void setInterfaceDefault_descriptor_data();

    void interfaceDefault();

    void serviceAdded();
    void serviceAdded_data();

    void serviceRemoved();
    void serviceRemoved_data();

private:
    QString m_xmlDirectory;
    QString m_pluginsDirectory;
    QStringList m_validPluginNames;
    QStringList m_validPluginPaths;
};

static const char *plugins[] =
    {"tst_sfw_sampleserviceplugin", "tst_sfw_sampleserviceplugin2", "tst_sfw_testservice2plugin"};

static const char *expectedPluginClassNames[] =
    {"SampleServicePluginClass", 0, "TestService"};

static inline QByteArray msgCannotLoadLibrary(const QString &name, const QString &error)
{
    QByteArray result = "Unable to load '";
    result += name.toLocal8Bit();
    result += "': ";
    result += error.toLocal8Bit();
    return result;
}

void tst_QServiceManager::initTestCase()
{
    qRegisterMetaType<QService::Scope>("QService::Scope");

    QSfwTestUtil::setupTempUserDb();
    QSfwTestUtil::setupTempSystemDb();

    m_xmlDirectory = QFINDTESTDATA("xml");
    QVERIFY2(!m_xmlDirectory.isEmpty(), "Unable to locate XML test data");
    m_pluginsDirectory = QFINDTESTDATA("plugins");
    QVERIFY2(!m_pluginsDirectory.isEmpty(), "Unable to locate plugins");

    // Example XML files specify plugins as 'plugins/foo', so, add parent directory.
    const QFileInfo pluginsDirectory(m_pluginsDirectory);
    QCoreApplication::addLibraryPath(pluginsDirectory.absolutePath());
    QCoreApplication::addLibraryPath(pluginsDirectory.absoluteFilePath());

    // Determine plugin and let QLibrary resolve the full path of the libraries
    // '/path/foo' -> '/path/libfoo.so' or '/path/foo.dll'
    const size_t pluginCount = sizeof(plugins) / sizeof(const char *);
    for (size_t i = 0; i < pluginCount; ++i) {
        const QString pluginName = QLatin1String(plugins[i]);
        const QString pluginPathIn = m_pluginsDirectory + QLatin1Char('/') + pluginName;
        QLibrary lib(pluginPathIn);
        QVERIFY2(lib.load(), msgCannotLoadLibrary(pluginPathIn, lib.errorString()).constData());
        lib.unload();
        m_validPluginNames << pluginName;
        m_validPluginPaths << lib.fileName();
    }
}

void tst_QServiceManager::init()
{
    QSfwTestUtil::removeTempUserDb();
    QSfwTestUtil::removeTempSystemDb();
    QSettings settings("com.nokia.qt.serviceframework.tests", "SampleServicePlugin");
    settings.setValue("installed", false);
}

void tst_QServiceManager::cleanupTestCase()
{
    QSfwTestUtil::removeTempUserDb();
    QSfwTestUtil::removeTempSystemDb();
    //process deferred delete events
    //QServiceManager::loadInterface makes use of deleteLater() when
    //cleaning up service objects and their respective QPluginLoader
    //we want to force the testcase to run the cleanup code
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
}

void tst_QServiceManager::constructor()
{
    QObject o;
    QServiceManager mgr(&o);
    QCOMPARE(mgr.scope(), QService::UserScope);
    QCOMPARE(mgr.parent(), &o);
}

void tst_QServiceManager::constructor_scope()
{
    QFETCH(QService::Scope, scope);

    QObject o;
    QServiceManager mgr(scope, &o);
    QCOMPARE(mgr.scope(), scope);
    QCOMPARE(mgr.parent(), &o);
}

void tst_QServiceManager::constructor_scope_data()
{
    QTest::addColumn<QService::Scope>("scope");

    QTest::newRow("user") << QService::UserScope;
    QTest::newRow("system") << QService::SystemScope;
}

void tst_QServiceManager::findServices()
{
    QFETCH(QList<QByteArray>, xmlBlocks);
    QFETCH(QStringList, interfaceNames);
    QFETCH(QSet<QString>, searchByInterfaceResult);
    QFETCH(QSet<QString>, searchAllResult);

    QServiceManager mgr;
    QServiceFilter wildcardFilter;

    // Check that nothing is found neither with default search or interface-search
    QVERIFY(mgr.findServices().isEmpty());
    foreach (const QString &serviceInterface, interfaceNames)
        QVERIFY(mgr.findServices(serviceInterface).isEmpty());
    QCOMPARE(mgr.findInterfaces(wildcardFilter).count(), 0);

    // Add all services from the xmlBlocks list
    foreach (const QByteArray &xml, xmlBlocks) {
        QBuffer buffer;
        buffer.setData(xml);
        QVERIFY2(mgr.addService(&buffer), PRINT_ERR(mgr));
    }
    // Check that all services are found with default search
    QCOMPARE(mgr.findServices().toSet(), searchAllResult);
    // Check that all services are found based on interface search
    foreach (const QString &serviceInterface, interfaceNames)
        QCOMPARE(mgr.findServices(serviceInterface).toSet(), searchByInterfaceResult);

    // Check that nothing is found with empty interface
    QCOMPARE(mgr.findServices("com.invalid.interface") , QStringList());
}

void tst_QServiceManager::findServices_data()
{
    QTest::addColumn< QList<QByteArray> >("xmlBlocks");
    QTest::addColumn<QStringList>("interfaceNames");
    QTest::addColumn< QSet<QString> >("searchByInterfaceResult");
    QTest::addColumn< QSet<QString> >("searchAllResult");

    QStringList interfaces;
    interfaces << "com.nokia.qt.TestInterfaceA";
    interfaces << "com.nokia.qt.TestInterfaceB";
    QByteArray interfacesXml;
    for (int i=0; i<interfaces.count(); i++)
        interfacesXml += "\n" + createInterfaceXml(interfaces[i]);

    QTest::newRow("one service")
            << (QList<QByteArray>() << createServiceXml("SomeTestService", interfacesXml, m_validPluginNames.first()))
            << interfaces
            << (QSet<QString>() << "SomeTestService")
            << (QSet<QString>() << "SomeTestService");

    QTest::newRow("multiple services with same interfaces")
            << (QList<QByteArray>() << createServiceXml("SomeTestService", interfacesXml, m_validPluginNames[0])
                                    << createServiceXml("SomeSimilarTestService", interfacesXml, m_validPluginNames[1]))
            << interfaces
            << (QSet<QString>() << "SomeTestService" << "SomeSimilarTestService")
            << (QSet<QString>() << "SomeTestService" << "SomeSimilarTestService");

    QStringList interfaces2;
    interfaces2 << "com.nokia.qt.TestInterfaceY";
    interfaces2 << "com.nokia.qt.TestInterfaceZ";
    QByteArray interfacesXml2;
    for (int i=0; i<interfaces2.count(); i++)
        interfacesXml2 += "\n" + createInterfaceXml(interfaces2[i]);
    QTest::newRow("multiple services with different interfaces")
            << (QList<QByteArray>() << createServiceXml("SomeTestService", interfacesXml, m_validPluginNames[0])
                                    << createServiceXml("TestServiceWithOtherInterfaces", interfacesXml2, m_validPluginNames[1]))
            << interfaces2
            << (QSet<QString>() << "TestServiceWithOtherInterfaces")
            << (QSet<QString>() << "SomeTestService" << "TestServiceWithOtherInterfaces");
}

void tst_QServiceManager::findServices_scope()
{
    QFETCH(QService::Scope, scope_add);
    QFETCH(QService::Scope, scope_find);
    QFETCH(bool, expectFound);

    QByteArray xml = createServiceXml("SomeTestService",
            createInterfaceXml("com.nokia.qt.TestInterface"), m_validPluginNames[0]);
    QBuffer buffer(&xml);

    QServiceManager mgrUser(QService::UserScope);
    QServiceManager mgrSystem(QService::SystemScope);

    QServiceManager &mgrAdd = scope_add == QService::UserScope ? mgrUser : mgrSystem;
    QServiceManager &mgrFind = scope_find == QService::UserScope ? mgrUser : mgrSystem;

    QVERIFY2(mgrAdd.addService(&buffer), PRINT_ERR(mgrAdd));
    QStringList result = mgrFind.findServices();
    QCOMPARE(!result.isEmpty(), expectFound);
}

void tst_QServiceManager::findServices_scope_data()
{
    QTest::addColumn<QService::Scope>("scope_add");
    QTest::addColumn<QService::Scope>("scope_find");
    QTest::addColumn<bool>("expectFound");

    QTest::newRow("user scope")
            << QService::UserScope << QService::UserScope << true;
    QTest::newRow("system scope")
            << QService::SystemScope << QService::SystemScope << true;

    QTest::newRow("user scope - add, system scope - find")
            << QService::UserScope << QService::SystemScope << false;
    QTest::newRow("system scope - add, user scope - find")
            << QService::SystemScope << QService::UserScope << true;
}

void tst_QServiceManager::findInterfaces_filter()
{
    QFETCH(QByteArray, xml);
    QFETCH(QServiceFilter, filter);
    QFETCH(QList<QServiceInterfaceDescriptor>, expectedInterfaces);

    QServiceManager mgr;

    QBuffer buffer(&xml);
    QVERIFY2(mgr.addService(&buffer), PRINT_ERR(mgr));

    QList<QServiceInterfaceDescriptor> result = mgr.findInterfaces(filter);

    qDebug() << "Wanted" << expectedInterfaces;

    qDebug() << "Got" << result;

    QCOMPARE(result.toSet(), expectedInterfaces.toSet());
}

void tst_QServiceManager::findInterfaces_filter_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QServiceFilter>("filter");
    QTest::addColumn<ServiceInterfaceDescriptorList>("expectedInterfaces");

    QString serviceName = "SomeTestService";
    DescriptorAttributes attributes;
    attributes[QServiceInterfaceDescriptor::Location] = m_validPluginNames.first();
    //attributes[QServiceInterfaceDescriptor::ServiceType] = QService::Plugin;

    QList<QServiceInterfaceDescriptor> descriptors;
    descriptors << createDescriptor("com.nokia.qt.TestInterfaceA", 1, 0, serviceName, attributes);
    descriptors << createDescriptor("com.nokia.qt.TestInterfaceB", 1, 0, serviceName, attributes);
    descriptors << createDescriptor("com.nokia.qt.TestInterfaceB", 2, 0, serviceName, attributes);
    descriptors << createDescriptor("com.nokia.qt.TestInterfaceB", 2, 3, serviceName, attributes);

    QByteArray serviceXml = createServiceXml(serviceName, descriptors);
    QServiceFilter filter;

    QTest::newRow("empty/wildcard filter")
            << serviceXml
            << QServiceFilter()
            << descriptors;

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceA");
    QTest::newRow("by interface name (A)")
            << serviceXml
            << filter
            << descriptors.mid(0, 1);

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB");
    QTest::newRow("by interface name (B)")
            << serviceXml
            << filter
            << descriptors.mid(1);

    filter = QServiceFilter();
    filter.setServiceName(serviceName);
    QTest::newRow("by service name, should find all")
            << serviceXml
            << filter
            << descriptors;

    filter = QServiceFilter();
    filter.setInterface("com.invalid.interface");
    QTest::newRow("by non-existing interface name")
            << serviceXml
            << filter
            << ServiceInterfaceDescriptorList();

    filter = QServiceFilter();
    filter.setServiceName("InvalidServiceName");
    QTest::newRow("by non-existing service name")
            << serviceXml
            << filter
            << ServiceInterfaceDescriptorList();

    //version lookup testing for existing interface
    //valid from first version onwards
    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "1.0");
    QTest::newRow("by version name 1.0 DefaultMatch, should find all B interfaces")
            << serviceXml
            << filter
            << descriptors.mid(1);

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "1.0", QServiceFilter::MinimumVersionMatch);
    QTest::newRow("by version name 1.0 MinimumMatch, should find all B interfaces")
            << serviceXml
            << filter
            << descriptors.mid(1);

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "1.0", QServiceFilter::ExactVersionMatch);
    QTest::newRow("by version name 1.0 ExactMatch, find B 1.0 only")
            << serviceXml
            << filter
            << descriptors.mid(1, 1);

    //valid with exact version match
    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "2.0");
    QTest::newRow("by version name 2.0 DefaultMatch, find B 2.0+")
            << serviceXml
            << filter
            << descriptors.mid(2);

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "2.0", QServiceFilter::MinimumVersionMatch);
    QTest::newRow("by version name 2.0 MinimumMatch, find B 2.0+")
            << serviceXml
            << filter
            << descriptors.mid(2);

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "2.0", QServiceFilter::ExactVersionMatch);
    QTest::newRow("by version name 2.0 ExactMatch, find B 2.0")
            << serviceXml
            << filter
            << descriptors.mid(2, 1);

    //valid but not exact version match
    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "1.9");
    QTest::newRow("by version name 1.9 DefaultMatch, find B 1.9+")
            << serviceXml
            << filter
            << descriptors.mid(2);

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "1.9", QServiceFilter::MinimumVersionMatch);
    QTest::newRow("by version name 1.9 MinimumMatch, find B 1.9+")
            << serviceXml
            << filter
            << descriptors.mid(2);

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "1.9", QServiceFilter::ExactVersionMatch);
    QTest::newRow("by version name 1.9 ExactMatch")
            << serviceXml
            << filter
            << ServiceInterfaceDescriptorList();

    //version doesn't exist yet
    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "3.9");
    QTest::newRow("by version name 3.9 DefaultMatch")
            << serviceXml
            << filter
            << ServiceInterfaceDescriptorList();

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "3.9", QServiceFilter::MinimumVersionMatch);
    QTest::newRow("by version name 3.9 MinimumMatch")
            << serviceXml
            << filter
            << ServiceInterfaceDescriptorList();

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "3.9", QServiceFilter::ExactVersionMatch);
    QTest::newRow("by version name 3.9 ExactMatch")
            << serviceXml
            << filter
            << ServiceInterfaceDescriptorList();

    //invalid version tag 1 -> match anything
    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "x3.9");
    QTest::newRow("by version name x3.9 DefaultMatch")
            << serviceXml<< filter
            << descriptors;

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "x3.9", QServiceFilter::MinimumVersionMatch);
    QTest::newRow("by version name x3.9 MinimumMatch")
            << serviceXml
            << filter
            << descriptors;

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "x3.9", QServiceFilter::ExactVersionMatch);
    QTest::newRow("by version name x3.9 ExactMatch")
            << serviceXml
            << filter
            << descriptors;

    //envalid/empty version tag
    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "");
    QTest::newRow("by empty version string DefaultMatch")
            << serviceXml
            << filter
            << descriptors.mid(1);

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "", QServiceFilter::MinimumVersionMatch);
    QTest::newRow("by empty version string MinimumMatch")
            << serviceXml
            << filter
            << descriptors.mid(1);

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "", QServiceFilter::ExactVersionMatch); //what's the result of this?
    QTest::newRow("by empty version string ExactMatch")
            << serviceXml
            << filter
            << descriptors.mid(1);

    //invalid version tag 2
    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "abc");
    QTest::newRow("by version name abc DefaultMatch")
            << serviceXml<< filter
            << descriptors;

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "abc", QServiceFilter::MinimumVersionMatch);
    QTest::newRow("by version name abc MinimumMatch")
            << serviceXml<< filter
            << descriptors;

    filter = QServiceFilter();
    filter.setInterface("com.nokia.qt.TestInterfaceB", "abc", QServiceFilter::ExactVersionMatch);
    QTest::newRow("by version name abc ExactMatch")
            << serviceXml
            << filter
            << descriptors;
}

void tst_QServiceManager::findInterfaces_scope()
{
    QFETCH(QService::Scope, scope_add);
    QFETCH(QService::Scope, scope_find);
    QFETCH(bool, expectFound);

    QByteArray xml = createServiceXml("SomeTestService",
            createInterfaceXml("com.nokia.qt.TestInterface"), m_validPluginNames[0]);
    QBuffer buffer(&xml);

    QServiceManager mgrUser(QService::UserScope);
    QServiceManager mgrSystem(QService::SystemScope);

    QServiceManager &mgrAdd = scope_add == QService::UserScope ? mgrUser : mgrSystem;
    QServiceManager &mgrFind = scope_find == QService::UserScope ? mgrUser : mgrSystem;

    QList<QServiceInterfaceDescriptor> result = mgrFind.findInterfaces(QString());
    QVERIFY(result.isEmpty());

    QVERIFY2(mgrAdd.addService(&buffer), PRINT_ERR(mgrAdd));
    result = mgrFind.findInterfaces("SomeTestService");
    QCOMPARE(!result.isEmpty(), expectFound);

    result = mgrFind.findInterfaces(QString());
    if (expectFound)
        QVERIFY(result.count() == 1);
    else
        QVERIFY(result.isEmpty());

    result = mgrFind.findInterfaces("NonExistingService");
    QVERIFY(result.isEmpty());
}

void tst_QServiceManager::findInterfaces_scope_data()
{
    findServices_scope_data();
}


void tst_QServiceManager::loadInterface_string()
{
    // The sampleservice.xml and sampleservice2.xml services in
    // tests/sampleserviceplugin and tests/sampleserviceplugin2 implement a
    // common interface, "com.nokia.qt.TestInterfaceA". If both are
    // registered, loadInterface(QString) should return the correct one
    // depending on which is set as the default.

    // Real servicenames and classnames
    QString serviceA = "SampleService";
    QString serviceAClassName = "SampleServicePluginClass";
    QString serviceB = "SampleService2";
    QString serviceBClassName = "SampleServicePluginClass2";

    QObject *obj = 0;
    QServiceManager mgr;
    QString commonInterface = "com.nokia.qt.TestInterfaceA";

    // Add first service. Adds the service described in
    // c/Private/<uid3 of this executable>/plugins/xmldata/sampleservice.xml
    QVERIFY2(mgr.addService(xmlTestDataPath("sampleservice.xml")), PRINT_ERR(mgr));

    obj = mgr.loadInterface(commonInterface);
    QVERIFY(obj != 0);
    QCOMPARE(QString(obj->metaObject()->className()), serviceAClassName);
    delete obj;
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    QCoreApplication::processEvents();

    // Add first service. Adds the service described in
    // c/Private/<uid3 of this executable>/plugins/xmldata/sampleservice2.xml
    QVERIFY2(mgr.addService(xmlTestDataPath("sampleservice2.xml")), PRINT_ERR(mgr));

    // if first service is set as default, it should be returned
    QVERIFY(mgr.setInterfaceDefault(serviceA, commonInterface));
    obj = mgr.loadInterface(commonInterface);
    QVERIFY(obj != 0);
    QCOMPARE(QString(obj->metaObject()->className()), serviceAClassName);
    delete obj;
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    QCoreApplication::processEvents();

    // if second service is set as default, it should be returned
    QVERIFY(mgr.setInterfaceDefault(serviceB, commonInterface));
    obj = mgr.loadInterface(commonInterface);
    QVERIFY(obj != 0);
    QCOMPARE(QString(obj->metaObject()->className()), serviceBClassName);
    delete obj;
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
}

void tst_QServiceManager::loadInterface_descriptor()
{
    QFETCH(QServiceInterfaceDescriptor, descriptor);
    QFETCH(QString, className);

    QObject* obj;
    {
        QServiceManager mgr;
        obj = mgr.loadInterface(descriptor);
        QVERIFY(obj != 0);
        QCOMPARE(QString::fromLatin1(obj->metaObject()->className()), className);
    }

    QVERIFY(obj != 0);

    delete obj;
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
}

void tst_QServiceManager::loadInterface_descriptor_data()
{
    QTest::addColumn<QServiceInterfaceDescriptor>("descriptor");
    QTest::addColumn<QString>("className");

    for (int i = 0; i < m_validPluginNames.size(); ++i) {
        if (const char *expectedClassName = expectedPluginClassNames[i]) {
            QServiceInterfaceDescriptor descriptor;
            QServiceInterfaceDescriptorPrivate *priv = new QServiceInterfaceDescriptorPrivate;
            priv->interfaceName = "com.nokia.qt.TestInterfaceA";    // needed by service plugin implementation
            priv->attributes[QServiceInterfaceDescriptor::Location] =  m_validPluginPaths.at(i);
            QServiceInterfaceDescriptorPrivate::setPrivate(&descriptor, priv);
            QTest::newRow(qPrintable(m_validPluginNames.at(i)))
                    << descriptor
                    << QString(QLatin1String(expectedClassName));
        }
    }
}

void tst_QServiceManager::loadInterface_testLoadedObjectAttributes()
{
    QServiceInterfaceDescriptor descriptor;
    QServiceInterfaceDescriptorPrivate *priv = new QServiceInterfaceDescriptorPrivate;
    priv->interfaceName = "com.nokia.qt.TestInterfaceA";    // needed by service plugin implementation
    priv->attributes[QServiceInterfaceDescriptor::Location] =  m_validPluginPaths.at(2);

    QServiceInterfaceDescriptorPrivate::setPrivate(&descriptor, priv);

    QServiceManager mgr;
    QObject *obj = mgr.loadInterface(descriptor);
    QVERIFY(obj != 0);

    bool invokeOk = false;
    QString name;

    // check attributes
    QMetaProperty nameProperty = obj->metaObject()->property(obj->metaObject()->indexOfProperty("name"));
    QVERIFY(nameProperty.isValid());
    QVERIFY(nameProperty.write(obj, "A service name"));
    QCOMPARE(nameProperty.read(obj), QVariant("A service name"));

    // check signals
    QVERIFY(obj->metaObject()->indexOfSignal("someSignal()") >= 0);
    QSignalSpy spy(obj, SIGNAL(someSignal()));
    invokeOk = QMetaObject::invokeMethod(obj, "someSignal");
    QVERIFY(invokeOk);
    QVERIFY(spy.count() == 1);

    // check slots
    invokeOk = QMetaObject::invokeMethod(obj, "callSlot");
    QVERIFY(invokeOk);
    invokeOk = QMetaObject::invokeMethod(obj, "callSlotAndSetName", Q_ARG(QString, "test string"));
    QVERIFY(invokeOk);
    invokeOk = QMetaObject::invokeMethod(obj, "callSlotAndReturnName", Q_RETURN_ARG(QString, name));
    QVERIFY(invokeOk);
    QCOMPARE(name, QString("test string"));

    // check invokables
    invokeOk = QMetaObject::invokeMethod(obj, "callInvokable");
    QVERIFY(invokeOk);

    // call method on a returned object
    QObject *embeddedObj = 0;
    int value = 0;
    invokeOk = QMetaObject::invokeMethod(obj, "embeddedTestService", Q_RETURN_ARG(QObject*, embeddedObj));
    QVERIFY(invokeOk);
    invokeOk = QMetaObject::invokeMethod(embeddedObj, "callWithInt", Q_RETURN_ARG(int, value), Q_ARG(int, 10));
    QVERIFY(invokeOk);
    QCOMPARE(value, 10);

    // call a method that is not invokable via meta system
    invokeOk = QMetaObject::invokeMethod(embeddedObj, "callNormalMethod");
    QVERIFY(!invokeOk);

    delete obj;
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
}

void tst_QServiceManager::loadLocalTypedInterface()
{
    QServiceManager mgr;

    QServiceInterfaceDescriptor descriptor;
    QServiceInterfaceDescriptorPrivate *priv = new QServiceInterfaceDescriptorPrivate;
    priv->interfaceName = "com.nokia.qt.TestInterfaceA";    // needed by service plugin implementation
    priv->attributes[QServiceInterfaceDescriptor::Location] = m_validPluginPaths.at(0);
    QServiceInterfaceDescriptorPrivate::setPrivate(&descriptor, priv);

    //use manual descriptor -> avoid database involvement
    SampleServicePluginClass *plugin = 0;
    plugin = mgr.loadLocalTypedInterface<SampleServicePluginClass>(descriptor);

    QVERIFY(plugin != 0);

    delete plugin;
    plugin = 0;
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    QCoreApplication::processEvents();

    //use database descriptor
    QFile file1(xmlTestDataPath("sampleservice.xml"));
    QVERIFY2(file1.exists(), qPrintable(QString("%1: %2").arg(xmlTestDataPath("sampleservice.xml")).arg(file1.errorString()) ));
    QVERIFY2(mgr.addService(&file1), PRINT_ERR(mgr));

    QCOMPARE(mgr.findServices("com.nokia.qt.TestInterfaceA"), QStringList("SampleService"));
    QCOMPARE(mgr.findServices("com.nokia.qt.TestInterfaceB"), QStringList("SampleService"));
    QCOMPARE(mgr.findServices("com.nokia.qt.TestInterfaceC"), QStringList("SampleService"));
    QList<QServiceInterfaceDescriptor> ifaces = mgr.findInterfaces("SampleService");
    QList<SampleServicePluginClass*> serviceObjects;
    QVERIFY(ifaces.count() == 3);
    for (int i = 0; i<ifaces.count(); i++) {
        plugin = mgr.loadLocalTypedInterface<SampleServicePluginClass>(ifaces.at(i));

        if (ifaces.at(i).interfaceName() == "com.nokia.qt.TestInterfaceC") {
            QVERIFY(plugin == 0);
        } else {
            QVERIFY(plugin != 0);
            plugin->testSlotOne();
            serviceObjects.append(plugin);
        }
    }

    //test for a bug where two service instances from same plugin
    //caused a crash when the first instance was deleted and
    //the second instance called
    QVERIFY(serviceObjects.count() == 2);
    delete serviceObjects.takeFirst();
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    QCoreApplication::processEvents();

    plugin = serviceObjects.takeFirst();
    plugin->testSlotOne();
    delete plugin;
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    QCoreApplication::processEvents();


    //use default lookup
    plugin = mgr.loadLocalTypedInterface<SampleServicePluginClass>("com.nokia.qt.TestInterfaceA");
    QVERIFY(plugin != 0);

    delete plugin;
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
    plugin = 0;

    //use totally wrong but QObject based template class type
    QFile *w = mgr.loadLocalTypedInterface<QFile>("com.nokia.qt.TestInterfaceA");
    QVERIFY(!w);

    //use non QObject based template class type
    //doesn't compile and should never compile
    //QString* s = mgr.loadLocalTypedInterface<QString>("com.nokia.qt.TestInterfaceA", &context, &session);
    //QVERIFY(!s);
}

#define TST_QSERVICEMANAGER_ADD_SERVICE(paramType, file) { \
    if (paramType == "QString") \
        QVERIFY2(mgr.addService(file->fileName()), PRINT_ERR(mgr)); \
    else if (paramType == "QIODevice") \
        QVERIFY2(mgr.addService(file), PRINT_ERR(mgr)); \
    else \
        QFAIL("tst_QServiceManager::addService(): Bad test parameter"); \
}

void tst_QServiceManager::addService()
{
    QFETCH(QString, paramType);

    QServiceManager mgr;

    QString commonInterface = "com.qt.serviceframework.tests.CommonInterface";
    QByteArray xmlA = createServiceXml("ServiceA", createInterfaceXml(commonInterface), m_validPluginNames[0]);
    QByteArray xmlB = createServiceXml("ServiceB", createInterfaceXml(commonInterface), m_validPluginNames[1]);

    QTemporaryFile *tempFileA = new QTemporaryFile(this);
    QVERIFY2(tempFileA->open(), "Can't open temp file A");
    tempFileA->write(xmlA);
    tempFileA->seek(0);
    QTemporaryFile *tempFileB = new QTemporaryFile(this);
    QVERIFY2(tempFileB->open(), "Can't open temp file B");
    tempFileB->write(xmlB);
    tempFileB->seek(0);

    TST_QSERVICEMANAGER_ADD_SERVICE(paramType, tempFileA);
    QCOMPARE(mgr.findServices(), QStringList("ServiceA"));

    // the service should be automatically set as the default for its
    // implemented interfaces since it was the first service added for them
    QCOMPARE(mgr.interfaceDefault(commonInterface).serviceName(), QString("ServiceA"));
    QCOMPARE(mgr.interfaceDefault(commonInterface).serviceName(), QString("ServiceA"));

    // add second service
    TST_QSERVICEMANAGER_ADD_SERVICE(paramType, tempFileB);
    QStringList result = mgr.findServices();
    QCOMPARE(result.count(), 2);
    QVERIFY(result.contains("ServiceA"));
    QVERIFY(result.contains("ServiceB"));

    // the default does not change once ServiceB is added
    QCOMPARE(mgr.interfaceDefault(commonInterface).serviceName(), QString("ServiceA"));
    QCOMPARE(mgr.interfaceDefault(commonInterface).serviceName(), QString("ServiceA"));

    delete tempFileA;
    delete tempFileB;
}

void tst_QServiceManager::addService_data()
{
    QTest::addColumn<QString>("paramType");

    QTest::newRow("string") << "QString";
    QTest::newRow("iodevice") << "QIODevice";
}

void tst_QServiceManager::addService_testInvalidServiceXml()
{
    QBuffer buffer;
    QServiceManager mgr;

    QVERIFY2(!mgr.addService(&buffer), PRINT_ERR(mgr));

    // a service with no interfaces
    QString xml = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
    xml += "<service><name>SomeService</name><filepath>" + m_validPluginNames.first() + "</filepath>\n";
    xml += "</service>\n";
    buffer.close();
    buffer.setData(xml.toLatin1());
    QVERIFY(!mgr.addService(&buffer));

    QVERIFY2(mgr.findServices().isEmpty(), PRINT_ERR(mgr));
}

void tst_QServiceManager::addService_testPluginLoading()
{
    QFETCH(QString, pluginPath);
    QFETCH(bool, isAdded);

    QServiceManager mgr;

    QByteArray xml = createServiceXml("SomeService", createInterfaceXml("com.qt.serviceframework.Interface"), pluginPath);
    QBuffer buffer(&xml);
    QVERIFY2(mgr.addService(&buffer) == isAdded, PRINT_ERR(mgr));

    // the service should not be added if the service plugin cannot be loaded
    if (!isAdded)
        QVERIFY(mgr.findServices().isEmpty());
}

void tst_QServiceManager::addService_testPluginLoading_data()
{
    QTest::addColumn<QString>("pluginPath");
    QTest::addColumn<bool>("isAdded");

    QTest::newRow("valid path") << m_validPluginNames.first() << true;
    QTest::newRow("invalid path") << "no_such_plugin" << false;
}

void tst_QServiceManager::addService_testInstallService()
{
    QSettings settings("com.nokia.qt.serviceframework.tests", "SampleServicePlugin");
    QCOMPARE(settings.value("installed").toBool(), false);

    QServiceManager mgr;
    QVERIFY2(mgr.addService(xmlTestDataPath("sampleservice.xml")), PRINT_ERR(mgr));
    QCOMPARE(mgr.findServices(), QStringList("SampleService"));

    // test installService() was called on the plugin
    QCOMPARE(settings.value("installed").toBool(), true);
}

void tst_QServiceManager::removeService()
{
    QServiceManager mgr;

    QVERIFY(!mgr.removeService("NonExistentService"));

    QSettings settings("com.nokia.qt.serviceframework.tests", "SampleServicePlugin");
    QCOMPARE(settings.value("installed").toBool(), false);

    qDebug() << "open" << xmlTestDataPath("sampleservice.xml");
    QVERIFY2(mgr.addService(xmlTestDataPath("sampleservice.xml")), PRINT_ERR(mgr));
    QCOMPARE(mgr.findServices("com.nokia.qt.TestInterfaceA"), QStringList("SampleService"));
    QCOMPARE(settings.value("installed").toBool(), true);

    QVERIFY(mgr.removeService("SampleService"));
    QVERIFY(mgr.findServices().isEmpty());
    QCOMPARE(mgr.findServices("com.nokia.qt.TestInterfaceA"), QStringList());
    QCOMPARE(settings.value("installed").toBool(), false);

    // add it again, should still work
    QVERIFY2(mgr.addService(xmlTestDataPath("sampleservice.xml")), PRINT_ERR(mgr));
    QCOMPARE(mgr.findServices("com.nokia.qt.TestInterfaceA"), QStringList("SampleService"));
    QCOMPARE(settings.value("installed").toBool(), true);
}

void tst_QServiceManager::setInterfaceDefault_strings()
{
    QServiceManager mgr;
    QString interfaceName = "com.nokia.qt.serviceframework.tests.AnInterface";
    DescriptorAttributes attributes;
    QServiceInterfaceDescriptor descriptor;
    QByteArray xml;

    attributes[QServiceInterfaceDescriptor::Location] = m_validPluginNames[0];
    descriptor = createDescriptor(interfaceName, 1, 0, "ServiceA", attributes);
    xml = createServiceXml("ServiceA",
            createInterfaceXml(QList<QServiceInterfaceDescriptor>() << descriptor),
            attributes[QServiceInterfaceDescriptor::Location].toString());
    QBuffer buffer(&xml);

    // fails if the specified interface hasn't been registered
    QCOMPARE(mgr.setInterfaceDefault("ServiceA", interfaceName), false);

    // now it works
    QVERIFY2(mgr.addService(&buffer), PRINT_ERR(mgr));
    QCOMPARE(mgr.setInterfaceDefault("ServiceA", interfaceName), true);
    QCOMPARE(mgr.interfaceDefault(interfaceName), descriptor);

    // replace the default with another service
    attributes[QServiceInterfaceDescriptor::Location] = m_validPluginNames[1];
    descriptor = createDescriptor(interfaceName, 1, 0, "ServiceB", attributes);
    xml = createServiceXml("ServiceB",
            createInterfaceXml(QList<QServiceInterfaceDescriptor>() << descriptor),
            attributes[QServiceInterfaceDescriptor::Location].toString());
    buffer.close();
    buffer.setData(xml);
    QVERIFY2(mgr.addService(&buffer), PRINT_ERR(mgr));
    QCOMPARE(mgr.setInterfaceDefault("ServiceB", interfaceName), true);
    QCOMPARE(mgr.interfaceDefault(interfaceName), descriptor);

    // bad arguments
    QCOMPARE(mgr.setInterfaceDefault("", ""), false);
    QCOMPARE(mgr.setInterfaceDefault("blah", "blah"), false);
    QCOMPARE(mgr.setInterfaceDefault("SampleService", ""), false);
}

void tst_QServiceManager::setInterfaceDefault_strings_multipleInterfaces()
{
    QServiceManager mgr;
    QString interfaceName = "com.nokia.qt.serviceframework.tests.AnInterface";
    DescriptorAttributes attributes;
    QServiceInterfaceDescriptor descriptor;
    QByteArray xml;

    // if there are multiple interfaces, the default should be the latest version
    attributes[QServiceInterfaceDescriptor::Location] = m_validPluginNames[0];
    QList<QServiceInterfaceDescriptor> descriptorList;
    descriptorList << createDescriptor(interfaceName, 1, 0, "ServiceC", attributes)
                   << createDescriptor(interfaceName, 1, 8, "ServiceC", attributes)
                   << createDescriptor(interfaceName, 1, 3, "ServiceC", attributes);
    xml = createServiceXml("ServiceC", createInterfaceXml(descriptorList),
            attributes[QServiceInterfaceDescriptor::Location].toString());
    QBuffer buffer(&xml);
    QVERIFY2(mgr.addService(&buffer), PRINT_ERR(mgr));
    QCOMPARE(mgr.setInterfaceDefault("ServiceC", interfaceName), true);
    QCOMPARE(mgr.interfaceDefault(interfaceName), descriptorList[1]);
}

void tst_QServiceManager::setInterfaceDefault_descriptor()
{
    QFETCH(QService::Scope, scope_add);
    QFETCH(QService::Scope, scope_find);
    QFETCH(bool, expectFound);

    QServiceManager mgr(scope_add);
    QServiceInterfaceDescriptor desc;

    QString interfaceName = "com.nokia.qt.serviceframework.TestInterface";
    DescriptorAttributes attributes;
    attributes[QServiceInterfaceDescriptor::Location] = m_validPluginNames.first();

    QCOMPARE(mgr.setInterfaceDefault(desc), false);

    desc = createDescriptor(interfaceName, 1, 0, "SomeService", attributes,
            scope_add);

    // fails if the specified interface hasn't been registered
    QCOMPARE(mgr.setInterfaceDefault(desc), false);

    // now it works
    QByteArray xml = createServiceXml("SomeService",
            createInterfaceXml(QList<QServiceInterfaceDescriptor>() << desc), m_validPluginNames.first());
    QBuffer buffer(&xml);
    QVERIFY2(mgr.addService(&buffer), PRINT_ERR(mgr));
    QCOMPARE(mgr.setInterfaceDefault(desc), true);

    QCOMPARE(mgr.interfaceDefault(interfaceName), desc);

    QServiceManager mgrWithOtherScope(scope_find);
    QCOMPARE(mgrWithOtherScope.interfaceDefault(interfaceName).isValid(), expectFound);
}

void tst_QServiceManager::setInterfaceDefault_descriptor_data()
{
    QTest::addColumn<QService::Scope>("scope_add");
    QTest::addColumn<QService::Scope>("scope_find");
    QTest::addColumn<bool>("expectFound");

    QTest::newRow("user scope")
            << QService::UserScope << QService::UserScope << true;
    QTest::newRow("system scope")
            << QService::SystemScope << QService::SystemScope << true;

    QTest::newRow("user scope - add, system scope - find")
            << QService::UserScope << QService::SystemScope << false;
    QTest::newRow("system scope - add, user scope - find")
            << QService::SystemScope << QService::UserScope << true;
}

void tst_QServiceManager::interfaceDefault()
{
    QServiceManager mgr;
    QVERIFY(!mgr.interfaceDefault("").isValid());
}

void tst_QServiceManager::serviceAdded()
{
    QFETCH(QByteArray, xml);
    QFETCH(QString, serviceName);
    QFETCH(QService::Scope, scope_modify);
    QFETCH(QService::Scope, scope_listen);
    QFETCH(bool, expectSignal);

    QBuffer buffer;
    buffer.setData(xml);
    QServiceManager mgr_modify(scope_modify);
    QServiceManager mgr_listen(scope_listen);

    // ensure mgr.connectNotify() is called
    ServicesListener *listener = new ServicesListener;
    connect(&mgr_listen, SIGNAL(serviceAdded(QString,QService::Scope)),
            listener, SLOT(serviceAdded(QString,QService::Scope)));

    QSignalSpy spyAdd(&mgr_listen, SIGNAL(serviceAdded(QString,QService::Scope)));
    QVERIFY2(mgr_modify.addService(&buffer), PRINT_ERR(mgr_modify));

    if (!expectSignal) {
        QTest::qWait(2000);
        QCOMPARE(spyAdd.count(), 0);
    } else {
        QTRY_COMPARE(spyAdd.count(), 1);
    }

    if (expectSignal) {
        QCOMPARE(spyAdd.at(0).at(0).toString(), serviceName);
        QCOMPARE( listener->params.at(0).second , scope_modify);
    }
    listener->params.clear();

    // Pause between file changes so they are detected separately
    QTest::qWait(2000);

    QSignalSpy spyRemove(&mgr_listen, SIGNAL(serviceRemoved(QString,QService::Scope)));
    QVERIFY(mgr_modify.removeService(serviceName));

    if (!expectSignal) {
        QTest::qWait(2000);
        QCOMPARE(spyRemove.count(), 0);
    } else {
        QTRY_COMPARE(spyRemove.count(), 1);
    }

#if !defined (Q_OS_WIN)
    // on win, cannot delete the database while it is in use
    // try it again after deleting the database
    deleteTestDatabasesAndWaitUntilDone();
#else
    // Pause between file changes so they are detected separately
    QTest::qWait(2000);
#endif

    spyAdd.clear();
    buffer.seek(0);
    QVERIFY2(mgr_modify.addService(&buffer), PRINT_ERR(mgr_modify));
    if (!expectSignal) {
        QTest::qWait(2000);
        QCOMPARE(spyAdd.count(), 0);
    } else {
        QTRY_COMPARE(spyAdd.count(), 1);
    }
    if (expectSignal) {
        QCOMPARE(spyAdd.at(0).at(0).toString(), serviceName);
        QCOMPARE(listener->params.at(0).second, scope_modify);
    }

    // ensure mgr.disconnectNotify() is called
    disconnect(&mgr_listen, SIGNAL(serviceRemoved(QString,QService::Scope)),
            listener, SLOT(serviceRemoved(QString,QService::Scope)));

    delete listener;
}

void tst_QServiceManager::serviceAdded_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QString>("serviceName");
    QTest::addColumn<QService::Scope>("scope_modify");
    QTest::addColumn<QService::Scope>("scope_listen");
    QTest::addColumn<bool>("expectSignal");

    QFile file1(xmlTestDataPath("sampleservice.xml"));
    QVERIFY(file1.open(QIODevice::ReadOnly));
    QFile file2(xmlTestDataPath("testserviceplugin.xml"));
    QVERIFY(file2.open(QIODevice::ReadOnly));

    QByteArray file1Data = file1.readAll();

    QTest::newRow("SampleService, user scope") << file1Data << "SampleService"
            << QService::UserScope << QService::UserScope << true;
    QTest::newRow("TestService, user scope") << file2.readAll() << "TestService"
            << QService::UserScope << QService::UserScope << true;

    QTest::newRow("system scope") << file1Data << "SampleService"
            << QService::SystemScope << QService::SystemScope << true;
    QTest::newRow("modify as user, listen in system") << file1Data << "SampleService"
            << QService::UserScope << QService::SystemScope << false;
    QTest::newRow("modify as system, listen in user") << file1Data << "SampleService"
            << QService::SystemScope << QService::UserScope << true;
}

void tst_QServiceManager::serviceRemoved()
{
    QFETCH(QByteArray, xml);
    QFETCH(QString, serviceName);
    QFETCH(QService::Scope, scope_modify);
    QFETCH(QService::Scope, scope_listen);
    QFETCH(bool, expectSignal);

    QBuffer buffer;
    buffer.setData(xml);
    QServiceManager mgr_modify(scope_modify);
    QServiceManager mgr_verify(scope_listen);

    // ensure mgr.connectNotify() is called
    ServicesListener *listenerVerify = new ServicesListener;
    connect(&mgr_verify, SIGNAL(serviceAdded(QString,QService::Scope)),
            listenerVerify, SLOT(serviceAdded(QString,QService::Scope)));

    QSignalSpy spyAddMod(&mgr_verify, SIGNAL(serviceAdded(QString,QService::Scope)));
    QVERIFY2(mgr_modify.addService(&buffer), PRINT_ERR(mgr_modify));

    if (!expectSignal) {
        QTest::qWait(2000);
        QCOMPARE(spyAddMod.count(), 0);
    } else {
        QTRY_COMPARE(spyAddMod.count(), 1);
    }

    // Pause between file changes so they are detected separately
    QTest::qWait(2000);

    QServiceManager mgr_listen(scope_listen);
    QSignalSpy spyAdd(&mgr_listen, SIGNAL(serviceAdded(QString,QService::Scope)));

    // ensure mgr.connectNotify() is called
    ServicesListener *listener = new ServicesListener;
    connect(&mgr_listen, SIGNAL(serviceRemoved(QString,QService::Scope)),
            listener, SLOT(serviceRemoved(QString,QService::Scope)));

    QSignalSpy spyRemove(&mgr_listen, SIGNAL(serviceRemoved(QString,QService::Scope)));
    QVERIFY(mgr_modify.removeService(serviceName));

    if (!expectSignal) {
        QTest::qWait(2000);
        QCOMPARE(spyRemove.count(), 0);
    } else {
        QTRY_COMPARE(spyRemove.count(), 1);
    }
    if (expectSignal) {
        QCOMPARE(spyRemove.at(0).at(0).toString(), serviceName);
        QCOMPARE(listener->params.at(0).second , scope_modify);
    }
    listener->params.clear();

#if !defined (Q_OS_WIN)
    // on win, cannot delete the database while it is in use
    // try it again after deleting the database
    deleteTestDatabasesAndWaitUntilDone();
#else
    // Pause between file changes so they are detected separately
    QTest::qWait(2000);
#endif
    spyAdd.clear();
    buffer.seek(0);
    QVERIFY2(mgr_modify.addService(&buffer), PRINT_ERR(mgr_modify));
    if (!expectSignal) {
        QTest::qWait(2000);
        QCOMPARE(spyAdd.count(), 0);
    } else {
        QTRY_COMPARE(spyAdd.count(), 1);
    }

    spyRemove.clear();

    // Pause between file changes so they are detected separately
    QTest::qWait(2000);

    QVERIFY(mgr_modify.removeService(serviceName));
    if (!expectSignal) {
        QTest::qWait(2000);
        QCOMPARE(spyRemove.count(), 0);
    } else {
        QTRY_COMPARE(spyRemove.count(), 1);
    }
    if (expectSignal) {
        QCOMPARE(spyRemove.at(0).at(0).toString(), serviceName);
        QCOMPARE(listener->params.at(0).second , scope_modify);
    }

    // ensure mgr.disconnectNotify() is called
    disconnect(&mgr_listen, SIGNAL(serviceRemoved(QString,QService::Scope)),
            listener, SLOT(serviceRemoved(QString,QService::Scope)));
    disconnect(&mgr_verify, SIGNAL(serviceRemoved(QString,QService::Scope)),
            listenerVerify, SLOT(serviceRemoved(QString,QService::Scope)));


    delete listener;
    delete listenerVerify;
}

void tst_QServiceManager::serviceRemoved_data()
{
    serviceAdded_data();
}

QTEST_MAIN(tst_QServiceManager)

#include "tst_qservicemanager.moc"
