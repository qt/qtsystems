TEMPLATE=app
TARGET=tst_qvaluespace_qml
target.path = /tmp
INSTALLS += target
SOURCES += tst_qvaluespace_qml.cpp
OTHER_FILES += \
    testcases/ContactsVersitTestCase.qml \
    testcases/tst_read_settings_object.qml \
    testcases/TestTools.qml \
    testcases/tst_read_setting.qml \
    testcases/tst_change_directory.qml \
    testcases/tst_change_setting.qml

QT += qmltest publishsubscribe jsondb
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
