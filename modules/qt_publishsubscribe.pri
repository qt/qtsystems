QT.publishsubscribe.VERSION = 5.0.0
QT.publishsubscribe.MAJOR_VERSION = 5
QT.publishsubscribe.MINOR_VERSION = 0
QT.publishsubscribe.PATCH_VERSION = 0

QT.publishsubscribe.name = QtPublishSubscribe
QT.publishsubscribe.bins = $$QT_MODULE_BIN_BASE
QT.publishsubscribe.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtPublishSubscribe
QT.publishsubscribe.private_includes = $$QT_MODULE_INCLUDE_BASE/QtPublishSubscribe/$$QT.publishsubscribe.VERSION
QT.publishsubscribe.sources = $$QT_MODULE_BASE/src/publishsubscribe
QT.publishsubscribe.libs = $$QT_MODULE_LIB_BASE
QT.publishsubscribe.plugins = $$QT_MODULE_PLUGIN_BASE
QT.publishsubscribe.imports = $$QT_MODULE_IMPORT_BASE
QT.publishsubscribe.depends = core
QT.publishsubscribe.DEFINES = QT_PUBLISHSUBSCRIBE_LIB

QT_CONFIG += publishsubscribe
