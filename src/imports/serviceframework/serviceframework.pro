TARGET  = declarative_serviceframework
TARGETPATH = QtServiceFramework

include(qserviceframeworkimport.pri)

target.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH
DESTDIR = $$QT.serviceframework.imports/$$TARGETPATH

qmldir.files += $$PWD/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$TARGETPATH

INSTALLS += qmldir target

QT += qml serviceframework

HEADERS += qdeclarativeservice_p.h

SOURCES += qdeclarativeservice.cpp \
           serviceframework.cpp 

jsondb|contains(QT_CONFIG, jsondb): {
    mtlib|contains(config_test_mtlib, yes): {
        DEFINES += QT_NO_DBUS QT_MTCLIENT_PRESENT
    }
}
