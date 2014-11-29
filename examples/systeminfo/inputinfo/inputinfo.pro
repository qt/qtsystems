#-------------------------------------------------
#
# Project created by QtCreator 2014-11-28T12:25:04
#
#-------------------------------------------------

QT       += core systeminfo

QT       -= gui

TARGET = inputinfo
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    inputtest.cpp

HEADERS += \
    inputtest.h

target.path = $$[QT_INSTALL_EXAMPLES]/systeminfo/inputinfo
        app.path = $$[QT_INSTALL_EXAMPLES]/systeminfo/inputinfo
        INSTALLS += target app
