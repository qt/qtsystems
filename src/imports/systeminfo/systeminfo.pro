TARGET  = declarative_systeminfo
TARGETPATH = QtSystemInfo
include(qsysteminfoimport.pri)
target.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH
DESTDIR = $$QT.systeminfo.imports/$$TARGETPATH
INSTALLS += target

qmldir.files += $$PWD/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$TARGETPATH
INSTALLS += qmldir

QT += declarative systeminfo

SOURCES += qsysteminfo.cpp
