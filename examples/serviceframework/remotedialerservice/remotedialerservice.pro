QT = core serviceframework
win32: CONFIG += console
mac:CONFIG -= app_bundle

HEADERS += \
           remotedialerservice.h
SOURCES += \
           main.cpp \
           remotedialerservice.cpp

# install
target.path =  $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/remotedialerservice
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS remotedialerservice.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/remotedialerservice
xml.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/xmldata
xml.files = remotedialerservice.xml
mtlib|config_mtlib {
    info.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/remotedialerservice
    info.files = info.json interfaces.json
}
INSTALLS += target sources xml info

DEFINES += TESTDATA_DIR=\\\"$$PWD/\\\"

