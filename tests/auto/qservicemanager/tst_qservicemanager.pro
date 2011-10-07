TARGET = tst_qservicemanager
CONFIG += testcase

QT = core sql serviceframework serviceframework-private testlib
CONFIG += parallel_test

DEFINES += OUTDIR=\\\"$$OUT_PWD/\\\" SRCDIR=\\\"$$PWD/\\\"

contains(config_test_jsondb, yes) {
    INCLUDEPATH += $$[QT_INSTALL_PREFIX]/opt/mt/include /opt/mt/include
    LIBS += -L$$[QT_INSTALL_PREFIX]/opt/mt/lib -lmtcore -Wl,-rpath,$$[QT_INSTALL_PREFIX]/opt/mt/lib
    LIBS += -L/opt/mt/lib -lmtcore -Wl,-rpath,/opt/mt/lib
    PKGCONFIG += mtcore
    DEFINES += QT_ADDON_JSONDB_LIB
    QT += jsondb jsondb-private
}

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
