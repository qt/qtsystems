load(qt_module)

TARGET = QtServiceFramework
QPRO_PWD = $PWD

CONFIG += module
MODULE_PRI = ../../modules/qt_serviceframework.pri

QT = core sql script

DEFINES += QT_BUILD_SFW_LIB QT_MAKEDLL


load(qt_module_config)

jsondb|contains(QT_CONFIG, jsondb): {
    mtcore|contains(config_test_mtcore, yes): {
        DEFINES += QT_NO_DBUS QT_ADDON_JSONDB_LIB
        CONFIG += link_pkgconfig
        PKGCONFIG += mtcore
        QT += jsondb jsondb-private
    }
    !no_wayland: {
        DEFINES += QT_WAYLAND_PRESENT
    }
    else: {
        error(Config option no_wayland is no longer valid in this configuration)
    }
}

include(ipc/ipc.pri)

PUBLIC_HEADERS += qservice.h \
    qservicemanager.h \
    qserviceplugininterface.h \
    qserviceinterfacedescriptor.h \
    qservicefilter.h \
    qremoteserviceregister.h
PRIVATE_HEADERS += servicemetadata_p.h \
    qserviceinterfacedescriptor_p.h \
    dberror_p.h
SOURCES += servicemetadata.cpp \
    qservicemanager.cpp \
    qserviceplugininterface.cpp \
    qserviceinterfacedescriptor.cpp \
    qservicefilter.cpp \
    dberror.cpp \
    qremoteserviceregister.cpp
symbian {
    contains(S60_VERSION, 5.2)|contains(MOBILITY_SD_MCL_BUILD, yes){
        DEFINES += SYMBIAN_EMULATOR_SUPPORTS_PERPROCESS_WSD
    }
    INCLUDEPATH += ./databasemanagerserver_symbian
    PRIVATE_HEADERS += databasemanager_symbian_p.h
    SOURCES += databasemanager_symbian.cpp
    TARGET.CAPABILITY = ALL \
        -TCB
    TARGET.UID3 = 0x2002AC84
    QtServiceFrameworkDeployment.sources = QtServiceFramework.dll \
        qsfwdatabasemanagerserver.exe
    QtServiceFrameworkDeployment.path = /sys/bin
    DEPLOYMENT += QtServiceFrameworkDeployment
} else: contains(DEFINES, QT_ADDON_JSONDB_LIB): {
    SOURCES += databasemanager_jsondb.cpp
    PRIVATE_HEADERS += databasemanager_jsondb_p.h
} else {
    SOURCES += servicedatabase.cpp \
        databasemanager.cpp
    PRIVATE_HEADERS += servicedatabase_p.h \
        databasemanager_p.h \
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
