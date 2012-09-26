QT += qml systeminfo
QT -= gui

HEADERS += \
    qdeclarativebatteryinfo_p.h \
    qdeclarativedeviceinfo_p.h \
    qdeclarativedisplayinfo_p.h \
    qdeclarativenetworkinfo_p.h \
    qdeclarativestorageinfo_p.h \
    qdeclarativedeviceprofile_p.h

SOURCES += \
    qdeclarativebatteryinfo.cpp \
    qdeclarativedeviceinfo.cpp \
    qdeclarativedisplayinfo.cpp \
    qdeclarativenetworkinfo.cpp \
    qdeclarativestorageinfo.cpp \
    qdeclarativedeviceprofile.cpp \
    qsysteminfo.cpp

load(qml_plugin)
