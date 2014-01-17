TARGET = tst_qservicemanager
CONFIG += testcase

QT = core sql serviceframework serviceframework-private testlib
QT -= gui

# Input
HEADERS += sampleservice/sampleserviceplugin.h \
           ../qsfwtestutil.h
SOURCES += tst_qservicemanager.cpp \
           sampleservice/sampleserviceplugin.cpp \
           ../qsfwtestutil.cpp


TESTDATA += xml/*
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
