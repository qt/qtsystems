TARGET = tst_client
CONFIG += testcase

QT += serviceframework testlib

CONFIG -= app_bundle

DEFINES += TESTDATA_DIR=\\\"$$PWD/\\\"

jsondb|contains(QT_CONFIG, jsondb): {
    mtcore|contains(config_test_mtcore, yes): {
        DEFINES += QT_JSONDB
    }
}

!contains(DEFINES, QT_JSONDB):contains(QT_CONFIG,dbus): {
    QT += dbus
    DEFINES+=SFW_USE_DBUS_BACKEND
}

SOURCES += tst_qservicemanager_ipc.cpp

symbian {
    TARGET.CAPABILITY = ALL -TCB
}
