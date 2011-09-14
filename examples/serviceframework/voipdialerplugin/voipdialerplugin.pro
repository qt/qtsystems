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

symbian {
    load(data_caging_paths)
    pluginDep.sources = serviceframework_voipdialerservice.dll
    pluginDep.path = $$QT_PLUGINS_BASE_DIR
    DEPLOYMENT += pluginDep

    xmlautoimport.path = /private/2002AC7F/import/
    xmlautoimport.sources = voipdialerservice.xml
    DEPLOYMENT += xmlautoimport

    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = LocalServices Location NetworkServices ReadUserData WriteUserData UserEnvironment
    load(armcc_warnings)
}

#not needed for symbian due to autoimport above
xml.path = $$QT_MOBILITY_EXAMPLES/xmldata
xml.files = voipdialerservice.xml
INSTALLS += xml
