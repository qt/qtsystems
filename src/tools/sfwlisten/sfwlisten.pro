TEMPLATE = app
TARGET = sfwlisten

CONFIG -= app_bundle

DESTDIR = $$QT.serviceframework.bins

QT  = core network

SOURCES = main.cpp

target.path = $$[QT_INSTALL_BINS]
INSTALLS += target

CONFIG += console
load(qt_targets)
