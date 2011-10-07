TARGET = tst_qservicemetadata
CONFIG += testcase

wince* {
    DEFINES+= TESTDATA_DIR=\\\".\\\"
}else:!symbian {
    DEFINES += TESTDATA_DIR=\\\"$$PWD/\\\"
}
               

CONFIG += parallel_test

QT += serviceframework serviceframework-private testlib

# Input 
SOURCES += tst_servicemetadata.cpp


symbian {
    TARGET.CAPABILITY = ALL -TCB
}
