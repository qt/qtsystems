QT = core serviceframework
win32: CONFIG += console
mac:CONFIG -= app_bundle

HEADERS += \
           remotedialerservice.h
SOURCES += \
           main.cpp \
           remotedialerservice.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/remotedialer
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS remotedialer.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/remotedialer
xml.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/xmldata
xml.files = remotedialerservice.xml
INSTALLS += target sources xml

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
