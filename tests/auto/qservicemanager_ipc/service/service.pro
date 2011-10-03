TARGET = qt_sfw_example_ipc_unittest
TEMPLATE = app

mac {
    CONFIG -= app_bundle
}

DESTDIR = ../client  #service must be in same dir as client binary

contains(jsondb_enabled, yes) {
    DEFINES += QT_JSONDB
} else {
    contains(QT_CONFIG,dbus):DEFINES+=SFW_USE_DBUS_BACKEND
}

QT += serviceframework serviceframework-private


SOURCES += main.cpp

symbian {
    TARGET.CAPABILITY = ALL -TCB
    addFiles.sources = testdata/*
    addFiles.path = xmldata
    DEPLOYMENT += addFiles
}

DEFINES += TESTDATA_DIR=\\\"$$PWD/\\\"
