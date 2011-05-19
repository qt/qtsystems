load(qt_module)

DESTDIR  = $$BUILD_TREE/lib

TARGET = QtSystemInfo
target.path = $$PREFIX/lib
INSTALLS += target

QT = core

CONFIG += module
MODULE_PRI = ../../modules/qt_systeminfo.pri

DEFINES += QT_BUILD_SYSTEMINFO_LIB

include(../src.pri)
