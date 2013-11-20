QT += qml systeminfo
QT -= gui

HEADERS += \
    qdeclarativedeviceinfo_p.h \
    qdeclarativenetworkinfo_p.h \
    qdeclarativestorageinfo_p.h

SOURCES += \
    qdeclarativedeviceinfo.cpp \
    qdeclarativenetworkinfo.cpp \
    qdeclarativestorageinfo.cpp \
    qsysteminfo.cpp

load(qml_plugin)
