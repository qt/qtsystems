/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qdebug.h>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <QJsonDocument>
#include <QtConcurrent/QFuture>
#include <QtConcurrent/QtConcurrentRun>
#include "qvaluespacepublisher.h"
#include "qvaluespacesubscriber.h"

#include <jsondblayer_p.h>

#include <QJsonDbConnection>
#include <QJsonDbReadRequest>
#include <QJsonDbRemoveRequest>
#include <QJsonDbCreateRequest>

#include <QJsonObject>
#include <QJsonArray>

Q_DECLARE_METATYPE(QtJsonDb::QJsonDbRequest::ErrorCode)

const int WAIT_FOR = 60;

static const QString getSystemPartition() { return QStringLiteral("com.nokia.mt.Settings"); }

class JsonDbHandler: QObject
{
    Q_OBJECT

    public:
        JsonDbHandler();
        ~JsonDbHandler();

        void cleanupJsonDb();
        void createJsonObjects(const QStringList &objects);

    private slots:
        void successSlot(int id);
        void errorSlot(QtJsonDb::QJsonDbRequest::ErrorCode id, QString code);

    private:
        QtJsonDb::QJsonDbConnection *mConnection;
        QList<QJsonObject> result;
        bool finished;

        void wait(QtJsonDb::QJsonDbRequest &);
};

JsonDbHandler::JsonDbHandler()
{
    qRegisterMetaType<QtJsonDb::QJsonDbRequest::ErrorCode>();

    mConnection = new QtJsonDb::QJsonDbConnection;
    mConnection->connectToServer();
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

void JsonDbHandler::errorSlot(QtJsonDb::QJsonDbRequest::ErrorCode, QString errorMessage)
{
    qDebug()<<"ERROR:"<<errorMessage;

    result = QList<QJsonObject>();

    finished = true;
}

void JsonDbHandler::wait(QtJsonDb::QJsonDbRequest &request)
{
    connect(&request,
            SIGNAL(resultsAvailable(int)),
            this,
            SLOT(successSlot(int)));

    connect(&request,
            SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)),
            this,
            SLOT(errorSlot(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));

    mConnection->send(&request);

    finished = false;

    for (int i = 0; i < 1000; i++) {
        if (finished)
            break;

        QTest::qWait(1);
    }
}

void JsonDbHandler::cleanupJsonDb()
{
    QtJsonDb::QJsonDbReadRequest request;
    request.setQuery("[?identifier startsWith \"com.nokia.mt.settings.pstest\"]");
    request.setPartition(getSystemPartition());

    wait(request);

    if (result.count() > 0) {
        QtJsonDb::QJsonDbRemoveRequest rmRequest(result);
        rmRequest.setPartition(getSystemPartition());

        wait(rmRequest);
    }
}

void JsonDbHandler::createJsonObjects(const QStringList &objectsStr)
{
    QList<QJsonObject> objects;

    foreach (QString object, objectsStr) {
        objects.append(QJsonDocument::fromJson(QByteArray(object.toStdString().c_str())).object());
    }

    QtJsonDb::QJsonDbCreateRequest request(objects);
    request.setPartition(getSystemPartition());

    wait(request);
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
    void testHandle_SetValueConcurrent();
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

    int concurrentSetValue(const QString &path, const QString &name, int loops);
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
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testChildren.sys1\", "
             "\"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testChildren.sub.sys2\", "
             "\"settings\": {\"setting1\":1}}";

    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL,
                        "com.nokia.mt.settings.pstest.testChildren",
                        QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    QSet<QString> children = layer->children(quintptr(&handle));
    QVERIFY2(children.count() == 2, "children() method failed!");
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
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testNotifyInterest.system\", "
             "\"setting1\":1}";

    // Subscribe for settings objects under com.testNotifyInterest
    JsonDbHandle handle(NULL,
                        "com.nokia.mt.settings.pstest.testNotifyInterest",
                        QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    layer->notifyInterest(quintptr(&handle), true);

    QSignalSpy spy(layer, SIGNAL(valueChanged()));
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), 0);

    jsonDbHandler.createJsonObjects(objects);

    QCOMPARE(spy.count(), 1);

    layer->notifyInterest(quintptr(&handle), false);

    jsonDbHandler.createJsonObjects(objects);

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
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testRemoveSubTree\", "
             "\"settings\": {\"setting1\":1}}";

    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL,
                        "com.nokia.mt.settings.pstest.testRemoveSubTree",
                        QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    // Deleting a subtree is not supported
    QVERIFY2(!layer->removeSubTree(NULL, quintptr(&handle)), "removeSubTree() failed!");
}

