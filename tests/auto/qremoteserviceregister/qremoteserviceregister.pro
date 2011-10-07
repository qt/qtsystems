TARGET = tst_qremoteserviceregister
CONFIG += testcase

QT = core serviceframework testlib

SOURCES += tst_qremoteserviceregister.cpp
HEADERS += service.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"

symbian {
    TARGET.CAPABILITY = ALL -TCB
}

symbian* {
    addFiles.sources = testdata/*
    addFiles.path = xmldata
    DEPLOYMENT += addFiles
}
