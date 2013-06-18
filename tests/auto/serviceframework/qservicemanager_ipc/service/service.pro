TARGET = qt_sfw_example_ipc_unittest
TEMPLATE = app

mac {
    CONFIG -= app_bundle
}

debug_and_release {
    CONFIG(debug, debug|release): \
        INFIX = /debug
    else: \
        INFIX = /release
}
DESTDIR = ../client$$INFIX  #service must be in same dir as client binary

qtHaveModule(dbus) {
    QT += dbus
    DEFINES+=SFW_USE_DBUS_BACKEND
}

QT += serviceframework serviceframework-private
QT += testlib # QFINDTESTDATA
DEFINES += TESTDATA_INSTALL_DIR=\\\"$$[QT_INSTALL_TESTS]/tst_qservicemanager_ipc/\\\"
DEFINES += TESTDATA_SRC_DIR=\\\"$$PWD/\\\"
SOURCES += main.cpp

target.path = $$[QT_INSTALL_TESTS]/tst_qservicemanager_ipc
INSTALLS += target
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
