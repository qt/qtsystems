TARGET = tst_servicedeletion
CONFIG += testcase

CONFIG += core servoiceframewok

QT = core serviceframework testlib

DEFINES += SRCDIR=\\\"$$PWD/\\\"


# Input
SOURCES += tst_servicedeletion.cpp


symbian {
    TARGET.CAPABILITY = ALL -TCB
    LIBS += -lefsrv
    contains(S60_VERSION, 5.2)|contains(MOBILITY_SD_MCL_BUILD, yes){
        DEFINES += SYMBIAN_EMULATOR_SUPPORTS_PERPROCESS_WSD
    }
}
