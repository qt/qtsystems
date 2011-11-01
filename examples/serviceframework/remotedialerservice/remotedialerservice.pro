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
jsondb {
    info.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/remotedialerservice
    info.files = info.json interfaces.json
}
INSTALLS += target sources xml info

symbian: CONFIG += qt_example
maemo5: CONFIG += qt_example

symbian*: {
    addFiles.sources = remotedialerservice.xml
    addFiles.path = /private/2002AC7F/import/
    DEPLOYMENT += addFiles
}
else {
    DEFINES += TESTDATA_DIR=\\\"$$PWD/\\\"
}
