TEMPLATE = app
TARGET = servicefw
DESTDIR = $$QT.serviceframework.bins

QT += serviceframework
QT -= gui
DEFINES += IGNORE_SERVICEMETADATA_EXPORT
INCLUDEPATH += ../../serviceframework

SOURCES = servicefw.cpp \
          ../../serviceframework/servicemetadata.cpp
HEADERS += ../../serviceframework/servicemetadata_p.h

target.path = $$[QT_INSTALL_BINS]
INSTALLS += target

CONFIG += console
CONFIG -= app_bundle
load(qt_targets)
