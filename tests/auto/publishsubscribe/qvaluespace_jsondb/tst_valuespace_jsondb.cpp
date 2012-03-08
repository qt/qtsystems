/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the test suite of the Qt Toolkit.
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qdebug.h>
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <QtQml/QJSValue>
#include <QtQml/QJSEngine>
#include <qvaluespace.h>
#include "qvaluespace_p.h"
#include "qvaluespacepublisher.h"
#include "qvaluespacesubscriber.h"

#include <jsondblayer_p.h>

#include <private/jsondb-connection_p.h>

class TestQValueSpaceJsonDb : public QObject
{
    Q_OBJECT

public:
    TestQValueSpaceJsonDb();

private Q_SLOTS:
    void init();
    void cleanup();

    void testLayer_AddWatch();
    void testLayer_RemoveWatches();
    void testLayer_Children();
    void testLayer_Id();
    void testLayer_Item();
    void testLayer_LayerOptions();
    void testLayer_NotifyInterest();
    void testLayer_RemoveHandle();
    void testLayer_RemoveSubTree();
    void testLayer_RemoveValue();
    void testLayer_SetProperty();
    void testLayer_SetValue();
    void testLayer_SupportsInterestNotification();
    void testLayer_Sync();
    void testLayer_Value();
    void testLayer_Instance();

    void testPath_JsonDbPath();
    void testPath_JsonDbPathString();
    void testPath_JsonDbPathOther();
    void testPath_GetPath();
    void testPath_OperatorAssign();
    void testPath_OperatorEqual();
    void testPath_OperatorPlusString();
    void testPath_OperatorPlusPath();
    void testPath_GetIdentifier();

    void testHandle_JsonDbHandle();
    void testHandle_Value();
    void testHandle_SetValue();
    void testHandle_UnsetValue();
    void testHandle_Subscribe();
    void testHandle_Unsubscribe();
    void testHandle_Children();
    void testHandle_RemoveSubTree();

    void testAPI_PublisherPath();
    void testAPI_PublishSubscribe();
    void testAPI_Notification();

public slots:
    void contentsChanged();

private:
    JsonDbLayer *layer;
};

TestQValueSpaceJsonDb::TestQValueSpaceJsonDb()
{

}

void cleanupJsonDb()
{
    QVariantMap map = JsonDbConnection::makeQueryRequest("[*]");
    map = JsonDbConnection::instance()->sync(map).value<QVariantMap>();

    QVariant object;

    foreach (object, map["data"].value<QVariantList>()) {
        QVariantMap delMap = object.value<QVariantMap>();

        if (delMap.contains("identifier") && (delMap["identifier"].toString().startsWith("com.pstest"))) {
            delMap = JsonDbConnection::makeRemoveRequest(object);
            JsonDbConnection::instance()->sync(delMap);
        }
    }

    QTest::qWait(100);
}

void TestQValueSpaceJsonDb::init()
{
    layer = new JsonDbLayer();
}

void TestQValueSpaceJsonDb::cleanup()
{
    delete layer;

    cleanupJsonDb();
}


void createJsonObjects(const QStringList &objects)
{
    QJSValue sc;
    QJSEngine engine;

    foreach (QString object, objects) {
        sc = engine.evaluate("(" % object % ")");
        QVariantMap map = JsonDbConnection::makeCreateRequest(sc.toVariant());

        JsonDbConnection::instance()->sync(map).value<QVariantMap>();
    }
}

bool exists(QString query)
{
    QVariantMap map = JsonDbConnection::makeQueryRequest(query);

    map = JsonDbConnection::instance()->sync(map).value<QVariantMap>();

    if (map["length"].value<int>() == 0)
        return false;

    return true;
}

QVariantMap getObject(const QString &identifier)
{
    QVariantMap map = JsonDbConnection::makeQueryRequest(QString("[?identifier=\"%1\"]").arg(identifier));
    QVariantMap result = JsonDbConnection::instance()->sync(map).value<QVariantMap>();

    if (!result.contains("length") || (result["length"].value<int>() != 1))
        return QVariantMap();

    return result["data"].value<QVariantList>()[0].value<QVariantMap>();;
}


