TARGET  = declarative_publishsubscribe
TARGETPATH = Qt/publishsubscribe

include(qpublishsubscribeimport.pri)
target.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH
DESTDIR = $$QT.publishsubscribe.imports/$$TARGETPATH
INSTALLS += target

qmldir.files += $$PWD/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$TARGETPATH
INSTALLS += qmldir

QT += declarative publishsubscribe

SOURCES += publishsubscribe.cpp \
           qdeclarativevaluespacepublisher.cpp \
           qdeclarativevaluespacepublishermetaobject.cpp \
           qdeclarativevaluespacesubscriber.cpp \
           qmetaobjectbuilder.cpp

HEADERS += qdeclarativevaluespacepublisher_p.h \
           qdeclarativevaluespacepublishermetaobject_p.h \
           qdeclarativevaluespacesubscriber_p.h \
           qmetaobjectbuilder_p.h
