TARGET = tst_qserviceinterfacedescriptor
CONFIG += testcase

QT += serviceframework serviceframework-private testlib
CONFIG += parallel_test

SOURCES += tst_qserviceinterfacedescriptor.cpp

symbian {
    TARGET.CAPABILITY = ALL -TCB
}