void TestQValueSpaceJsonDb::testLayer_AddWatch()
{
    // addWatch() is currently not implemented
    //QVERIFY2(false, "Not implemented!");
}

void TestQValueSpaceJsonDb::testLayer_RemoveWatches()
{
    // removeWatches() is currently not implemented
    //QVERIFY2(false, "Not implemented!");
}

void TestQValueSpaceJsonDb::testLayer_Children()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testChildren.app1\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testChildren.sys1\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testChildren.sub.app2\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testChildren.sub.sys2\", \"settings\": {\"setting1\":1}}";

    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com.pstest.testChildren", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        //QSet<QString> children = handle.children();
        QSet<QString> children = layer->children(quintptr(&handle));
        QVERIFY2(children.count() == 3, "children() method failed!");
        QVERIFY2(children.contains("app1"), "children() method failed!");
        QVERIFY2(children.contains("sub"), "children() method failed!");
        QVERIFY2(children.contains("sys1"), "children() method failed!");
    }
    catch(...) { }
}

void TestQValueSpaceJsonDb::testLayer_Id()
{
    QVERIFY2(layer->id() == QVALUESPACE_JSONDB_LAYER, "id() failed!");
}

void TestQValueSpaceJsonDb::testLayer_Item()
{
    JsonDbHandle* handle = reinterpret_cast<JsonDbHandle*>(layer->item(NULL, ""));
    QVERIFY2(handle != NULL, "item() failed!");
    QVERIFY2(handle->getPath() == "", "item() failed!");

    JsonDbHandle* handle2 = reinterpret_cast<JsonDbHandle*>(layer->item(NULL, "testItemLayer/b/c"));
    QVERIFY2(handle2 != NULL, "item() failed!");
    QVERIFY2(handle2->getPath() == "testItemLayer.b.c", "item() failed!");

    JsonDbHandle* handle3 = reinterpret_cast<JsonDbHandle*>(layer->item(quintptr(handle), ""));
    QVERIFY2(handle3 != NULL, "item() failed!");
    QVERIFY2(handle3->getPath() == "", "item() failed!");

    JsonDbHandle* handle4 = reinterpret_cast<JsonDbHandle*>(layer->item(quintptr(handle2), "d/e"));
    QVERIFY2(handle4 != NULL, "item() failed!");
    QVERIFY2(handle4->getPath() == "testItemLayer.b.c.d.e", "item() failed!");
}

void TestQValueSpaceJsonDb::testLayer_LayerOptions()
{
    QVERIFY2((layer->layerOptions() | QValueSpace::WritableLayer)
             && (layer->layerOptions() | QValueSpace::PermanentLayer), "Not implemented!");
}

void TestQValueSpaceJsonDb::testLayer_NotifyInterest()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testNotifyInterest.app\", \"setting1\":1}";

    // Subscribe for settings objects under com.testNotifyInterest
    JsonDbHandle handle(NULL, "com.pstest.testNotifyInterest", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    layer->notifyInterest(quintptr(&handle), true);

    try {
        QSignalSpy spy(layer, SIGNAL(valueChanged()));
        QVERIFY(spy.isValid());
        QCOMPARE(spy.count(), 0);

        createJsonObjects(objects);

        QTest::qWait(100);
        QCOMPARE(spy.count(), 1);

        layer->notifyInterest(quintptr(&handle), false);

        createJsonObjects(objects);

        QTest::qWait(100);
        QCOMPARE(spy.count(), 1);
    } catch(...) {
        return;
    }
}

void TestQValueSpaceJsonDb::testLayer_RemoveHandle()
{
    JsonDbHandle* handle = new JsonDbHandle(NULL, "", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QSignalSpy spy(handle, SIGNAL(destroyed()));
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), 0);

    layer->removeHandle(quintptr(handle));

    QTest::qWait(100);
    QCOMPARE(spy.count(), 1);
}

void TestQValueSpaceJsonDb::testLayer_RemoveSubTree()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testRemoveSubTree\", \"settings\": {\"setting1\":1}}";

    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "/com.pstest.testRemoveSubTree", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        // Deleting a subtree is not supported
        QVERIFY2(!layer->removeSubTree(NULL, quintptr(&handle)), "removeSubTree() failed!");
    }
    catch(...) { }
}

