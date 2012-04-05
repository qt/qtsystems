QT = core gui serviceframework qml quick
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
mtlib|config_mtlib {
    info.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework/dialer
    info.files = info.json
}
INSTALLS += target sources info qmlsources

maemo5: CONFIG += qt_example

DEFINES += TESTDATA_DIR=\\\"$$PWD/\\\"
