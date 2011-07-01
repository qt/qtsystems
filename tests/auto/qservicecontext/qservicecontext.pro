load(qttest_p4)

CONFIG+=parallel_test

QT = core serviceframework

# Input 
HEADERS += 
SOURCES += tst_qservicecontext.cpp 

symbian {
    TARGET.CAPABILITY = ALL -TCB
}
