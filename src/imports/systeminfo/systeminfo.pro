QT += qml systeminfo
QT -= gui

HEADERS += \
    qdeclarativedeviceinfo_p.h \
    qdeclarativenetworkinfo_p.h

SOURCES += \
    qdeclarativedeviceinfo.cpp \
    qdeclarativenetworkinfo.cpp \
    qsysteminfo.cpp

load(qml_plugin)
