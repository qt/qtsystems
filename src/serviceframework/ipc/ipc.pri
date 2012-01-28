INCLUDEPATH += ipc

QT += core-private

!jsondb:!contains(config_test_jsondb, yes):contains(QT_CONFIG,dbus) {
    DEFINES += SFW_USE_DBUS_BACKEND
    QT += dbus \
        network
    PRIVATE_HEADERS += ipc/qremoteserviceregister_dbus_p.h \
        ipc/objectendpoint_dbus_p.h \
        ipc/qservicemetaobject_dbus_p.h
    SOURCES += ipc/qremoteserviceregister_dbus_p.cpp \
        ipc/objectendpoint_dbus.cpp \
        ipc/qservicemetaobject_dbus.cpp
} else {
    QT += network
    PRIVATE_HEADERS += ipc/qremoteserviceregister_ls_p.h \
        ipc/objectendpoint_p.h
    SOURCES += ipc/qremoteserviceregister_ls_p.cpp \
        ipc/objectendpoint.cpp
}

PRIVATE_HEADERS += ipc/qslotinvoker_p.h \
    ipc/qsignalintercepter_p.h \
    ipc/instancemanager_p.h \
    ipc/qservicepackage_p.h \
    ipc/qsecuritypackage_p.h \
    ipc/proxyobject_p.h \
    ipc/ipcendpoint_p.h \
    ipc/qremoteserviceregister_p.h \
    ipc/qremoteserviceregisterentry_p.h

SOURCES += ipc/qslotinvoker.cpp \
    ipc/qsignalintercepter.cpp \
    ipc/instancemanager.cpp \
    ipc/qservicepackage.cpp \
    ipc/qsecuritypackage.cpp \
    ipc/proxyobject.cpp \
    ipc/ipcendpoint.cpp \
    ipc/qremoteserviceregister_p.cpp

OTHER_FILES += \
    ipc/json-schema.txt
