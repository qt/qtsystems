QT += qml systeminfo
QT -= gui

HEADERS += \
    qdeclarativebatteryinfo_p.h \
    qdeclarativedeviceinfo_p.h \
    qdeclarativenetworkinfo_p.h \
    qdeclarativestorageinfo_p.h

SOURCES += \
    qdeclarativebatteryinfo.cpp \
    qdeclarativedeviceinfo.cpp \
    qdeclarativenetworkinfo.cpp \
    qdeclarativestorageinfo.cpp \
    qsysteminfo.cpp

load(qml_plugin)
