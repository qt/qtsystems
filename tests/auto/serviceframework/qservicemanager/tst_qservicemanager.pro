TARGET = tst_qservicemanager
CONFIG += testcase

QT = core sql serviceframework serviceframework-private testlib
QT -= gui
CONFIG += parallel_test

jsondb|contains(QT_CONFIG, jsondb) {
    mtlib|contains(config_test_mtlib, yes) {
        DEFINES += QT_ADDON_JSONDB_LIB
        QT += jsondb
    }
}

# Input 
HEADERS += sampleservice/sampleserviceplugin.h \
           ../qsfwtestutil.h
SOURCES += tst_qservicemanager.cpp \
           sampleservice/sampleserviceplugin.cpp \
           ../qsfwtestutil.cpp


TESTDATA += xml/*
