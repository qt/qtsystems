TARGET = tst_servicedatabase
CONFIG += testcase

QT = core sql serviceframework serviceframework-private testlib

addFiles.files = testdata/*
addFiles.path = testdata
DEPLOYMENT += addFiles


!mtlib:!contains(config_test_mtlib, yes) {
    DEFINES += TESTDATA_DIR=\\\"$$PWD/\\\"
}

# Input 
SOURCES += tst_servicedatabase.cpp \
            
