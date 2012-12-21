TARGET = tst_qservicemanager
CONFIG += testcase

QT = core sql serviceframework serviceframework-private testlib
QT -= gui

jsondb|qtHaveModule(jsondb) {
    mtlib|config_mtlib {
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
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
