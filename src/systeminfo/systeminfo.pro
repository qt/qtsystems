load(qt_module)

TARGET = QtSystemInfo
QPRO_PWD = $PWD

CONFIG += module
MODULE_PRI = ../../modules/qt_systeminfo.pri

QT = core gui network widgets

DEFINES += QT_BUILD_SYSTEMINFO_LIB QT_MAKEDLL

load(qt_module_config)

PUBLIC_HEADERS = qsysteminfoglobal.h \
                 qdeviceinfo.h \
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

!win32:!embedded:!qpa:!mac:!symbian: CONFIG += x11

win32 {
    contains(CONFIG, release) {
       CONFIG -= console
    }

    win32-msvc*: {
        LIBS += -lUser32 -lGdi32 -lPowrProf -lBthprops -lWlanapi -lWs2_32 -lVfw32
    }

    win32-g++: {
        LIBS += -luser32 -lgdi32 -lpowrprof -lbthprops -lwlanapi -lws2_32 -lvfw32
    }

    PRIVATE_HEADERS += qscreensaver_win_p.h \
                       qinputdeviceinfo_win_p.h \
                       qdisplayinfo_win_p.h \
                       qdeviceinfo_win_p.h \
                       qstorageinfo_win_p.h \
                       qbatteryinfo_win_p.h \
                       qnetworkinfo_win_p.h

    SOURCES += qscreensaver_win.cpp \
               qinputdeviceinfo_win.cpp \
               qdisplayinfo_win.cpp \
               qdeviceinfo_win.cpp \
               qstorageinfo_win.cpp \
               qbatteryinfo_win.cpp \
               qnetworkinfo_win.cpp
}

x11 {
    LIBS += -lXrandr -lX11
}

linux-* {
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
        contains(config_test_ofono, yes) {
            QT += dbus
            PRIVATE_HEADERS += qofonowrapper_p.h
            SOURCES += qofonowrapper.cpp
        } else {
            DEFINES += QT_NO_OFONO
        }

        contains(config_test_udisks, yes): {
            QT += dbus
        } else: {
            DEFINES += QT_NO_UDISKS
        }

        contains(config_test_bluez, yes) {
            QT += dbus
            CONFIG += link_pkgconfig
            PKGCONFIG += bluez

            PRIVATE_HEADERS += qbluezwrapper_p.h
            SOURCES += qbluezwrapper.cpp
        } else {
            DEFINES += QT_NO_BLUEZ
        }
    } else {
        DEFINES += QT_NO_OFONO QT_NO_UDISKS QT_NO_BLUEZ
    }

    contains(config_test_udev, yes) {
        CONFIG += link_pkgconfig
        PKGCONFIG += udev
        LIBS += -ludev
    } else {
        DEFINES += QT_NO_UDEV
    }
}

HEADERS = qtsysteminfoversion.h $$PUBLIC_HEADERS $$PRIVATE_HEADERS
