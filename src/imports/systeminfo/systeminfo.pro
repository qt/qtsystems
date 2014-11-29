QT += qml systeminfo
QT -= gui

HEADERS += \
    qdeclarativedeviceinfo_p.h \
    qdeclarativenetworkinfo_p.h

SOURCES += \
    qdeclarativedeviceinfo.cpp \
    qdeclarativenetworkinfo.cpp \
    qsysteminfo.cpp

linux-*: !simulator: {
    HEADERS += \
        qdeclarativeinputdevicemodel_p.h
    SOURCES += \
        qdeclarativeinputdevicemodel.cpp \
}
load(qml_plugin)
