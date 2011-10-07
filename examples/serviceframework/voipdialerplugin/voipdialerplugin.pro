QT = core serviceframework

TEMPLATE = lib
CONFIG += plugin

INCLUDEPATH += ../../../src/serviceframework

HEADERS += voipdialer.h \
           voipdialerplugin.h

SOURCES += voipdialer.cpp \
           voipdialerplugin.cpp

TARGET = voipdialerplugin

contains(QT_CONFIG, declarative):DEFINES += DECLARATIVE

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/voipdialerplugin
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS voipdialerplugin.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/voipdialerplugin
xml.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/xmldata
xml.files = voipdialerservice.xml

INSTALLS += target sources xml addFiles

symbian: CONFIG += qt_example
maemo5: CONFIG += qt_example

symbian*: {
    addFiles.sources = remotedialerservice.xml
}

symbian {
    load(data_caging_paths)
    pluginDep.sources = serviceframework_voipdialerservice.dll
    pluginDep.path = $$QT_PLUGINS_BASE_DIR
    DEPLOYMENT += pluginDep

    addFiles.sources = voipdialerservice.xml
    addFiles.path = /private/2002AC7F/import/
    DEPLOYMENT += addFiles

    DEPLOYMENT += xmlautoimport

    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = LocalServices Location NetworkServices ReadUserData WriteUserData UserEnvironment
    load(armcc_warnings)
}
else {
    DEFINES += TESTDATA_DIR=\\\"$$PWD/\\\"
}