void TestQValueSpaceJsonDb::testLayer_RemoveValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testUnsetValue.system\", "
             "\"setting1\":1}";
    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL,
                        "com.nokia.mt.settings.pstest.testUnsetValue.system",
                        QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    // Settings may not be deleted
    QVERIFY2(!layer->removeValue(NULL, quintptr(&handle), "setting1"), "removeValue() failed!");//handle.unsetValue("setting1"), "unsetValue()");

    // Settings objects may not be deleted
    QVERIFY2(!layer->removeValue(NULL, quintptr(&handle), ""), "removeValue() failed!");
}

void TestQValueSpaceJsonDb::testLayer_SetProperty()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testSetProperty.system\", "
             "\"setting1\":1}";

    // Subscribe for settings objects under com.testSubscribe
    JsonDbHandle handle(NULL,
                        "com.nokia.mt.settings.pstest.testSetProperty",
                        QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    layer->setProperty(quintptr(&handle), QAbstractValueSpaceLayer::Publish);

    QSignalSpy spy(layer, SIGNAL(handleChanged(quintptr)));
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), 0);

    jsonDbHandler.createJsonObjects(objects);

    QCOMPARE(spy.count(), 1);

    layer->setProperty(quintptr(&handle), (QAbstractValueSpaceLayer::Properties)0);

    jsonDbHandler.createJsonObjects(objects);

    QCOMPARE(spy.count(), 1);
}

void TestQValueSpaceJsonDb::testLayer_SetValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testSetValue.system\", "
             "\"settings\": {\"setting1\":1}}";
    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL,
                        "com.nokia.mt.settings.pstest.testSetValue.system",
                        QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    QVERIFY2(layer->setValue(NULL, quintptr(&handle), "setting1", 42), "setValue() failed!");

    QVariant value;
    QVERIFY2(handle.value("setting1", &value), "value() failed!");
    QVERIFY2(value.value<int>() == 42, "setValue() failed!");

    //JsonDbHandle handle3(NULL, "com.pstest.testSetValue.system", QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    QVERIFY2(layer->setValue(NULL, quintptr(&handle), "setting1", 42), "setValue() failed!");

    QVERIFY2(handle.value("setting1", &value), "value() failed!");
    QVERIFY2(value.value<int>() == 42, "setValue() failed!");

    // Creating a new setting is not allowed
    QVERIFY2(!layer->setValue(NULL, quintptr(&handle), "new_setting", 42), "setValue() failed!");

    // Changing a whole settings object is not allowed
    QVERIFY2(!layer->setValue(NULL, quintptr(&handle), "", 42), "setValue() failed!");

    // Creating a new settings object is not allowed
    JsonDbHandle handle2(NULL,
                         "com.nokia.mt.settings.pstest.testSetValue.system2",
                         QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    QVERIFY2(!layer->setValue(NULL, quintptr(&handle2), "", 42), "setValue() failed!");
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
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testValueLayer.system\", "
             "\"settings\": {\"setting1\":1}}";
    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL,
                        "/com/nokia/mt/settings/pstest/testValueLayer/system",
                        QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QVariant value;

    QVERIFY2(layer->value(quintptr(&handle), "setting1", &value), "value() failed!");
    int intValue = value.value<int>();
    QVERIFY2(intValue == 1, "value() failed!");

    // Try to access the whole settings object
    QVERIFY2(layer->value(quintptr(&handle), &value), "value() failed!");
    QVariantMap map = value.value<QVariantMap>();
    QVERIFY2( map.contains("identifier") &&
             (map["identifier"] == "com.nokia.mt.settings.pstest.testValueLayer.system") &&
              map.contains("settings") &&
              (map["settings"].value<QVariantMap>()["setting1"] == "1"),
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
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testValueHandle.system\", "
             "\"settings\": {\"setting1\":1}}";
    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL,
                        "/com/nokia/mt/settings/pstest/testValueHandle/system",
                        QValueSpace::PermanentLayer | QValueSpace::WritableLayer);
    QVariant value;

    QVERIFY2(handle.value("setting1", &value), "value() failed!");
    int intValue = value.value<int>();
    QVERIFY2(intValue == 1, "value() failed!");

    // Try to access the whole settings object
    QVERIFY2(handle.value("", &value), "value() failed!");
    QVariantMap map = value.value<QVariantMap>();
    QVERIFY2( map.contains("identifier") &&
             (map["identifier"] == "com.nokia.mt.settings.pstest.testValueHandle.system") &&
              map.contains("settings") &&
              (map["settings"].value<QVariantMap>()["setting1"] == "1"),
             "value() failed!");

    QVERIFY2(!handle.value("testValueLayer", &value), "value() failed!");
}

