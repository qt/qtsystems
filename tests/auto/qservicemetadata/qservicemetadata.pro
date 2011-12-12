TARGET = tst_qservicemetadata
CONFIG += testcase

wince* {
    DEFINES+= TESTDATA_DIR=\\\".\\\"
}else {
    !mtlib:!contains(config_test_mtlib, yes) {
        DEFINES += TESTDATA_DIR=\\\"$$PWD/\\\"
    }
}
               

CONFIG += parallel_test

QT += serviceframework serviceframework-private testlib

# Input 
SOURCES += tst_servicemetadata.cpp


symbian {
    TARGET.CAPABILITY = ALL -TCB
}

addFiles.files = testdata/*
addFiles.path = testdata
DEPLOYMENT += addFiles
