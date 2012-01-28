TARGET = tst_servicedeletion
CONFIG += testcase

CONFIG += core servoiceframewok

QT = core serviceframework testlib
QT -= gui

!mtlib:!contains(config_test_mtlib, yes): DEFINES += SRCDIR=\\\"$$PWD/\\\"

# Input
SOURCES += tst_servicedeletion.cpp

addFiles.files = xmldata/*
addFiles.path = xmldata
DEPLOYMENT += addFiles
