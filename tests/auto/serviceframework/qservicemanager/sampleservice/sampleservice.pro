TEMPLATE      = lib
CONFIG       += plugin

HEADERS       = sampleserviceplugin.h
SOURCES       = sampleserviceplugin.cpp
TARGET        = tst_sfw_sampleserviceplugin
DESTDIR = ../plugins

QT += serviceframework

target.path = $$[QT_INSTALL_TESTS]/tst_qservicemanager/plugins
INSTALLS += target
