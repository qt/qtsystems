QT.serviceframework.VERSION = 5.0.0
QT.serviceframework.MAJOR_VERSION = 5
QT.serviceframework.MINOR_VERSION = 0
QT.serviceframework.PATCH_VERSION = 0

QT.serviceframework.name = QtServiceFramework
QT.serviceframework.bins = $$QT_MODULE_BIN_BASE
QT.serviceframework.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtServiceFramework
QT.serviceframework.private_includes = $$QT_MODULE_INCLUDE_BASE/QtServiceFramework/$$QT.serviceframework.VERSION
QT.serviceframework.sources = $$QT_MODULE_BASE/src/serviceframework
QT.serviceframework.libs = $$QT_MODULE_LIB_BASE
QT.serviceframework.plugins = $$QT_MODULE_PLUGIN_BASE
QT.serviceframework.imports = $$QT_MODULE_IMPORT_BASE
QT.serviceframework.depends = core gui network
QT.serviceframework.DEFINES = QT_SERVICEFRAMEWORK_LIB

QT_CONFIG += serviceframework