void TestQValueSpaceJsonDb::testHandle_SetValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testSetValue.system\", "
             "\"settings\": {\"setting1\":1}}";
    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL,
                        "com.nokia.mt.settings.pstest.testSetValue.system",
                        QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    QVERIFY2(handle.setValue("setting1", 42), "setValue() failed!");
    QTest::qWait(WAIT_FOR);

    QVariant value;
    QVERIFY2(handle.value("setting1", &value), "value() failed!");

    QVERIFY2(value.value<int>() == 42, "setValue() failed!");

    // Use handle with setting path
    JsonDbHandle handle2(NULL,
                         "com.nokia.mt.settings.pstest.testSetValue.system.setting1",
                         QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    QVERIFY2(handle2.setValue("", 123), "setValue() failed!");
    QVERIFY2(handle2.value("", &value), "value() failed!");
    QVERIFY2(value.value<int>() == 123, "setValue() failed!");

    // Creating a new settings object is not allowed
    JsonDbHandle handle3(NULL,
                         "com.nokia.mt.settings.pstest.testSetValue.system2",
                         QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    QVERIFY2(!handle3.setValue("", 42), "setValue() failed!");
}

void TestQValueSpaceJsonDb::testHandle_SetValueConcurrent()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testSetValueConcurrent\", "
             "\"settings\": {\"setting1\":0, \"setting2\":0}}";
    jsonDbHandler.createJsonObjects(objects);

    int loops = 100;
    QString path = "/com/nokia/mt/settings/pstest/testSetValueConcurrent";

    QFuture<int> f1 = QtConcurrent::run(this,
                                        &TestQValueSpaceJsonDb::concurrentSetValue,
                                        path,
                                        QString("setting1"),
                                        loops);

    QFuture<int> f2 = QtConcurrent::run(this,
                                        &TestQValueSpaceJsonDb::concurrentSetValue,
                                        path,
                                        QString("setting2"),
                                        loops);

    f1.waitForFinished();
    f2.waitForFinished();

    // TODO
    QSKIP("Test disabled until JSON DB activates stale updates by default!");
    QVERIFY2(f1.result() == loops, QString::number(f1.result()).toStdString().c_str());
    QVERIFY2(f2.result() == loops, QString::number(f2.result()).toStdString().c_str());
}

