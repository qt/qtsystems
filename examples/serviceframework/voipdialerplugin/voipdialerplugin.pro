QT = core serviceframework

TEMPLATE = lib
CONFIG += plugin

INCLUDEPATH += ../../../src/serviceframework

HEADERS += voipdialer.h \
           voipdialerplugin.h

SOURCES += voipdialer.cpp \
           voipdialerplugin.cpp

TARGET = voipdialerplugin

qtHaveModule(qml): DEFINES += DECLARATIVE

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/voipdialerplugin
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS voipdialerplugin.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/voipdialerplugin
xml.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/xmldata
xml.files = voipdialerservice.xml

INSTALLS += target sources xml

DEFINES += TESTDATA_DIR=\\\"$$PWD/\\\"

