TARGET = tst_qservicemetadata
CONFIG += testcase

CONFIG += parallel_test

QT += serviceframework serviceframework-private testlib
QT -= gui

# Input
SOURCES += tst_servicemetadata.cpp


TESTDATA += testdata/*
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
