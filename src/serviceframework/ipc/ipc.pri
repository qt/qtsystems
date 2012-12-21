INCLUDEPATH += ipc
win32 {
    INCLUDEPATH += .
}

QT += core-private

isEmpty(SFW_BACKEND) {
    qtHaveModule(dbus) {
        SFW_BACKEND = dbus
    } else {
        linux {
            SFW_BACKEND = unix
        } else {
            SFW_BACKEND = localsocket
        }
    }
}

equals(SFW_BACKEND,unix) {
    DEFINES += SFW_USE_UNIX_BACKEND
    PRIVATE_HEADERS += ipc/qremoteserviceregister_unix_p.h \
        ipc/objectendpoint_p.h
    SOURCES += ipc/qremoteserviceregister_unix_p.cpp \
        ipc/objectendpoint.cpp
} else:equals(SFW_BACKEND,dbus) {
    DEFINES += SFW_USE_DBUS_BACKEND
    QT_FOR_PRIVATE += dbus \
        network
    PRIVATE_HEADERS += ipc/qremoteserviceregister_dbus_p.h \
        ipc/objectendpoint_dbus_p.h \
        ipc/qservicemetaobject_dbus_p.h
    SOURCES += ipc/qremoteserviceregister_dbus_p.cpp \
        ipc/objectendpoint_dbus.cpp \
        ipc/qservicemetaobject_dbus.cpp
} else:equals(SFW_BACKEND,localsocket) {
    DEFINES += SFW_USE_LOCALSOCKET_BACKEND
    QT_FOR_PRIVATE += network
    PRIVATE_HEADERS += ipc/qremoteserviceregister_ls_p.h \
        ipc/objectendpoint_p.h
    SOURCES += ipc/qremoteserviceregister_ls_p.cpp \
        ipc/objectendpoint.cpp
} else {
    error("Unkown SFW_BACKEND $$SFW_BACKEND")
}

PRIVATE_HEADERS += ipc/qslotinvoker_p.h \
    ipc/qsignalintercepter_p.h \
    ipc/instancemanager_p.h \
    ipc/qservicepackage_p.h \
    ipc/proxyobject_p.h \
    ipc/ipcendpoint_p.h \
    ipc/qremoteserviceregister_p.h \
    ipc/qremoteserviceregisterentry_p.h

SOURCES += ipc/qslotinvoker.cpp \
    ipc/qsignalintercepter.cpp \
    ipc/instancemanager.cpp \
    ipc/qservicepackage.cpp \
    ipc/proxyobject.cpp \
    ipc/ipcendpoint.cpp \
    ipc/qremoteserviceregister_p.cpp

OTHER_FILES += \
    ipc/json-schema.txt
