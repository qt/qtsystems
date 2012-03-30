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
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <QJsonDocument>
#include "qvaluespacepublisher.h"
#include "qvaluespacesubscriber.h"

#include <jsondblayer_p.h>

#include <QJsonDbConnection>
#include <QJsonDbReadRequest>
#include <QJsonDbRemoveRequest>
#include <QJsonDbCreateRequest>

#include <QJsonObject>
#include <QJsonArray>

#define WAIT_FOR 60

class JsonDbHandler: QObject
{
    Q_OBJECT

    public:
        JsonDbHandler();
        ~JsonDbHandler();

        void cleanupJsonDb();
        void createJsonObjects(const QStringList &objects);
        QVariantMap getObject(const QString &identifier);
        bool exists(QString query);

    private slots:
        void successSlot(int id);
        void errorSlot(QtJsonDb::QJsonDbRequest::ErrorCode id, QString code);

    private:
        QtJsonDb::QJsonDbConnection *mConnection;
        QList<QJsonObject> result;
        bool finished;

        void wait();
};

JsonDbHandler::JsonDbHandler()
{
    mConnection = new QtJsonDb::QJsonDbConnection;
    mConnection->connectToServer();

    finished = false;
}

JsonDbHandler::~JsonDbHandler()
{
    delete mConnection;
}

void JsonDbHandler::successSlot(int)
{
    result = ((QtJsonDb::QJsonDbReadRequest*)sender())->takeResults();

    finished = true;
}

void JsonDbHandler::errorSlot(QtJsonDb::QJsonDbRequest::ErrorCode, QString)
{
    result = QList<QJsonObject>();

    finished = true;
}

void JsonDbHandler::wait()
{
    while (!finished)
        QTest::qWait(1);

    finished = false;
}

void JsonDbHandler::cleanupJsonDb()
{
    QtJsonDb::QJsonDbReadRequest request;
    request.setQuery("[?identifier~=\"/com.pstest/\"]");

    connect(&request,
            SIGNAL(resultsAvailable(int)),
            this,
            SLOT(successSlot(int)));

    connect(&request,
            SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            this,
            SLOT(errorSlot(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));

    mConnection->send(&request);

    wait();

    QtJsonDb::QJsonDbRemoveRequest rmRequest(result);

    mConnection->send(&rmRequest);

    QTest::qWait(WAIT_FOR);
}

void JsonDbHandler::createJsonObjects(const QStringList &objectsStr)
{
    QList<QJsonObject> objects;

    foreach (QString object, objectsStr) {
        objects.append(QJsonDocument::fromJson(QByteArray(object.toStdString().c_str())).object());
    }

    QtJsonDb::QJsonDbCreateRequest request(objects);

    connect(&request, SIGNAL(resultsAvailable(int)), this, SLOT(successSlot(int)));

    connect(&request, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            this, SLOT(errorSlot(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));

    mConnection->send(&request);

    QTest::qWait(WAIT_FOR);
}

bool JsonDbHandler::exists(QString query)
{
    QtJsonDb::QJsonDbReadRequest* mRequest = new QtJsonDb::QJsonDbReadRequest;
    mRequest->setQuery(query);

    connect(mRequest, SIGNAL(resultsAvailable(int)), this, SLOT(successSlot(int)));

    connect(mRequest, SIGNAL(finished()), mRequest, SLOT(deleteLater()));

    connect(mRequest, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            this, SLOT(errorSlot(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));

    connect(mRequest, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            mRequest, SLOT(deleteLater()));

    mConnection->send(mRequest);

    wait();

    return result.count() > 0;
}

QVariantMap JsonDbHandler::getObject(const QString &identifier)
{
    QtJsonDb::QJsonDbReadRequest* mRequest = new QtJsonDb::QJsonDbReadRequest;
    mRequest->setQuery(QString("[?identifier=\"%1\"]").arg(identifier));

    connect(mRequest, SIGNAL(resultsAvailable(int)), this, SLOT(successSlot(int)));

    connect(mRequest, SIGNAL(finished()), mRequest, SLOT(deleteLater()));

    connect(mRequest, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            this, SLOT(errorSlot(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));

    connect(mRequest, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            mRequest, SLOT(deleteLater()));

    mConnection->send(mRequest);

    wait();

    if (result.count() == 0) {
        return QVariantMap();
    }

    return result.at(0).toVariantMap();
}



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
    void testAPI_NotificationSetting();
    void testAPI_NotificationUnique();
    void testAPI_ReadSystemSettingsObject();
    void testAPI_cd();
    void testAPI_MultipleWrites();
    void testAPI_MultipleWritesWithNotifications();

public slots:
    void contentsChanged();
    void onMultipleWritesWithNotifications();

private:
    JsonDbLayer *layer;
    bool changed;
    bool value;

    JsonDbHandler jsonDbHandler;
};

TestQValueSpaceJsonDb::TestQValueSpaceJsonDb()
{

}

void TestQValueSpaceJsonDb::init()
{
    layer = new JsonDbLayer();
}

void TestQValueSpaceJsonDb::cleanup()
{
    delete layer;

    jsonDbHandler.cleanupJsonDb();
}

void TestQValueSpaceJsonDb::testLayer_AddWatch()
{
    // addWatch() is currently not implemented
}

void TestQValueSpaceJsonDb::testLayer_RemoveWatches()
{
    // removeWatches() is currently not implemented
}

void TestQValueSpaceJsonDb::testLayer_Children()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testChildren.app1\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testChildren.sys1\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testChildren.sub.app2\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testChildren.sub.sys2\", \"settings\": {\"setting1\":1}}";

    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL, "com.pstest.testChildren", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QSet<QString> children = layer->children(quintptr(&handle));
    QVERIFY2(children.count() == 3, "children() method failed!");
    QVERIFY2(children.contains("app1"), "children() method failed!");
    QVERIFY2(children.contains("sub"), "children() method failed!");
    QVERIFY2(children.contains("sys1"), "children() method failed!");
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

    QSignalSpy spy(layer, SIGNAL(valueChanged()));
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), 0);

    jsonDbHandler.createJsonObjects(objects);

    QTest::qWait(WAIT_FOR);
    QCOMPARE(spy.count(), 1);

    layer->notifyInterest(quintptr(&handle), false);

    jsonDbHandler.createJsonObjects(objects);

    QTest::qWait(WAIT_FOR);
    QCOMPARE(spy.count(), 1);
}

