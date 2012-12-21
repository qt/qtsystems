TARGET = QtSystemInfo
QPRO_PWD = $PWD

QT = core network

PUBLIC_HEADERS = qsysteminfoglobal.h \
                 qdeviceinfo.h \
                 qstorageinfo.h \
                 qscreensaver.h \
                 qbatteryinfo.h \
                 qnetworkinfo.h \
                 qdeviceprofile.h

SOURCES += qdeviceinfo.cpp \
           qstorageinfo.cpp \
           qscreensaver.cpp \
           qbatteryinfo.cpp \
           qnetworkinfo.cpp \
           qdeviceprofile.cpp

qtHaveModule(gui) {
    QT += gui
    PUBLIC_HEADERS += qdisplayinfo.h
    SOURCES += qdisplayinfo.cpp
}

win32: !simulator: {
    contains(CONFIG, release) {
       CONFIG -= console
    }

    win32-msvc*: {
        LIBS += -lUser32 -lGdi32 -lPowrProf -lBthprops -lWs2_32 -lVfw32 -lSetupapi -lIphlpapi -lOle32 -lWbemuuid
    }

    win32-g++*: {
        LIBS += -luser32 -lgdi32 -lpowrprof -lbthprops -lws2_32 -lmsvfw32 -lavicap32 -luuid
    }

    PRIVATE_HEADERS += qscreensaver_win_p.h \
                       qdeviceinfo_win_p.h \
                       qstorageinfo_win_p.h \
                       qbatteryinfo_win_p.h \
                       qnetworkinfo_win_p.h \
                       windows/qwmihelper_win_p.h \
                       qsysteminfoglobal_p.h

    SOURCES += qscreensaver_win.cpp \
               qdeviceinfo_win.cpp \
               qstorageinfo_win.cpp \
               qbatteryinfo_win.cpp \
               qnetworkinfo_win.cpp \
               windows/qwmihelper_win.cpp

       LIBS += \
            -lOle32 \
            -lUser32 \
            -lGdi32 \
            -lIphlpapi \
            -lOleaut32 \
            -lPowrProf \
            -lSetupapi

  win32-g++: {
        LIBS += -luser32 -lgdi32
    }

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
               qscreensaver_linux.cpp

    x11|config_x11 {
        CONFIG += link_pkgconfig
        PKGCONFIG += x11
    } else: {
        DEFINES += QT_NO_X11
    }

    config_bluez {
        CONFIG += link_pkgconfig
        PKGCONFIG += bluez
    } else: {
        DEFINES += QT_NO_BLUEZ
    }

    qtHaveModule(dbus) {
        config_ofono: {
            QT += dbus
            PRIVATE_HEADERS += qofonowrapper_p.h
            SOURCES += qofonowrapper.cpp
        } else {
            DEFINES += QT_NO_OFONO
        }

        config_udisks {
            QT_PRIVATE += dbus
        } else: {
            DEFINES += QT_NO_UDISKS
        }
    } else {
        DEFINES += QT_NO_OFONO QT_NO_UDISKS
    }

    config_udev {
        CONFIG += link_pkgconfig
        PKGCONFIG += udev
        LIBS += -ludev
        PRIVATE_HEADERS += qudevwrapper_p.h
        SOURCES += qudevwrapper.cpp
    } else {
        DEFINES += QT_NO_UDEV
    }

    config_libsysinfo {
        CONFIG += link_pkgconfig
        PKGCONFIG += sysinfo
        LIBS += -lsysinfo
    } else: {
        DEFINES += QT_NO_LIBSYSINFO
    }
}

macx:!simulator {
#CONFIG -= x86_64
QT += core-private
         OBJECTIVE_SOURCES += qbatteryinfo_mac.mm \
                  qdeviceinfo_mac.mm \
                  qdisplayinfo_mac.mm \
                  qnetworkinfo_mac.mm \
                  qscreensaver_mac.mm \
                  qstorageinfo_mac.mm

         PRIVATE_HEADERS += qbatteryinfo_mac_p.h \
                  qdeviceinfo_mac_p.h \
                  qdisplayinfo_mac_p.h \
                  qnetworkinfo_mac_p.h \
                  qscreensaver_mac_p.h \
                  qstorageinfo_mac_p.h

         LIBS += -framework SystemConfiguration \
                -framework Foundation \
                -framework IOKit  \
                -framework QTKit \
                -framework CoreWLAN \
                -framework CoreLocation \
                -framework CoreFoundation \
                -framework ScreenSaver \
                -framework IOBluetooth \
                -framework CoreServices \
                -framework DiskArbitration \
                -framework ApplicationServices
}

simulator {
    QT_PRIVATE += simulator
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
                   qstorageinfo_linux.cpp

        x11|config_x11 {
            CONFIG += link_pkgconfig
            PKGCONFIG += x11
        } else: {
            DEFINES += QT_NO_X11
        }

        config_bluez {
            CONFIG += link_pkgconfig
            PKGCONFIG += bluez
        } else: {
            DEFINES += QT_NO_BLUEZ
        }

        qtHaveModule(dbus) {
            config_ofono: {
            QT += dbus
            PRIVATE_HEADERS += qofonowrapper_p.h \
                               qnetworkinfo_linux_p.h

            SOURCES += qofonowrapper.cpp \
                       qnetworkinfo_linux.cpp
            } else {
                DEFINES += QT_NO_OFONO
            }
            contains(config_test_udisks, yes): {
                QT_PRIVATE += dbus
            } else: {
                DEFINES += QT_NO_UDISKS
            }
        } else {
            DEFINES += QT_NO_OFONO QT_NO_UDISKS
        }

        config_udev {
            CONFIG += link_pkgconfig
            PKGCONFIG += udev
            LIBS += -ludev
            PRIVATE_HEADERS += qudevwrapper_p.h
            SOURCES += qudevwrapper.cpp
        } else {
            DEFINES += QT_NO_UDEV
        }

        config_libsysinfo {
            CONFIG += link_pkgconfig
            PKGCONFIG += sysinfo
            LIBS += -lsysinfo
        } else: {
            DEFINES += QT_NO_LIBSYSINFO
        }
    }
}

QMAKE_DOCS = $$PWD/../../doc/config/systeminfo/qtsysteminfo.qdocconf

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
load(qt_module)

# This must be done after loading qt_module.prf
config_bluez {
    # bluetooth.h is not standards compliant
    contains(QMAKE_CXXFLAGS, -std=c++0x) {
        QMAKE_CXXFLAGS -= -std=c++0x
        QMAKE_CXXFLAGS += -std=gnu++0x
        CONFIG -= c++11
    }
    c++11 {
        CONFIG -= c++11
        QMAKE_CXXFLAGS += -std=gnu++0x
    }
}

# Enable doc submodule doc builds
include (../../doc/config/systeminfo/qtsysteminfo_doc.pri)