void TestQValueSpaceJsonDb::testLayer_RemoveValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testUnsetValue.app\", \"setting1\":1}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testUnsetValue.system\", \"setting2\":2}";
    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com.pstest.testUnsetValue.app", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        // Settings may not be deleted
        QVERIFY2(!layer->removeValue(NULL, quintptr(&handle), "setting1"), "removeValue() failed!");//handle.unsetValue("setting1"), "unsetValue()");

        // Settings objects may not be deleted
        QVERIFY2(!layer->removeValue(NULL, quintptr(&handle), ""), "removeValue() failed!");

        JsonDbHandle handle2(NULL, "com.pstest.testUnsetValue.system", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        // Settings may not be deleted
        QVERIFY2(!layer->removeValue(NULL, quintptr(&handle2), "setting2"), "removeValue() failed!");

        // Settings objects may not be deleted
        QVERIFY2(!layer->removeValue(NULL, quintptr(&handle2), ""), "removeValue() failed!");
    } catch(...) {}
}

void TestQValueSpaceJsonDb::testLayer_SetProperty()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testSetProperty.app\", \"setting1\":1}";

    // Subscribe for settings objects under com.testSubscribe
    JsonDbHandle handle(NULL, "com.pstest.testSetProperty", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    layer->setProperty(quintptr(&handle), QAbstractValueSpaceLayer::Publish);

    try {
        QSignalSpy spy(layer, SIGNAL(handleChanged(quintptr)));
        QVERIFY(spy.isValid());
        QCOMPARE(spy.count(), 0);

        createJsonObjects(objects);

        QTest::qWait(100);
        QCOMPARE(spy.count(), 1);

        layer->setProperty(quintptr(&handle), (QAbstractValueSpaceLayer::Properties)0);

        createJsonObjects(objects);

        QTest::qWait(100);
        QCOMPARE(spy.count(), 1);
    } catch(...) {
        return;
    }
}

void TestQValueSpaceJsonDb::testLayer_SetValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testSetValue.app\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testSetValue.system\", \"settings\": {\"setting2\":2}}";
    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com.pstest.testSetValue.app", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        QVERIFY2(layer->setValue(NULL, quintptr(&handle), "setting1", 42), "setValue() failed!");

        QVariant value;
        QVERIFY2(handle.value("setting1", &value), "value() failed!");
        QVERIFY2(value.value<int>() == 42, "setValue() failed!");

        JsonDbHandle handle2(NULL, "com.pstest.testSetValue.app.setting1", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        QVERIFY2(handle2.value("", &value), "value() failed!");
        QVERIFY2(value.value<int>() == 42, "setValue() failed!");

        JsonDbHandle handle3(NULL, "com.pstest.testSetValue.system", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        QVERIFY2(layer->setValue(NULL, quintptr(&handle3), "setting2", 42), "setValue() failed!");

        QVERIFY2(handle3.value("setting2", &value), "value() failed!");
        QVERIFY2(value.value<int>() == 42, "setValue() failed!");

        // Creating a new setting is not allowed
        QVERIFY2(!layer->setValue(NULL, quintptr(&handle3), "new_setting", 42), "setValue() failed!");

        // Changing a whole settings object is not allowed
        QVERIFY2(!layer->setValue(NULL, quintptr(&handle3), "", 42), "setValue() failed!");

        // Creating a new settings object is not allowed
        JsonDbHandle handle4(NULL, "com.pstest.testSetValue.system2", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        QVERIFY2(!layer->setValue(NULL, quintptr(&handle4), "", 42), "setValue() failed!");
    } catch(...) {}
}

void TestQValueSpaceJsonDb::testLayer_SupportsInterestNotification()
{
    QVERIFY2(layer->supportsInterestNotification(), "supportsInterestNotification() failed!");
}

void TestQValueSpaceJsonDb::testLayer_Sync()
{
    // Nothing to test because the back-end isn't assynchronous
}

void TestQValueSpaceJsonDb::testLayer_Value()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testValueLayer.app\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testValueLayer.system\", \"settings\": {\"setting2\":2}}";
    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com/pstest/testValueLayer/app", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        QVariant value;

        QVERIFY2(layer->value(quintptr(&handle), "setting1", &value), "value() failed!");
        int intValue = value.value<int>();
        QVERIFY2(intValue == 1, "value() failed!");

        // Try to access the whole settings object
        QVERIFY2(layer->value(quintptr(&handle), &value), "value() failed!");
        QVariantMap map = value.value<QVariantMap>();
        QVERIFY2( map.contains("identifier") &&
                 (map["identifier"] == "com.pstest.testValueLayer.app") &&
                  map.contains("settings") &&
                  (map["settings"].value<QVariantMap>()["setting1"] == "1"),
                 "value() failed!");

        JsonDbHandle handle2(NULL, "com/pstest/testValueLayer/system", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        QVERIFY2(layer->value(quintptr(&handle2), "setting2", &value), "value() failed!");
        intValue = value.value<int>();
        QVERIFY2(intValue == 2, "value() failed!");

        // Try to access the whole settings object
        QVERIFY2(layer->value(quintptr(&handle2), &value), "value() failed!");
        map = value.value<QVariantMap>();
        QVERIFY2( map.contains("identifier") &&
                 (map["identifier"] == "com.pstest.testValueLayer.system") &&
                  map.contains("settings") &&
                  (map["settings"].value<QVariantMap>()["setting2"] == "2"),
                 "value() failed!");

        QVERIFY2(!layer->value(quintptr(&handle), "testValueLayer", &value), "value() failed!");
    } catch(...) {}
}

