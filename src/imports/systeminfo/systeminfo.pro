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

HEADERS += \
    qdeclarativebatteryinfo_p.h \
    qdeclarativedeviceinfo_p.h \
    qdeclarativenetworkinfo_p.h \
    qdeclarativestorageinfo_p.h

SOURCES += \
    qdeclarativebatteryinfo.cpp \
    qdeclarativedeviceinfo.cpp \
    qdeclarativenetworkinfo.cpp \
    qdeclarativestorageinfo.cpp \
    qsysteminfo.cpp
