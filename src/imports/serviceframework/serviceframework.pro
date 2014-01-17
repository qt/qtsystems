QT += qml serviceframework
QT -= gui

HEADERS += qdeclarativeservice_p.h \
           qdeclarativeserviceold_p.h

SOURCES += qdeclarativeservice.cpp \
           qdeclarativeserviceold.cpp \
           serviceframework.cpp

load(qml_plugin)
