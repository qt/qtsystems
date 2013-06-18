include(../../auto.pri)

QT += publishsubscribe

!config_gconf {
    DEFINES += QT_NO_GCONFLAYER
}

SOURCES += tst_qvaluespace.cpp
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
