load(qttest_p4)

SOURCES += tst_qservicefilter.cpp

QT += serviceframework

CONFIG += parallel_test

symbian {
    TARGET.CAPABILITY = ALL -TCB
}
