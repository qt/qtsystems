TARGET = tst_qservicemanager_ipc
CONFIG += testcase
testcase.timeout = 600 # this test is slow

QT += serviceframework testlib
QT -= gui

# Increase the stack size on MSVC to 4M to avoid a stack overflow
win32-msvc*:QMAKE_LFLAGS += /STACK:4194304

CONFIG -= app_bundle

qtHaveModule(dbus): {
    QT += dbus
    DEFINES+=SFW_USE_DBUS_BACKEND
}

SOURCES += tst_qservicemanager_ipc.cpp

TESTDATA += xmldata/*
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
