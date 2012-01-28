TEMPLATE      = lib
CONFIG       += plugin

HEADERS       = sampleserviceplugin2.h
SOURCES       = sampleserviceplugin2.cpp
TARGET        = tst_sfw_sampleserviceplugin2
DESTDIR = ../plugins

QT += serviceframework

RPM_PACKAGE_NAME = $$(RPM_PACKAGE_NAME)
isEmpty(RPM_PACKAGE_NAME) {
    RPM_PACKAGE_NAME = unknown-package
}
QMAKE_EXTRA_VARIABLES += RPM_PACKAGE_NAME

target.path = $$[QT_INSTALL_LIBS]/$(EXPORT_RPM_PACKAGE_NAME)-tests/tst_qservicemanager/plugins
