include(../auto.pri)

QMAKE_LIBS += -Wl,-rpath,$${QT.systeminfo.libs}

QT += systeminfo

SOURCES += tst_qstorageinfo.cpp
