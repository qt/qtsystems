# QML tests in this directory must not depend on an OpenGL context.

TEMPLATE = app
CONFIG += warn_on qmltestcase

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


