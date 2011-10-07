TARGET = tst_servicedatabase
CONFIG += testcase

QT = core sql serviceframework serviceframework-private testlib

symbian {
    addFiles.sources = testdata/*
    addFiles.path = testdata
    DEPLOYMENT += addFiles
}

!symbian {
    DEFINES += TESTDATA_DIR=\\\"$$PWD/\\\"
}

# Input 
SOURCES += tst_servicedatabase.cpp \
            
symbian {
    TARGET.CAPABILITY = ALL -TCB
}
