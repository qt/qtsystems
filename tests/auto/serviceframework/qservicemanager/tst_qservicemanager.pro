TARGET = tst_qservicemanager
CONFIG += testcase

QT = core sql serviceframework serviceframework-private testlib
QT -= gui
CONFIG += parallel_test

jsondb|contains(QT_CONFIG, jsondb) {
    mtlib|contains(config_test_mtlib, yes) {
        DEFINES += QT_ADDON_JSONDB_LIB
        QT += jsondb jsondb-private
    }
}

# Input 
HEADERS += sampleservice/sampleserviceplugin.h \
           ../qsfwtestutil.h
SOURCES += tst_qservicemanager.cpp \
           sampleservice/sampleserviceplugin.cpp \
           ../qsfwtestutil.cpp


addFiles.files = xml/testserviceplugin.xml \
                    xml/sampleservice.xml \
                    xml/sampleservice2.xml
addFiles.path = xml
DEPLOYMENT += addFiles