void TestQValueSpaceJsonDb::testLayer_RemoveHandle()
{
    JsonDbHandle* handle = new JsonDbHandle(NULL, "", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QSignalSpy spy(handle, SIGNAL(destroyed()));
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), 0);

    layer->removeHandle(quintptr(handle));

    QTest::qWait(WAIT_FOR);
    QCOMPARE(spy.count(), 1);
}

void TestQValueSpaceJsonDb::testLayer_RemoveSubTree()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testRemoveSubTree\", \"settings\": {\"setting1\":1}}";

    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL, "/com.pstest.testRemoveSubTree", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    // Deleting a subtree is not supported
    QVERIFY2(!layer->removeSubTree(NULL, quintptr(&handle)), "removeSubTree() failed!");
}

void TestQValueSpaceJsonDb::testLayer_RemoveValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testUnsetValue.app\", \"setting1\":1}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testUnsetValue.system\", \"setting2\":2}";
    jsonDbHandler.createJsonObjects(objects);

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
}

void TestQValueSpaceJsonDb::testLayer_SetProperty()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testSetProperty.app\", \"setting1\":1}";

    // Subscribe for settings objects under com.testSubscribe
    JsonDbHandle handle(NULL, "com.pstest.testSetProperty", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    layer->setProperty(quintptr(&handle), QAbstractValueSpaceLayer::Publish);

    QSignalSpy spy(layer, SIGNAL(handleChanged(quintptr)));
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), 0);

    jsonDbHandler.createJsonObjects(objects);

    QTest::qWait(WAIT_FOR);
    QCOMPARE(spy.count(), 1);

    layer->setProperty(quintptr(&handle), (QAbstractValueSpaceLayer::Properties)0);

    jsonDbHandler.createJsonObjects(objects);

    QTest::qWait(WAIT_FOR);
    QCOMPARE(spy.count(), 1);
}

void TestQValueSpaceJsonDb::testLayer_SetValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testSetValue.app\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testSetValue.system\", \"settings\": {\"setting2\":2}}";
    jsonDbHandler.createJsonObjects(objects);

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
}

void TestQValueSpaceJsonDb::testLayer_SupportsInterestNotification()
{
    QVERIFY2(layer->supportsInterestNotification(), "supportsInterestNotification() failed!");
}

void TestQValueSpaceJsonDb::testLayer_Sync()
{

}