int TestQValueSpaceJsonDb::concurrentSetValue(const QString &path, const QString &name, int loops)
{
    JsonDbHandle handle(NULL, path, QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    for (int i = 0; i < loops; i++) {
        while (!handle.setValue(name, i + 1)) {
            qDebug()<<"handle.setValue() failed";
        }
    }

    QTest::qWait(WAIT_FOR);

    QVariant var;
    if (!handle.value(name, &var)) {
        return -1;
    }

    return var.toInt();
}

void TestQValueSpaceJsonDb::testHandle_UnsetValue()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testUnsetValue.system\", "
             "\"setting1\":1}";
    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL,
                        "com.nokia.mt.settings.pstest.testUnsetValue.system",
                        QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    // Settings may not be deleted
    QVERIFY2(!handle.unsetValue("setting1"), "unsetValue()");

    // Settings objects may not be deleted
    QVERIFY2(!handle.unsetValue(""), "unsetValue()");
}

void TestQValueSpaceJsonDb::testHandle_Subscribe()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testSubscribe.system\", "
             "\"setting1\":1}";

    // Subscribe for settings objects under com.testSubscribe
    JsonDbHandle handle(NULL,
                        "/com/nokia/mt/settings/pstest/testSubscribe/system",
                        QValueSpace::PermanentLayer | QValueSpace::WritableLayer);


    connect(&handle,
            SIGNAL(valueChanged()),
            this,
            SLOT(contentsChanged()));

    handle.subscribe();

    changed = false;

    jsonDbHandler.createJsonObjects(objects);

    for (int i = 0; i < 1000; i++) {
        if (changed)
            break;

        QTest::qWait(1);
        QCoreApplication::processEvents();
    }

    QVERIFY(changed);
}

void TestQValueSpaceJsonDb::testHandle_Unsubscribe()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testUnsubscribe.system\", "
             "\"setting1\":1}";

    JsonDbHandle handle(NULL,
                        "com.nokia.mt.settings.pstest.testUnsubscribe",
                        QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    QSignalSpy spy(&handle, SIGNAL(valueChanged()));
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), 0);

    handle.subscribe();

    QTest::qWait(WAIT_FOR);

    handle.unsubscribe();

    jsonDbHandler.createJsonObjects(objects);

    QCOMPARE(spy.count(), 0);
}

void TestQValueSpaceJsonDb::testHandle_Children()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testChildren.sys1\", "
             "\"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testChildren.sub.sys2\", "
             "\"settings\": {\"setting1\":1}}";

    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL,
                        "com.nokia.mt.settings.pstest.testChildren",
                        QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    QSet<QString> children = handle.children();
    QVERIFY2(children.count() == 2, "children() method failed!");
    QVERIFY2(children.contains("sub"), "children() method failed!");
    QVERIFY2(children.contains("sys1"), "children() method failed!");
}

void TestQValueSpaceJsonDb::testHandle_RemoveSubTree()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testRemoveSubTree\", "
             "\"settings\": {\"setting1\":1}}";

    jsonDbHandler.createJsonObjects(objects);

    JsonDbHandle handle(NULL,
                        "com.nokia.mt.settings.pstest.testRemoveSubTree",
                        QValueSpace::PermanentLayer | QValueSpace::WritableLayer);

    // Deleting a subtree is not supported
    QVERIFY2(!handle.removeSubTree(), "removeSubTree() failed!");
}

void TestQValueSpaceJsonDb::testAPI_PublisherPath()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.PublisherPath\", "
             "\"settings\": {\"setting1\":1}}";
    jsonDbHandler.createJsonObjects(objects);

    QString path = "/com/nokia/mt/settings/pstest/PublisherPath";
    QValueSpacePublisher publisher(path);
    QVERIFY(publisher.isConnected());
    QCOMPARE(publisher.path(), path);
}

