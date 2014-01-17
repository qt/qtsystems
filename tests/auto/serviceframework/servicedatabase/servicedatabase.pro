TARGET = tst_servicedatabase
CONFIG += testcase

QT = core sql serviceframework serviceframework-private testlib

TESTDATA += testdata/*

# Input
SOURCES += tst_servicedatabase.cpp \

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
