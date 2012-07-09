QT += qml publishsubscribe core-private

SOURCES += publishsubscribe.cpp \
           qdeclarativevaluespacepublisher.cpp \
           qdeclarativevaluespacepublishermetaobject.cpp \
           qdeclarativevaluespacesubscriber.cpp

HEADERS += qdeclarativevaluespacepublisher_p.h \
           qdeclarativevaluespacepublishermetaobject_p.h \
           qdeclarativevaluespacesubscriber_p.h

load(qml_plugin)
