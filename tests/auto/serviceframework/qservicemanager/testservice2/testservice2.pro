TEMPLATE      = lib
CONFIG       += plugin

HEADERS       = testserviceplugin.h testservice.h testserviceinterface.h
SOURCES       = testserviceplugin.cpp
TARGET        = tst_sfw_testservice2plugin
DESTDIR = ../plugins

QT += serviceframework

target.path = $$[QT_INSTALL_TESTS]/tst_qservicemanager/plugins
INSTALLS += target
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