void TestQValueSpaceJsonDb::testLayer_Instance()
{
    QVERIFY2(JsonDbLayer::instance() != NULL, "instance() failed!");
}


void TestQValueSpaceJsonDb::testPath_JsonDbPath()
{
    JsonDbPath path;
    QVERIFY2(path.getPath() == "", "Path is not an empty string!");
}

void TestQValueSpaceJsonDb::testPath_JsonDbPathString()
{
    JsonDbPath path1("/a/b/c");
    QVERIFY2(path1.getPath() == "a.b.c", "String constructor failed!");

    JsonDbPath path2("a/b/c");
    QVERIFY2(path2.getPath() == "a.b.c", "String constructor failed!");

    JsonDbPath path3("/a/b/c/");
    QVERIFY2(path3.getPath() == "a.b.c", "String constructor failed!");

    JsonDbPath path4("a/b/c/");
    QVERIFY2(path4.getPath() == "a.b.c", "String constructor failed!");

    JsonDbPath path5("a.b.c");
    QVERIFY2(path5.getPath() == "a.b.c", "String constructor failed!");
}

void TestQValueSpaceJsonDb::testPath_JsonDbPathOther()
{
    JsonDbPath path1("/a/b/c");
    JsonDbPath path11(path1);
    QVERIFY2(path11.getPath() == "a.b.c", "Path constructor failed!");

    JsonDbPath path2("a/b/c");
    JsonDbPath path22(path2);
    QVERIFY2(path22.getPath() == "a.b.c", "Path constructor failed!");

    JsonDbPath path3("/a/b/c/");
    JsonDbPath path33(path3);
    QVERIFY2(path33.getPath() == "a.b.c", "Path constructor failed!");

    JsonDbPath path4("a/b/c/");
    JsonDbPath path44(path4);
    QVERIFY2(path44.getPath() == "a.b.c", "Path constructor failed!");
}

void TestQValueSpaceJsonDb::testPath_GetPath()
{
    // Already tested in constructor test methods above
}

void TestQValueSpaceJsonDb::testPath_OperatorAssign()
{
    JsonDbPath path1;
    JsonDbPath path11 = path1;
    QVERIFY2(path11.getPath() == "", "Assignment operator failed!");

    JsonDbPath path2("/a/b/c");
    JsonDbPath path22 = path2;
    QVERIFY2(path22.getPath() == "a.b.c", "Assignment operator failed!");
}

