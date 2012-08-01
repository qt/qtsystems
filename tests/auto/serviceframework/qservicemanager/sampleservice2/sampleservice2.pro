TEMPLATE      = lib
CONFIG       += plugin

HEADERS       = sampleserviceplugin2.h
SOURCES       = sampleserviceplugin2.cpp
TARGET        = tst_sfw_sampleserviceplugin2
DESTDIR = ../plugins

QT += serviceframework

target.path = $$[QT_INSTALL_TESTS]/tst_qservicemanager/plugins
INSTALLS += target
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
