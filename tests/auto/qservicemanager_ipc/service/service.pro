TARGET = qt_sfw_example_ipc_unittest
TEMPLATE = app

mac {
    CONFIG -= app_bundle
}

DESTDIR = ../client  #service must be in same dir as client binary

jsondb|contains(QT_CONFIG, jsondb) {
    mtlib|contains(config_test_mtlib, yes) {
        DEFINES += QT_JSONDB
    }
}

!contains(DEFINES, QT_JSONDB):contains(QT_CONFIG,dbus) {
    QT += dbus
    DEFINES+=SFW_USE_DBUS_BACKEND
}

QT += serviceframework serviceframework-private


SOURCES += main.cpp
symbian {
    TARGET.CAPABILITY = ALL -TCB
}
!mtlib:!contains(config_test_mtlib, yes): DEFINES += TESTDATA_DIR=\\\"$$PWD/\\\"

RPM_PACKAGE_NAME = $$(RPM_PACKAGE_NAME)
isEmpty(RPM_PACKAGE_NAME) {
    RPM_PACKAGE_NAME = unknown-package
}
QMAKE_EXTRA_VARIABLES += RPM_PACKAGE_NAME

target.path = $$[QT_INSTALL_LIBS]/$(EXPORT_RPM_PACKAGE_NAME)-tests/tst_client

addFiles.files = ../xmldata/*
addFiles.path = $$[QT_INSTALL_LIBS]/$(EXPORT_RPM_PACKAGE_NAME)-tests/tst_client/xmldata
INSTALLS += addFiles
