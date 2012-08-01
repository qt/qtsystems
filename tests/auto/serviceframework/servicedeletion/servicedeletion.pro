TARGET = tst_servicedeletion
CONFIG += testcase

QT = core serviceframework testlib
QT -= gui

# Input
SOURCES += tst_servicedeletion.cpp

TESTDATA += xmldata/*
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
