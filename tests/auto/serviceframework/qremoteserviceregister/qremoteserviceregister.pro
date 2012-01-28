TARGET = tst_qremoteserviceregister
CONFIG += testcase

QT = core serviceframework testlib
QT -= gui

SOURCES += tst_qremoteserviceregister.cpp
HEADERS += service.h

!mtlib:!contains(config_test_mtlib, yes): DEFINES += SRCDIR=\\\"$$PWD/\\\"

addFiles.files = xmldata/*
addFiles.path = xmldata
DEPLOYMENT += addFiles
