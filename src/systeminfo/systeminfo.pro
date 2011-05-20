load(qt_module)

TEMPLATE = lib
DESTDIR  = $$BUILD_TREE/lib

TARGET = QtSystemInfo
target.path = $$PREFIX/lib
INSTALLS += target

QT = core

CONFIG += module
MODULE_PRI = ../../modules/qt_systeminfo.pri

DEFINES += QT_BUILD_SYSTEMINFO_LIB

include(../src.pri)

HEADERS += qdeviceinfo.h \
           qdisplayinfo.h \
           qstorageinfo.h

SOURCES += qdeviceinfo.cpp \
           qdisplayinfo.cpp \
           qstorageinfo.cpp

unix {
    LIBS += -lXrandr -lblkid

    HEADERS += qdeviceinfo_linux_p.h \
               qdisplayinfo_linux_p.h \
               qstorageinfo_linux_p.h

    SOURCES += qdeviceinfo_linux.cpp \
               qdisplayinfo_linux.cpp \
               qstorageinfo_linux.cpp
}
