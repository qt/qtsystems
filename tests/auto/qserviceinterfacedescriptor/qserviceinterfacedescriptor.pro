load(qttest_p4)

QT += serviceframework serviceframework-private
CONFIG += parallel_test

SOURCES += tst_qserviceinterfacedescriptor.cpp

symbian {
    TARGET.CAPABILITY = ALL -TCB
}
