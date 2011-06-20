include(../auto.pri)

QMAKE_LIBS += -Wl,-rpath,$${QT.publishsubscribe.libs}

QT += publishsubscribe

!contains(contextkit_enabled, yes) {
    DEFINES += QT_NO_CONTEXTKIT
}

SOURCES += tst_qvaluespace.cpp
