load(qt_module)

include(../../src.pri)

TEMPLATE = lib
DESTDIR  = $$QT_SYSTEMKIT_BUILD_TREE/imports/Qt/systeminfo
CONFIG += qt plugin

TARGET  = $$qtLibraryTarget(declarative_systeminfo)
target.path = $$QT_SYSTEMKIT_IMPORTS/Qt/systeminfo
INSTALLS += target

qmldir.files += $$PWD/qmldir
qmldir.path +=  $$QT_SYSTEMKIT_IMPORTS/Qt/systeminfo
INSTALLS += qmldir

QT += declarative

INCLUDEPATH += $$QT_SYSTEMKIT_SOURCE_TREE/src/systeminfo
LIBS += -L$$QT_SYSTEMKIT_BUILD_TREE/lib -lQtSystemInfo
unix: QMAKE_RPATHDIR += $$QT_SYSTEMKIT_BUILD_TREE/lib

SOURCES += qsysteminfo.cpp
