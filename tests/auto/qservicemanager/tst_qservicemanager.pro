load(qttest_p4)

QT = core sql serviceframework serviceframework-private
CONFIG += parallel_test


# Input 
HEADERS += sampleservice/sampleserviceplugin.h \
           ../qsfwtestutil.h
SOURCES += tst_qservicemanager.cpp \
           sampleservice/sampleserviceplugin.cpp \
           ../qsfwtestutil.cpp


symbian|wince* {
    symbian {
        TARGET.CAPABILITY = ALL -TCB
        LIBS += -lefsrv
        contains(S60_VERSION, 5.2)|contains(MOBILITY_SD_MCL_BUILD, yes){
            DEFINES += SYMBIAN_EMULATOR_SUPPORTS_PERPROCESS_WSD
        }
    }

    addFiles.sources = xml/testserviceplugin.xml \
                       sampleservice/xml/sampleservice.xml \
                       sampleservice2/xml/sampleservice2.xml
    addFiles.path = plugins/xmldata
    DEPLOYMENT += addFiles
}
