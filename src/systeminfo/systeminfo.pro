load(qt_module)

TARGET = QtSystemInfo
QPRO_PWD = $PWD

CONFIG += module
MODULE_PRI = ../../modules/qt_systeminfo.pri

QT = core gui network

DEFINES += QT_BUILD_SYSTEMINFO_LIB QT_MAKEDLL

load(qt_module_config)

PUBLIC_HEADERS = qsysteminfoglobal.h \
                 qdeviceinfo.h \
                 qdisplayinfo.h \
                 qstorageinfo.h \
                 qscreensaver.h \
                 qbatteryinfo.h \
                 qnetworkinfo.h \
                 qdeviceprofile.h

SOURCES += qdeviceinfo.cpp \
           qdisplayinfo.cpp \
           qstorageinfo.cpp \
           qscreensaver.cpp \
           qbatteryinfo.cpp \
           qnetworkinfo.cpp \
           qdeviceprofile.cpp

win32: !simulator: {
    contains(CONFIG, release) {
       CONFIG -= console
    }

    win32-msvc*: {
        LIBS += -lUser32 -lGdi32 -lPowrProf -lBthprops -lWlanapi -lWs2_32 -lVfw32
    }

    win32-g++*: {
        LIBS += -luser32 -lgdi32 -lpowrprof -lbthprops -lwlanapi -lws2_32 -lmsvfw32 -lavicap32
    }

    PRIVATE_HEADERS += qscreensaver_win_p.h \
                       qdeviceinfo_win_p.h \
                       qstorageinfo_win_p.h \
                       qbatteryinfo_win_p.h \
                       qnetworkinfo_win_p.h

    SOURCES += qscreensaver_win.cpp \
               qdeviceinfo_win.cpp \
               qstorageinfo_win.cpp \
               qbatteryinfo_win.cpp \
               qnetworkinfo_win.cpp
}

