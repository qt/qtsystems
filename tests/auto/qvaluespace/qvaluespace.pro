include(../auto.pri)

QMAKE_LIBS += -Wl,-rpath,$${QT.publishsubscribe.libs}

QT += publishsubscribe

!contains(config_test_gconf, yes) {
    DEFINES += QT_NO_GCONFLAYER
}

!contains(config_test_contextkit, yes) {
    DEFINES += QT_NO_CONTEXTKIT
}

SOURCES += tst_qvaluespace.cpp
