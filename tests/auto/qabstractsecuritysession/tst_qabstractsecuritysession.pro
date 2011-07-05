load(qttest_p4)

QT = core serviceframework

DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += ../qsfwtestutil.h
SOURCES += tst_qabstractsecuritysession.cpp \
           ../qsfwtestutil.cpp

symbian|wince* {
    
    symbian {
        TARGET.CAPABILITY = ALL -TCB
        LIBS += -lefsrv
        contains(S60_VERSION, 5.2)|contains(MOBILITY_SD_MCL_BUILD, yes){
            DEFINES += SYMBIAN_EMULATOR_SUPPORTS_PERPROCESS_WSD
        }
    }

    addFiles.sources = ../../testservice2/xml/testserviceplugin.xml
    addFiles.path = plugins/xmldata
    DEPLOYMENT += addFiles
}
