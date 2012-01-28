TEMPLATE      = lib
CONFIG       += plugin

HEADERS       = testserviceplugin.h testservice.h testserviceinterface.h
SOURCES       = testserviceplugin.cpp
TARGET        = tst_sfw_testservice2plugin
DESTDIR = ../plugins

QT += serviceframework

RPM_PACKAGE_NAME = $$(RPM_PACKAGE_NAME)
isEmpty(RPM_PACKAGE_NAME) {
    RPM_PACKAGE_NAME = unknown-package
}
QMAKE_EXTRA_VARIABLES += RPM_PACKAGE_NAME

target.path = $$[QT_INSTALL_LIBS]/$(EXPORT_RPM_PACKAGE_NAME)-tests/tst_qservicemanager/plugins
