QT = core gui serviceframework declarative
win32: CONFIG += console
mac:CONFIG -= app_bundle

SOURCES += main.cpp

RESOURCES += dialer.qrc

QML_CONTENT_FILES = \
content/DialButton.qml \
content/DialScreen.qml \
content/DialerList.qml \
content/qmldir \
content/call.png \
content/hangup.png

QML_MISC_FILES = \
dialer.qml \
icon.png

OTHER_FILES += \
content/DialButton.qml \
content/DialScreen.qml \
content/DialerList.qml \
content/qmldir \
content/call.png \
content/hangup.png \
dialer.qml \
icon.png


# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/dialer
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS $$QML_MISC_FILES dialer.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/dialer
qmlsources.files = $$QML_CONTENT_FILES
qmlsources.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/dialer/content
jsondb|contains(config_test_jsondb, yes) {
    info.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/dialer
    info.files = info.json
}
INSTALLS += target sources info qmlsources

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


symbian {
    TARGET.EPOCHEAPSIZE = 0x20000 0x2000000
}