void TestQValueSpaceJsonDb::testLayer_Value()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testValueLayer.app\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testValueLayer.system\", \"settings\": {\"setting2\":2}}";
    jsonDbHandler.createJsonObjects(objects);

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
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testValueHandle.app\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testValueHandle.system\", \"settings\": {\"setting2\":2}}";
    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL, "com/pstest/testValueHandle/app", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QVariant value;

    QVERIFY2(handle.value("setting1", &value), "value() failed!");
    int intValue = value.value<int>();
    QVERIFY2(intValue == 1, "value() failed!");

    // Try to access the whole settings object
    QVERIFY2(handle.value("", &value), "value() failed!");
    QVariantMap map = value.value<QVariantMap>();
    QVERIFY2( map.contains("identifier") &&
             (map["identifier"] == "com.pstest.testValueHandle.app") &&
              map.contains("settings") &&
              (map["settings"].value<QVariantMap>()["setting1"] == "1"),
             "value() failed!");

    JsonDbHandle handle2(NULL, "com/pstest/testValueHandle/system", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    QVERIFY2(handle2.value("setting2", &value), "value() failed!");
    intValue = value.value<int>();
    QVERIFY2(intValue == 2, "value() failed!");

    // Try to access the whole settings object
    QVERIFY2(handle2.value("", &value), "value() failed!");
    map = value.value<QVariantMap>();
    QVERIFY2( map.contains("identifier") &&
             (map["identifier"] == "com.pstest.testValueHandle.system") &&
              map.contains("settings") &&
              (map["settings"].value<QVariantMap>()["setting2"] == "2"),
             "value() failed!");

    QVERIFY2(!handle.value("testValueLayer", &value), "value() failed!");
}

void TestQValueSpaceJsonDb::testHandle_SetValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testSetValue.app\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testSetValue.system\", \"settings\": {\"setting2\":2}}";
    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL, "com.pstest.testSetValue.app", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    QVERIFY2(handle.setValue("setting1", 42), "setValue() failed!");
    QTest::qWait(WAIT_FOR);

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
}

void TestQValueSpaceJsonDb::testHandle_UnsetValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testUnsetValue.app\", \"setting1\":1}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testUnsetValue.system\", \"setting2\":2}";
    jsonDbHandler.createJsonObjects(objects);

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
}

void TestQValueSpaceJsonDb::testHandle_Subscribe()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testSubscribe.app\", \"setting1\":1}";

    // Subscribe for settings objects under com.testSubscribe
    JsonDbHandle handle(NULL, "/com/pstest/testSubscribe/app", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    handle.subscribe();

    QSignalSpy spy(&handle, SIGNAL(valueChanged()));
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), 0);

    jsonDbHandler.createJsonObjects(objects);

    QTest::qWait(WAIT_FOR);
    QCOMPARE(spy.count(), 1);
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

    QTest::qWait(WAIT_FOR);

    handle.unsubscribe();

    jsonDbHandler.createJsonObjects(objects);

    QTest::qWait(WAIT_FOR);

    QCOMPARE(spy.count(), 0);
}

void TestQValueSpaceJsonDb::testHandle_Children()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testChildren.app1\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testChildren.sys1\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testChildren.sub.app2\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testChildren.sub.sys2\", \"settings\": {\"setting1\":1}}";

    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL, "com.pstest.testChildren", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QSet<QString> children = handle.children();
    QVERIFY2(children.count() == 3, "children() method failed!");
    QVERIFY2(children.contains("app1"), "children() method failed!");
    QVERIFY2(children.contains("sub"), "children() method failed!");
    QVERIFY2(children.contains("sys1"), "children() method failed!");
}

void TestQValueSpaceJsonDb::testHandle_RemoveSubTree()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.testRemoveSubTree\", \"settings\": {\"setting1\":1}}";

    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL, "/com.pstest.testRemoveSubTree", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    // Deleting a subtree is not supported
    QVERIFY2(!handle.removeSubTree(), "removeSubTree() failed!");
}

void TestQValueSpaceJsonDb::testAPI_PublisherPath()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.PublisherPath\", \"settings\": {\"setting1\":1}}";
    jsonDbHandler.createJsonObjects(objects);

    QString path = "/com/pstest/PublisherPath";
    QValueSpacePublisher publisher(path);
    QVERIFY(publisher.isConnected());
    QCOMPARE(publisher.path(), path);
}

