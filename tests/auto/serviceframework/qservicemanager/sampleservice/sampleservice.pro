TEMPLATE      = lib
CONFIG       += plugin

HEADERS       = sampleserviceplugin.h
SOURCES       = sampleserviceplugin.cpp
TARGET        = tst_sfw_sampleserviceplugin
DESTDIR = ../plugins

QT += serviceframework

RPM_PACKAGE_NAME = $$(RPM_PACKAGE_NAME)
isEmpty(RPM_PACKAGE_NAME) {
    RPM_PACKAGE_NAME = unknown-package
}
QMAKE_EXTRA_VARIABLES += RPM_PACKAGE_NAME

target.path = $$[QT_INSTALL_LIBS]/$(EXPORT_RPM_PACKAGE_NAME)-tests/tst_qservicemanager/plugins
