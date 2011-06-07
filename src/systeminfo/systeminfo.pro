load(qt_module)

include(../src.pri)

TEMPLATE = lib
VERSION = $${QT.systeminfo.VERSION}
DESTDIR  = $$QT_SYSTEMKIT_BUILD_TREE/lib

TARGET = QtSystemInfo
target.path = $$QT_SYSTEMKIT_LIB
INSTALLS += target

QT = core network

CONFIG += module
MODULE_PRI = ../../modules/qt_systeminfo.pri

DEFINES += QT_BUILD_SYSTEMINFO_LIB

PUBLIC_HEADERS += qdeviceinfo.h \
                  qdisplayinfo.h \
                  qstorageinfo.h \
                  qscreensaver.h \
                  qbatteryinfo.h \
                  qnetworkinfo.h \
                  qdeviceprofile.h \
                  qinputdeviceinfo.h

SOURCES += qdeviceinfo.cpp \
           qdisplayinfo.cpp \
           qstorageinfo.cpp \
           qscreensaver.cpp \
           qbatteryinfo.cpp \
           qnetworkinfo.cpp \
           qdeviceprofile.cpp \
           qinputdeviceinfo.cpp

unix {
    LIBS += -lXrandr

    PRIVATE_HEADERS += qdeviceinfo_linux_p.h \
                       qdisplayinfo_linux_p.h \
                       qstorageinfo_linux_p.h \
                       qscreensaver_linux_p.h \
                       qbatteryinfo_linux_p.h \
                       qnetworkinfo_linux_p.h \
                       qinputdeviceinfo_linux_p.h

    SOURCES += qdeviceinfo_linux.cpp \
               qdisplayinfo_linux.cpp \
               qstorageinfo_linux.cpp \
               qscreensaver_linux.cpp \
               qbatteryinfo_linux.cpp \
               qnetworkinfo_linux.cpp \
               qinputdeviceinfo_linux.cpp

    contains(QT_CONFIG, dbus): {
        QT += dbus

        PRIVATE_HEADERS += qofonowrapper_p.h

        SOURCES += qofonowrapper.cpp

    } else {
        DEFINES += QT_NO_OFONO
    }

    contains(blkid_enabled, yes) {
        LIBS += -lblkid
    } else {
        DEFINES += QT_NO_BLKID
    }
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

headers.files = $$PUBLIC_HEADERS
headers.path = $$QT_SYSTEMKIT_INCLUDE/$$TARGET
INSTALLS += headers
