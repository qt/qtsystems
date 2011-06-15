QT.systeminfo.VERSION = 5.0.0
QT.systeminfo.MAJOR_VERSION = 5
QT.systeminfo.MINOR_VERSION = 0
QT.systeminfo.PATCH_VERSION = 0

QT.systeminfo.name = QtSystemInfo
QT.systeminfo.bins = $$QT_MODULE_BIN_BASE
QT.systeminfo.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtSystemInfo
QT.systeminfo.private_includes = $$QT_MODULE_INCLUDE_BASE/QtSystemInfo/$$QT.systeminfo.VERSION
QT.systeminfo.sources = $$QT_MODULE_BASE/src/systeminfo
QT.systeminfo.libs = $$QT_MODULE_LIB_BASE
QT.systeminfo.plugins = $$QT_MODULE_PLUGIN_BASE
QT.systeminfo.imports = $$QT_MODULE_IMPORT_BASE
QT.systeminfo.depends = core gui network

QT_CONFIG += systeminfo
