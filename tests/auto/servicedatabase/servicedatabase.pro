load(qttest_p4)

QT = core sql serviceframework serviceframework-private

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