void TestQValueSpaceJsonDb::testAPI_PublishSubscribe()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.PublishSubscribe.first\", "
             "\"settings\": {\"setting1\":1}}";
    jsonDbHandler.createJsonObjects(objects);

    QString path = "/com/nokia/mt/settings/pstest/PublishSubscribe/first";
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

    // Try to read the whole system settings object
    subscriber.setPath(path);
    QVariant wholeObject = subscriber.value();
    QVERIFY(wholeObject.isValid());

    QVariantMap objectMap = wholeObject.toMap();
    QVERIFY(!objectMap.isEmpty());
    QVERIFY(objectMap.contains(QStringLiteral("settings")));
    QVERIFY(objectMap[QStringLiteral("settings")].toMap().contains(QStringLiteral("setting1")));
    QVERIFY(objectMap[QStringLiteral("settings")].toMap()[QStringLiteral("setting1")].toInt() == 42);
}

void TestQValueSpaceJsonDb::testAPI_Notification()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.Notification\", "
             "\"settings\": {\"setting1\":1}}";


    QString path = "/com/nokia/mt/settings/pstest/Notification";

    QValueSpaceSubscriber subscriber(QValueSpace::PermanentLayer | QValueSpace::WritableLayer, path);
    connect(&subscriber, SIGNAL(contentsChanged()), this, SLOT(contentsChanged()));
    QVERIFY(subscriber.isConnected());

    changed = false;

    jsonDbHandler.createJsonObjects(objects);

    for (int i = 0; i < 1000; i++) {
        if (changed)
            break;

        QTest::qWait(1);
        QCoreApplication::processEvents();
    }

    QVERIFY(changed);
}

void TestQValueSpaceJsonDb::testAPI_NotificationSetting()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.Notification\", "
             "\"settings\": {\"setting1\":1, \"setting2\":2}}";
    jsonDbHandler.createJsonObjects(objects);

    int value = 42;

    QValueSpaceSubscriber subscriber(QValueSpace::PermanentLayer | QValueSpace::WritableLayer,
                                     "/com/nokia/mt/settings/pstest/Notification/setting2");

    connect(&subscriber, SIGNAL(contentsChanged()), this, SLOT(contentsChanged()));
    QVERIFY(subscriber.isConnected());
    QCOMPARE(subscriber.value().toInt(), 2);

    QSignalSpy spy(&subscriber, SIGNAL(contentsChanged()));
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), 0);

    QValueSpacePublisher publisher("/com/nokia/mt/settings/pstest/Notification/setting1");
    QVERIFY(publisher.isConnected());
    publisher.setValue("", value);
    publisher.sync();

    QTest::qWait(WAIT_FOR);
    QCOMPARE(spy.count(), 1);
}

void TestQValueSpaceJsonDb::testAPI_NotificationUnique()
{
    QStringList objects;
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.Notification1\", "
             "\"settings\": {\"setting1\":1}}";
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.Notification2\", "
             "\"settings\": {\"setting2\":2}}";
    jsonDbHandler.createJsonObjects(objects);

    int value = 42;

    QValueSpaceSubscriber subscriber1(QValueSpace::PermanentLayer | QValueSpace::WritableLayer,
                                      "/com/nokia/mt/settings/pstest/Notification1");

    connect(&subscriber1, SIGNAL(contentsChanged()), this, SLOT(contentsChanged()));
    QVERIFY(subscriber1.isConnected());
    QCOMPARE(subscriber1.value("setting1").toInt(), 1);

    QValueSpaceSubscriber subscriber2(QValueSpace::PermanentLayer | QValueSpace::WritableLayer,
                                      "/com/nokia/mt/settings/pstest/Notification2");

    connect(&subscriber2, SIGNAL(contentsChanged()), this, SLOT(contentsChanged()));
    QVERIFY(subscriber2.isConnected());
    QCOMPARE(subscriber2.value("setting2").toInt(), 2);

    QSignalSpy spy1(&subscriber1, SIGNAL(contentsChanged()));
    QVERIFY(spy1.isValid());
    QCOMPARE(spy1.count(), 0);

    QSignalSpy spy2(&subscriber2, SIGNAL(contentsChanged()));
    QVERIFY(spy2.isValid());
    QCOMPARE(spy2.count(), 0);

    QValueSpacePublisher publisher("/com/nokia/mt/settings/pstest/Notification1");
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
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.ReadSystemSettingsObject\", "
             "\"settings\": {\"setting1\":1}}";
    jsonDbHandler.createJsonObjects(objects);

    QValueSpaceSubscriber subscriber(QValueSpace::PermanentLayer | QValueSpace::WritableLayer,
                                     "com.nokia.mt.settings.pstest.ReadSystemSettingsObject");

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
                                     "/com/nokia/mt/settings/pstest/cd");
    QVERIFY(subscriber.path() == "/com/nokia/mt/settings/pstest/cd");

    subscriber.cd("/com/nokia/mt/settings/pstest/cd/sub");
    QVERIFY(subscriber.path() == "/com/nokia/mt/settings/pstest/cd/sub");
}

