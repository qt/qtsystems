TARGET = tst_qremoteserviceregister
CONFIG += testcase

QT = core serviceframework testlib
QT -= gui

SOURCES += tst_qremoteserviceregister.cpp
HEADERS += service.h

TESTDATA += xmldata/*

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
