load(qttest_p4)

INCLUDEPATH += ../../../src/serviceframework/ipc
include(../../../src/serviceframework/ipc/metaobjectbuilder.pri)

QT = core serviceframework

CONFIG += parallel_test

# Input

SOURCES += tst_qmetaobjectbuilder.cpp

symbian {
    TARGET.CAPABILITY = ALL -TCB
    LIBS += -lefsrv
}