void TestQValueSpaceJsonDb::testPath_OperatorEqual()
{
    JsonDbPath path1;
    JsonDbPath path11;
    QVERIFY2(path1 == path11, "Equal operator failed!");

    JsonDbPath path2("/a/b/c");
    JsonDbPath path22("/a/b/c");
    QVERIFY2(path2 == path22, "Equal operator failed!");

    JsonDbPath path3;
    JsonDbPath path33("/a/b/c");
    QVERIFY2(!(path3 == path33), "Equal operator failed!");

    JsonDbPath path4("/a/b");
    JsonDbPath path44("/a/b/c");
    QVERIFY2(!(path4 == path44), "Equal operator failed!");
}

void TestQValueSpaceJsonDb::testPath_OperatorPlusString()
{
    JsonDbPath path1;
    QVERIFY2((path1 + "").getPath() == "", "Plus string operator failed!");

    JsonDbPath path2;
    QVERIFY2((path2 + "/a/b/c").getPath() == "a.b.c", "Plus string operator failed!");

    JsonDbPath path3("/a/b/c");
    QVERIFY2((path3 + "").getPath() == "a.b.c", "Plus string operator failed!");

    JsonDbPath path4("/a/b/c");
    QVERIFY2((path4 + "d/e").getPath() == "a.b.c.d.e", "Plus string operator failed!");

    JsonDbPath path5("/a/b/c/");
    QVERIFY2((path5 + "d/e").getPath() == "a.b.c.d.e", "Plus string operator failed!");

    JsonDbPath path6("/a/b/c/");
    QVERIFY2((path6 + "/d/e").getPath() == "a.b.c.d.e", "Plus string operator failed!");
}

void TestQValueSpaceJsonDb::testPath_OperatorPlusPath()
{
    JsonDbPath path1;
    JsonDbPath path11;
    QVERIFY2((path1 + path11).getPath() == "", "Plus path operator failed!");

    JsonDbPath path2;
    JsonDbPath path22("/a/b/c");
    QVERIFY2((path2 + path22).getPath() == "a.b.c", "Plus path operator failed!");

    JsonDbPath path3("/a/b/c");
    JsonDbPath path33;
    QVERIFY2((path3 + path33).getPath() == "a.b.c", "Plus path operator failed!");

    JsonDbPath path4("/a/b/c");
    JsonDbPath path44("d/e");
    QVERIFY2((path4 + path44).getPath() == "a.b.c.d.e", "Plus path operator failed!");

    JsonDbPath path5("/a/b/c/");
    JsonDbPath path55("d/e");
    QVERIFY2((path5 + path55).getPath() == "a.b.c.d.e", "Plus path operator failed!");

    JsonDbPath path6("/a/b/c/");
    JsonDbPath path66("/d/e");
    QVERIFY2((path6 + path66).getPath() == "a.b.c.d.e", "Plus path operator failed!");
}

void TestQValueSpaceJsonDb::testPath_GetIdentifier()
{
    QString path = "com.nokia.mail.setting1";
    QStringList parts = JsonDbPath::getIdentifier(path);

    QVERIFY2(parts.count() == 2, "JsonDbPath::getIdentifier(path) failed!");
    QVERIFY2(parts[0] == "com.nokia.mail", "JsonDbPath::getIdentifier(path) failed!");
    QVERIFY2(parts[1] == "setting1", "JsonDbPath::getIdentifier(path) failed!");
}


