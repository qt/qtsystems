TARGET = tst_qserviceinterfacedescriptor
CONFIG += testcase

QT += serviceframework serviceframework-private testlib
QT -= gui
CONFIG += parallel_test

SOURCES += tst_qserviceinterfacedescriptor.cpp

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