linux-*: !simulator: {
    PRIVATE_HEADERS += qdeviceinfo_linux_p.h \
                       qdisplayinfo_linux_p.h \
                       qstorageinfo_linux_p.h \
                       qbatteryinfo_linux_p.h \
                       qnetworkinfo_linux_p.h \
                       qscreensaver_linux_p.h \
                       qdeviceprofile_linux_p.h

    SOURCES += qdeviceinfo_linux.cpp \
               qdisplayinfo_linux.cpp \
               qstorageinfo_linux.cpp \
               qbatteryinfo_linux.cpp \
               qnetworkinfo_linux.cpp \
               qscreensaver_linux.cpp \
               qdeviceprofile_linux.cpp

    x11|contains(config_test_x11, yes): {
        CONFIG += link_pkgconfig
        PKGCONFIG += x11
    } else: {
        DEFINES += QT_NO_X11
    }

    contains(QT_CONFIG, jsondb): {
        QT +=  jsondb
        PRIVATE_HEADERS += qjsondbwrapper_p.h
        SOURCES += qjsondbwrapper.cpp
    } else: {
        DEFINES += QT_NO_JSONDB QT_NO_MTLIB
    }

    mtlib|contains(config_test_mtlib, yes): {
        CONFIG += link_pkgconfig
        PKGCONFIG += mt-client
    } else: {
        DEFINES += QT_NO_MTLIB
    }

    contains(config_test_bluez, yes): {
        CONFIG += link_pkgconfig
        PKGCONFIG += bluez
    } else: {
        DEFINES += QT_NO_BLUEZ
    }

    contains(QT_CONFIG, sfw_netreg) {
        QT += serviceframework
        PRIVATE_HEADERS += qnetworkservicewrapper_p.h
        SOURCES += qnetworkservicewrapper.cpp
    } else {
        DEFINES += QT_NO_SFW_NETREG
    }

    contains(QT_CONFIG, dbus): {
        contains(config_test_ofono, yes) : !contains(QT_CONFIG, sfw_netreg) {
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
    } else {
        DEFINES += QT_NO_OFONO QT_NO_UDISKS
    }

    contains(config_test_udev, yes) {
        CONFIG += link_pkgconfig
        PKGCONFIG += udev
        LIBS += -ludev
        PRIVATE_HEADERS += qudevwrapper_p.h
        SOURCES += qudevwrapper.cpp
    } else {
        DEFINES += QT_NO_UDEV
    }

    contains(config_test_libsysinfo, yes): {
        CONFIG += link_pkgconfig
        PKGCONFIG += sysinfo
        LIBS += -lsysinfo
    } else: {
        DEFINES += QT_NO_LIBSYSINFO
    }
}

simulator {
    QT += simulator
    DEFINES += QT_SIMULATOR
    PRIVATE_HEADERS += qsysteminfodata_simulator_p.h \
                       qsysteminfobackend_simulator_p.h \
                       qsysteminfoconnection_simulator_p.h \
                       qsysteminfo_simulator_p.h


    SOURCES += qsysteminfodata_simulator.cpp \
               qsysteminfobackend_simulator.cpp \
               qsysteminfoconnection_simulator.cpp \
               qsysteminfo_simulator.cpp


    linux-*: {
        PRIVATE_HEADERS += qdisplayinfo_linux_p.h \
                           qscreensaver_linux_p.h \
                           qdeviceprofile_linux_p.h \
                           qstorageinfo_linux_p.h

        SOURCES += qdisplayinfo_linux.cpp \
                   qscreensaver_linux.cpp \
                   qdeviceprofile_linux.cpp \
                   qstorageinfo_linux.cpp

        x11|contains(config_test_x11, yes): {
            CONFIG += link_pkgconfig
            PKGCONFIG += x11
        } else: {
            DEFINES += QT_NO_X11
        }

        contains(QT_CONFIG, jsondb): {
            QT +=  jsondb
            PRIVATE_HEADERS += qjsondbwrapper_p.h \
                               qdeviceinfo_linux_p.h

            SOURCES += qjsondbwrapper.cpp \
                       qdeviceinfo_linux.cpp
        } else: {
            DEFINES += QT_NO_JSONDB QT_NO_MTLIB
        }

        mtlib|contains(config_test_mtlib, yes): {
            CONFIG += link_pkgconfig
            PKGCONFIG += mt-client
        } else: {
            DEFINES += QT_NO_MTLIB
        }

        contains(config_test_bluez, yes): {
            CONFIG += link_pkgconfig
            PKGCONFIG += bluez
        } else: {
            DEFINES += QT_NO_BLUEZ
        }

        contains(QT_CONFIG, sfw_netreg) {
            QT += serviceframework
            PRIVATE_HEADERS += qnetworkservicewrapper_p.h \
                               qnetworkinfo_linux_p.h

            SOURCES += qnetworkservicewrapper.cpp \
                       qnetworkinfo_linux.cpp
        } else {
            DEFINES += QT_NO_SFW_NETREG
        }

        contains(QT_CONFIG, dbus): {
            contains(config_test_ofono, yes) : !contains(QT_CONFIG, sfw_netreg) {
            QT += dbus
            PRIVATE_HEADERS += qofonowrapper_p.h \
                               qnetworkinfo_linux_p.h

            SOURCES += qofonowrapper.cpp \
                       qnetworkinfo_linux.cpp
            } else {
                DEFINES += QT_NO_OFONO
            }
            contains(config_test_udisks, yes): {
                QT += dbus
            } else: {
                DEFINES += QT_NO_UDISKS
            }
        } else {
            DEFINES += QT_NO_OFONO QT_NO_UDISKS
        }

        contains(config_test_udev, yes) {
            CONFIG += link_pkgconfig
            PKGCONFIG += udev
            LIBS += -ludev
            PRIVATE_HEADERS += qudevwrapper_p.h
            SOURCES += qudevwrapper.cpp
        } else {
            DEFINES += QT_NO_UDEV
        }

        contains(config_test_libsysinfo, yes): {
            CONFIG += link_pkgconfig
            PKGCONFIG += sysinfo
            LIBS += -lsysinfo
        } else: {
            DEFINES += QT_NO_LIBSYSINFO
        }
    }
}

HEADERS = qtsysteminfoversion.h $$PUBLIC_HEADERS $$PRIVATE_HEADERS
