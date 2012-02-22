load(qt_module)

TARGET = QtServiceFramework
QPRO_PWD = $PWD

CONFIG += module
MODULE_PRI = ../../modules/qt_serviceframework.pri

QT = core sql

DEFINES += QT_BUILD_SFW_LIB QT_MAKEDLL


load(qt_module_config)

jsondb|contains(QT_CONFIG, jsondb): {
    mtlib|contains(config_test_mtlib, yes): {
        DEFINES += QT_NO_DBUS QT_ADDON_JSONDB_LIB QT_MTCLIENT_PRESENT
        QT += jsondbcompat
    }
    !no_wayland: {
        DEFINES += QT_WAYLAND_PRESENT
    }
    else: {
        error(Config option no_wayland is no longer valid in this configuration)
    }
    no_security: {
        warning(SERVICE FRAMEWORK SECURITY IS DISABLED)
        DEFINES += QT_DISABLE_SECURITY
    }
}

include(ipc/ipc.pri)

PUBLIC_HEADERS += qservice.h \
    qservicemanager.h \
    qserviceplugininterface.h \
    qserviceinterfacedescriptor.h \
    qservicefilter.h \
    qremoteserviceregister.h \
    qserviceclientcredentials.h
PRIVATE_HEADERS += servicemetadata_p.h \
    qserviceinterfacedescriptor_p.h \
    dberror_p.h \
    qserviceclientcredentials_p.h
SOURCES += servicemetadata.cpp \
    qservicemanager.cpp \
    qserviceplugininterface.cpp \
    qserviceinterfacedescriptor.cpp \
    qservicefilter.cpp \
    dberror.cpp \
    qremoteserviceregister.cpp \
    qserviceclientcredentials.cpp

contains(DEFINES, QT_ADDON_JSONDB_LIB): {
    SOURCES += databasemanager_jsondb.cpp
    PRIVATE_HEADERS += databasemanager_jsondb_p.h
} else {
    SOURCES += servicedatabase.cpp \
        databasemanager.cpp
    PRIVATE_HEADERS += servicedatabase_p.h \
        databasemanager_p.h \
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
