TARGET  = declarative_serviceframework
TARGETPATH = QtServiceFramework

include(qserviceframeworkimport.pri)

target.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH
DESTDIR = $$QT.serviceframework.imports/$$TARGETPATH

qmldir.files += $$PWD/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$TARGETPATH

INSTALLS += qmldir target

QT += qml serviceframework

HEADERS += qdeclarativeservice_p.h \
           qdeclarativeserviceold_p.h

SOURCES += qdeclarativeservice.cpp \
           qdeclarativeserviceold.cpp \
           serviceframework.cpp 