void TestQValueSpaceJsonDb::testAPI_MultipleWrites()
{
    QStringList objects;
    objects<<QString("{\"_type\":\"%1\", \"identifier\":\"%2\", \"settings\":{%3}}")
             .arg("com.nokia.mt.settings.SystemSettings")
             .arg("com.nokia.mt.settings.pstest.testAPI_MultipleWrites")
             .arg("\"setting1\":0, \"setting2\":true, \"setting3\":\"\"");
    jsonDbHandler.createJsonObjects(objects);

    QValueSpaceSubscriber subscriber(QValueSpace::PermanentLayer | QValueSpace::WritableLayer,
                                     "/com/nokia/mt/settings/pstest/testAPI_MultipleWrites");
    QVERIFY(subscriber.isConnected());

    QValueSpacePublisher publisher("/com/nokia/mt/settings/pstest/testAPI_MultipleWrites");
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
    objects<<"{\"_type\":\"com.nokia.mt.settings.SystemSettings\", "
             "\"identifier\":\"com.nokia.mt.settings.pstest.testAPI_MultipleWritesWithNotifications\", "
             "\"settings\": {\"setting1\":false}}";
    jsonDbHandler.createJsonObjects(objects);

    QValueSpaceSubscriber subscriber(QValueSpace::PermanentLayer | QValueSpace::WritableLayer,
                                     "/com/nokia/mt/settings/pstest/testAPI_MultipleWritesWithNotifications");
    QVERIFY(subscriber.isConnected());

    connect(&subscriber, SIGNAL(contentsChanged()), this, SLOT(onMultipleWritesWithNotifications()));

    QValueSpacePublisher publisher("/com/nokia/mt/settings/pstest/testAPI_MultipleWritesWithNotifications");
    QVERIFY(publisher.isConnected());

    bool nextValue = true;

    for (int i = 0; i < 50; i++) {
        qDebug()<<"LOOP"<<i + 1;

        changed = false;

        publisher.setValue("setting1", nextValue);

        nextValue = !nextValue;

        for (int i = 0; i < 1000; i++) {
            if (changed)
                return;

            QTest::qWait(1);
        }

        QVERIFY(value == !nextValue);
    }
}

void TestQValueSpaceJsonDb::onMultipleWritesWithNotifications()
{
    changed = true;
    static QValueSpaceSubscriber subscriber(QValueSpace::PermanentLayer | QValueSpace::WritableLayer,
                                            "/com/nokia/mt/settings/pstest/testAPI_MultipleWritesWithNotifications");

    value = subscriber.value("setting1").toBool();
}

void TestQValueSpaceJsonDb::contentsChanged()
{
    qDebug()<<"! BINGO !";

    changed = true;
}

#include "tst_valuespace_jsondb.moc"

QTEST_MAIN(TestQValueSpaceJsonDb)
