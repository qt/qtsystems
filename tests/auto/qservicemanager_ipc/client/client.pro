load(qttest_p4)

QT += serviceframework

CONFIG -= app_bundle

DEFINES += TESTDATA_DIR=\\\"$$PWD/\\\"

contains(jsondb_enabled, yes) {
    DEFINES += QT_JSONDB
} else {
    contains(QT_CONFIG,dbus):DEFINES+=SFW_USE_DBUS_BACKEND
}

SOURCES += tst_qservicemanager_ipc.cpp

symbian {
    TARGET.CAPABILITY = ALL -TCB
}
