load(qttest_p4)

wince* {
    DEFINES+= TESTDATA_DIR=\\\".\\\"
}else:!symbian {
    DEFINES += TESTDATA_DIR=\\\"$$PWD/\\\"
}
               

CONFIG += parallel_test

QT += serviceframework serviceframework-private

# Input 
SOURCES += tst_servicemetadata.cpp


symbian {
    TARGET.CAPABILITY = ALL -TCB
}
