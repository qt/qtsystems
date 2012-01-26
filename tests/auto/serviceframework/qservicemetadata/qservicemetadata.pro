TARGET = tst_qservicemetadata
CONFIG += testcase

CONFIG += parallel_test

QT += serviceframework serviceframework-private testlib
QT -= gui

# Input 
SOURCES += tst_servicemetadata.cpp


symbian {
    TARGET.CAPABILITY = ALL -TCB
}

addFiles.files = testdata/*
addFiles.path = testdata
DEPLOYMENT += addFiles