void TestQValueSpaceJsonDb::testAPI_PublishSubscribe()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.PublishSubscribe.first\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.PublishSubscribe.second\", \"settings\": {\"setting2\":2}}";
    jsonDbHandler.createJsonObjects(objects);

    QString path = "/com/pstest/PublishSubscribe/first";
    QString name = "setting1";
    int value = 42;

    QValueSpaceSubscriber subscriber(QValueSpace::PermanentLayer | QValueSpace::WritableLayer, path);
    QVERIFY(subscriber.isConnected());
    QCOMPARE(subscriber.value(name).toInt(), 1);

    QValueSpacePublisher publisher(path);
    QVERIFY(publisher.isConnected());
    publisher.setValue(name, value);
    publisher.sync();

    QTest::qWait(WAIT_FOR);

    QCOMPARE(subscriber.value(name).toInt(), value);

    subscriber.setPath(path + QStringLiteral("/") + name);
    QCOMPARE(subscriber.value().toInt(), value);

    // Try to read the whole application settings object
    subscriber.setPath(path);
    QVariant wholeObject = subscriber.value();
    QVERIFY(wholeObject.isValid());

    QVariantMap objectMap = wholeObject.toMap();
    QVERIFY(!objectMap.isEmpty());
    QVERIFY(objectMap.contains(QStringLiteral("settings")));
    QVERIFY(objectMap[QStringLiteral("settings")].toMap().contains(QStringLiteral("setting1")));
    QVERIFY(objectMap[QStringLiteral("settings")].toMap()[QStringLiteral("setting1")].toInt() == 42);

    // Try to read the whole system settings object
    subscriber.setPath("/com/pstest/PublishSubscribe/second");
    QVERIFY(subscriber.path() == "/com/pstest/PublishSubscribe/second");
    wholeObject = subscriber.value();
    QVERIFY(wholeObject.isValid());

    objectMap = wholeObject.toMap();
    QVERIFY(!objectMap.isEmpty());
    QVERIFY(objectMap.contains(QStringLiteral("settings")));
    QVERIFY(objectMap[QStringLiteral("settings")].toMap().contains(QStringLiteral("setting2")));
    QVERIFY(objectMap[QStringLiteral("settings")].toMap()[QStringLiteral("setting2")].toInt() == 2);
}

void TestQValueSpaceJsonDb::testAPI_Notification()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.Notification\", \"settings\": {\"setting1\":1}}";
    jsonDbHandler.createJsonObjects(objects);

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

    QTest::qWait(WAIT_FOR);
    QCOMPARE(subscriber.value(name).toInt(), value);
    QCOMPARE(spy.count(), 1);
}

void TestQValueSpaceJsonDb::testAPI_NotificationSetting()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.pstest.Notification\", \"settings\": {\"setting1\":1, \"setting2\":2}}";
    jsonDbHandler.createJsonObjects(objects);

    int value = 42;

    QValueSpaceSubscriber subscriber(QValueSpace::PermanentLayer | QValueSpace::WritableLayer,
                                     "/com/pstest/Notification/setting2");
    connect(&subscriber, SIGNAL(contentsChanged()), this, SLOT(contentsChanged()));
    QVERIFY(subscriber.isConnected());
    QCOMPARE(subscriber.value().toInt(), 2);

    QSignalSpy spy(&subscriber, SIGNAL(contentsChanged()));
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), 0);

    QValueSpacePublisher publisher("/com/pstest/Notification/setting1");
    QVERIFY(publisher.isConnected());
    publisher.setValue("", value);
    publisher.sync();

    QTest::qWait(WAIT_FOR);
    QCOMPARE(spy.count(), 1);
}

void TestQValueSpaceJsonDb::testAPI_NotificationUnique()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.Notification1\", \"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.Notification2\", \"settings\": {\"setting2\":2}}";
    jsonDbHandler.createJsonObjects(objects);

    int value = 42;

    QValueSpaceSubscriber subscriber1(QValueSpace::PermanentLayer | QValueSpace::WritableLayer, "/com/pstest/Notification1");
    connect(&subscriber1, SIGNAL(contentsChanged()), this, SLOT(contentsChanged()));
    QVERIFY(subscriber1.isConnected());
    QCOMPARE(subscriber1.value("setting1").toInt(), 1);

    QValueSpaceSubscriber subscriber2(QValueSpace::PermanentLayer | QValueSpace::WritableLayer, "/com/pstest/Notification2");
    connect(&subscriber2, SIGNAL(contentsChanged()), this, SLOT(contentsChanged()));
    QVERIFY(subscriber2.isConnected());
    QCOMPARE(subscriber2.value("setting2").toInt(), 2);

    QSignalSpy spy1(&subscriber1, SIGNAL(contentsChanged()));
    QVERIFY(spy1.isValid());
    QCOMPARE(spy1.count(), 0);

    QSignalSpy spy2(&subscriber2, SIGNAL(contentsChanged()));
    QVERIFY(spy2.isValid());
    QCOMPARE(spy2.count(), 0);

    QValueSpacePublisher publisher("/com/pstest/Notification1");
    QVERIFY(publisher.isConnected());
    publisher.setValue("setting1", value);
    publisher.sync();

    QTest::qWait(WAIT_FOR);
    QCOMPARE(subscriber1.value("setting1").toInt(), value);
    QCOMPARE(spy1.count(), 1);

    QCOMPARE(spy2.count(), 0);
}