void TestQValueSpaceJsonDb::testHandle_JsonDbHandle()
{
    JsonDbHandle handle(NULL, "", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QVERIFY2(handle.getPath() == "", "JsonDbHandle constructor failed!");

    JsonDbHandle handle1(NULL, "/testJsonDbHandle/b/c", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QVERIFY2(handle1.getPath() == "testJsonDbHandle.b.c", "JsonDbHandle constructor failed!");

    JsonDbHandle handle2(&handle1, "/d/e", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QVERIFY2(handle2.getPath() == "testJsonDbHandle.b.c.d.e", "JsonDbHandle constructor failed!");

    JsonDbHandle handle3(&handle1, "", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QVERIFY2(handle3.getPath() == "testJsonDbHandle.b.c", "JsonDbHandle constructor failed!");
}

void TestQValueSpaceJsonDb::testHandle_Value()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testValueLayer.app\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testValueLayer.system\", \"settings\": {\"setting2\":2}}";
    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com/pstest/testValueLayer/app", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        QVariant value;

        QVERIFY2(handle.value("setting1", &value), "value() failed!");
        int intValue = value.value<int>();
        QVERIFY2(intValue == 1, "value() failed!");

        // Try to access the whole settings object
        QVERIFY2(handle.value("", &value), "value() failed!");
        QVariantMap map = value.value<QVariantMap>();
        QVERIFY2( map.contains("identifier") &&
                 (map["identifier"] == "com.pstest.testValueLayer.app") &&
                  map.contains("settings") &&
                  (map["settings"].value<QVariantMap>()["setting1"] == "1"),
                 "value() failed!");

        JsonDbHandle handle2(NULL, "com/pstest/testValueLayer/system", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        QVERIFY2(handle2.value("setting2", &value), "value() failed!");
        intValue = value.value<int>();
        QVERIFY2(intValue == 2, "value() failed!");

        // Try to access the whole settings object
        QVERIFY2(handle2.value("", &value), "value() failed!");
        map = value.value<QVariantMap>();
        QVERIFY2( map.contains("identifier") &&
                 (map["identifier"] == "com.pstest.testValueLayer.system") &&
                  map.contains("settings") &&
                  (map["settings"].value<QVariantMap>()["setting2"] == "2"),
                 "value() failed!");

        QVERIFY2(!handle.value("testValueLayer", &value), "value() failed!");
    } catch(...) {}
}

void TestQValueSpaceJsonDb::testHandle_SetValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testSetValue.app\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testSetValue.system\", \"settings\": {\"setting2\":2}}";
    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com.pstest.testSetValue.app", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        QVERIFY2(handle.setValue("setting1", 42), "setValue() failed!");

        QVariant value;
        QVERIFY2(handle.value("setting1", &value), "value() failed!");
        QVERIFY2(value.value<int>() == 42, "setValue() failed!");

        JsonDbHandle handle2(NULL, "com.pstest.testSetValue.system", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        QVERIFY2(handle2.setValue("setting2", 42), "setValue() failed!");

        QVERIFY2(handle2.value("setting2", &value), "value() failed!");
        QVERIFY2(value.value<int>() == 42, "setValue() failed!");

        // Creating a new setting is not allowed
        QVERIFY2(!handle2.setValue("new_setting", 42), "setValue() failed!");

        // Changing a whole settings object is not allowed
        QVERIFY2(!handle2.setValue("", 42), "setValue() failed!");

        // Use handle with setting path
        JsonDbHandle handle3(NULL, "com.pstest.testSetValue.app.setting1", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        QVERIFY2(handle3.setValue("", 123), "setValue() failed!");
        QVERIFY2(handle3.value("", &value), "value() failed!");
        QVERIFY2(value.value<int>() == 123, "setValue() failed!");

        // Creating a new settings object is not allowed
        JsonDbHandle handle4(NULL, "com.pstest.testSetValue.system2", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        QVERIFY2(!handle4.setValue("", 42), "setValue() failed!");
    } catch(...) {}
}

void TestQValueSpaceJsonDb::testHandle_UnsetValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testUnsetValue.app\", \"setting1\":1}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testUnsetValue.system\", \"setting2\":2}";
    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com.pstest.testUnsetValue.app", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        // Settings may not be deleted
        QVERIFY2(!handle.unsetValue("setting1"), "unsetValue()");

        // Settings objects may not be deleted
        QVERIFY2(!handle.unsetValue(""), "unsetValue()");

        JsonDbHandle handle2(NULL, "com.pstest.testUnsetValue.system", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        // Settings may not be deleted
        QVERIFY2(!handle2.unsetValue("setting2"), "unsetValue()");

        // Settings objects may not be deleted
        QVERIFY2(!handle2.unsetValue(""), "unsetValue()");
    } catch(...) {}
}

void TestQValueSpaceJsonDb::testHandle_Subscribe()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testSubscribe.app\", \"setting1\":1}";

    // Subscribe for settings objects under com.testSubscribe
    JsonDbHandle handle(NULL, "com.pstest.testSubscribe", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    handle.subscribe();

    try {
        QSignalSpy spy(&handle, SIGNAL(valueChanged()));
        QVERIFY(spy.isValid());
        QCOMPARE(spy.count(), 0);

        createJsonObjects(objects);

        QTest::qWait(100);
        QCOMPARE(spy.count(), 1);
    } catch(...) {
        return;
    }
}

void TestQValueSpaceJsonDb::testHandle_Unsubscribe()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testUnsubscribe.app\", \"setting1\":1}";

    JsonDbHandle handle(NULL, "com.pstest.testUnsubscribe", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    QSignalSpy spy(&handle, SIGNAL(valueChanged()));
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), 0);

    handle.subscribe();

    QTest::qWait(100);

    handle.unsubscribe();

    createJsonObjects(objects);

    QTest::qWait(100);

    QCOMPARE(spy.count(), 0);
}

void TestQValueSpaceJsonDb::testHandle_Children()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testChildren.app1\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testChildren.sys1\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testChildren.sub.app2\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testChildren.sub.sys2\", \"settings\": {\"setting1\":1}}";

    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "com.pstest.testChildren", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
        QSet<QString> children = handle.children();
        QVERIFY2(children.count() == 3, "children() method failed!");
        QVERIFY2(children.contains("app1"), "children() method failed!");
        QVERIFY2(children.contains("sub"), "children() method failed!");
        QVERIFY2(children.contains("sys1"), "children() method failed!");
    }
    catch(...) { }
}

