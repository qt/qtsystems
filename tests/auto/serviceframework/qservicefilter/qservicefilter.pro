TARGET = tst_qservicefilter
CONFIG += testcase

SOURCES += tst_qservicefilter.cpp

QT += serviceframework testlib
QT -= gui

CONFIG += parallel_test

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