void TestQValueSpaceJsonDb::testAPI_ReadSystemSettingsObject()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.ApplicationSettings\", \"identifier\":\"com.nokia.mt.settings\"}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.nokia.mt.settings.ReadSystemSettingsObject\", \"settings\": {\"setting1\":1}}";
    jsonDbHandler.createJsonObjects(objects);

    QValueSpaceSubscriber subscriber(QValueSpace::PermanentLayer | QValueSpace::WritableLayer,
                                     "com.nokia.mt.settings.ReadSystemSettingsObject");
    QVERIFY(subscriber.isConnected());
    QVariant settingsObject = subscriber.value();
    QVERIFY(settingsObject.isValid());

    QVariantMap map = settingsObject.toMap();
    QVERIFY(map.contains("settings"));
    QVERIFY(map.value("settings").toMap().contains("setting1"));
    QVERIFY(map.value("settings").toMap().value("setting1").toInt() == 1);
}

void TestQValueSpaceJsonDb::testAPI_cd()
{
    QValueSpaceSubscriber subscriber(QValueSpace::PermanentLayer | QValueSpace::WritableLayer,
                                     "/com/pstest/cd");
    QVERIFY(subscriber.path() == "/com/pstest/cd");

    subscriber.cd("/com/pstest/cd/sub");
    QVERIFY(subscriber.path() == "/com/pstest/cd/sub");
}

void TestQValueSpaceJsonDb::testAPI_MultipleWrites()
{
    QStringList objects;
    objects<<QString("{\"_type\":\"%1\", \"identifier\":\"%2\", \"settings\":{%3}}")
             .arg("com.nokia.mt.settings.SystemSettings")
             .arg("com.pstest.testAPI_MultipleWrites")
             .arg("\"setting1\":0, \"setting2\":true, \"setting3\":\"\"");
    jsonDbHandler.createJsonObjects(objects);

    QValueSpaceSubscriber subscriber(QValueSpace::PermanentLayer | QValueSpace::WritableLayer,
                                     "/com/pstest/testAPI_MultipleWrites");
    QVERIFY(subscriber.isConnected());

    QValueSpacePublisher publisher("/com/pstest/testAPI_MultipleWrites");
    QVERIFY(publisher.isConnected());

    for (int i = 0; i < 10; i++) {
        qDebug()<<"LOOP"<<i + 1;
        publisher.setValue("setting1", i + 1);
        publisher.setValue("setting2", (i % 2) == 1);
        publisher.setValue("setting3", QString::number(i));

        publisher.sync();
        QTest::qWait(WAIT_FOR * 2);

        QVERIFY(subscriber.value("setting1").toInt() == i + 1);
        QVERIFY(subscriber.value("setting2").toBool() == ((i % 2) == 1));
        QVERIFY(subscriber.value("setting3").toString() == QString::number(i));
    }
}

void TestQValueSpaceJsonDb::testAPI_MultipleWritesWithNotifications()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", \"identifier\":\"com.pstest.testAPI_MultipleWritesWithNotifications\", \"settings\": {\"setting1\":false}}";
    jsonDbHandler.createJsonObjects(objects);

    QValueSpaceSubscriber subscriber(QValueSpace::PermanentLayer | QValueSpace::WritableLayer,
                                     "/com/pstest/testAPI_MultipleWritesWithNotifications");
    QVERIFY(subscriber.isConnected());

    connect(&subscriber, SIGNAL(contentsChanged()), this, SLOT(onMultipleWritesWithNotifications()));

    QValueSpacePublisher publisher("/com/pstest/testAPI_MultipleWritesWithNotifications");
    QVERIFY(publisher.isConnected());

    bool nextValue = true;

    for (int i = 0; i < 50; i++) {
        qDebug()<<"LOOP"<<i + 1;

        changed = false;

        publisher.setValue("setting1", nextValue);

        nextValue = !nextValue;

        while (!changed) {
            QTest::qWait(1);
        }

        QVERIFY(value == !nextValue);
    }
}

void TestQValueSpaceJsonDb::onMultipleWritesWithNotifications()
{
    changed = true;
    static QValueSpaceSubscriber subscriber(QValueSpace::PermanentLayer | QValueSpace::WritableLayer,
                                            "/com/pstest/testAPI_MultipleWritesWithNotifications");

    value = subscriber.value("setting1").toBool();
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
