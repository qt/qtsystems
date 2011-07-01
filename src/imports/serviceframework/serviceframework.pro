TARGET  = declarative_serviceframework
TARGETPATH = Qt/serviceframework

include(qserviceframeworkimport.pri)

target.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH
DESTDIR = $$QT.serviceframework.imports/$$TARGETPATH

qmldir.files += $$PWD/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$TARGETPATH

INSTALLS += qmldir target

QT += declarative serviceframework

HEADERS += qdeclarativeservice_p.h

SOURCES += qdeclarativeservice.cpp \
           serviceframework.cpp 

symbian {
    TARGET.EPOCALLOWDLLDATA=1
    TARGET.CAPABILITY = All -Tcb
    TARGET.UID3 = 0x20021323
    load(armcc_warnings)
    # Specifies what files shall be deployed: the plugin itself and the qmldir file.
    importFiles.sources = $$DESTDIR/declarative_serviceframework$${QT_LIBINFIX}.dll qmldir 
    importFiles.path = $$QT_IMPORTS_BASE_DIR/$$TARGETPATH
    DEPLOYMENT = importFiles
}