void TestQValueSpaceJsonDb::testHandle_RemoveSubTree()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testRemoveSubTree\", \"settings\": {\"setting1\":1}}";

    createJsonObjects(objects);

    try {
        JsonDbHandle handle(NULL, "/com.pstest.testRemoveSubTree", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

        // Deleting a subtree is not supported
        QVERIFY2(!handle.removeSubTree(), "removeSubTree() failed!");
    }
    catch(...) { }
}

void TestQValueSpaceJsonDb::testAPI_PublisherPath()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.PublisherPath\", \"settings\": {\"setting1\":1}}";
    createJsonObjects(objects);

    QString path = "/com/pstest/PublisherPath";
    QValueSpacePublisher publisher(path);
    QVERIFY(publisher.isConnected());
    QCOMPARE(publisher.path(), path);
}

void TestQValueSpaceJsonDb::testAPI_PublishSubscribe()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.PublishSubscribe\", \"settings\": {\"setting1\":1}}";
    createJsonObjects(objects);

    QString path = "/com/pstest/PublishSubscribe";
    QString name = "setting1";
    int value = 42;

    QValueSpaceSubscriber subscriber(QValueSpace::PermanentLayer | QValueSpace::WritableLayer, path);
    QVERIFY(subscriber.isConnected());
    QCOMPARE(subscriber.value(name).toInt(), 1);

    QValueSpacePublisher publisher(path);
    QVERIFY(publisher.isConnected());
    publisher.setValue(name, value);
    publisher.sync();


    QCOMPARE(subscriber.value(name).toInt(), value);

    subscriber.setPath(path + QStringLiteral("/") + name);
    QCOMPARE(subscriber.value().toInt(), value);
}

void TestQValueSpaceJsonDb::testAPI_Notification()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.Notification\", \"settings\": {\"setting1\":1}}";
    createJsonObjects(objects);

    QString path = "/com/pstest/Notification";
    QString name = "setting1";
    int value = 42;

    QValueSpaceSubscriber subscriber(QValueSpace::PermanentLayer | QValueSpace::WritableLayer, path);
    connect(&subscriber, SIGNAL(contentsChanged()), this, SLOT(contentsChanged()));
    QVERIFY(subscriber.isConnected());
    QCOMPARE(subscriber.value(name).toInt(), 1);

    QSignalSpy spy(&subscriber, SIGNAL(contentsChanged()));
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), 0);

    QValueSpacePublisher publisher(path);
    QVERIFY(publisher.isConnected());
    publisher.setValue(name, value);
    publisher.sync();

    QTest::qWait(100);
    QCOMPARE(subscriber.value(name).toInt(), value);
    QCOMPARE(spy.count(), 1);
}

void TestQValueSpaceJsonDb::contentsChanged()
{
    qDebug()<<"! BINGO !";
}

#include "tst_valuespace_jsondb.moc"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    TestQValueSpaceJsonDb test;

    return  QTest::qExec(&test, argc, argv);
}
