TARGET = QtServiceFramework
QT = core

sfwdebug {
    DEFINES += QT_SFW_IPC_DEBUG
    QT_PRIVATE += network
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
    qserviceclientcredentials_p.h \
    qservicedebuglog_p.h

SOURCES += servicemetadata.cpp \
    qservicemanager.cpp \
    qserviceplugininterface.cpp \
    qserviceinterfacedescriptor.cpp \
    qservicefilter.cpp \
    dberror.cpp \
    qremoteserviceregister.cpp \
    qserviceclientcredentials.cpp \
    qservicedebuglog.cpp \
    qserviceoperations.cpp \
    qservicereply.cpp \
    qservicerequest.cpp

QT_FOR_PRIVATE += sql
SOURCES += servicedatabase.cpp \
    databasemanager.cpp
PRIVATE_HEADERS += servicedatabase_p.h \
    databasemanager_p.h \

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS \
    qserviceoperations_p.h \
    qservicereply.h \
    qservicereply_p.h \
    qservicerequest_p.h

QMAKE_DOCS = $$PWD/../../doc/config/serviceframework/qtserviceframework.qdocconf

load(qt_module)
