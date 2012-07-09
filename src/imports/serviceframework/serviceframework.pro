QT += qml serviceframework

HEADERS += qdeclarativeservice_p.h \
           qdeclarativeserviceold_p.h

SOURCES += qdeclarativeservice.cpp \
           qdeclarativeserviceold.cpp \
           serviceframework.cpp 

load(qml_plugin)
