# QML tests in this directory must not depend on an OpenGL context.

TEMPLATE = app
CONFIG += qmltestcase

QT += serviceframework qml

TARGET = tst_serviceobject
CONFIG(debug_and_release) {
  CONFIG(debug, debug|release) {
    DESTDIR = ../debug
  } else {
    DESTDIR = ../release
  }
} else {
  DESTDIR = ..
}

SOURCES += ../main.cpp

OTHER_FILES += ../*.qml


DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
