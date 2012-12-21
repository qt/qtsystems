include(../../auto.pri)

QT += publishsubscribe

!config_gconf {
    DEFINES += QT_NO_GCONFLAYER
}

!qtHaveModule(jsondb) {
    DEFINES += QT_NO_JSONDBLAYER
} else {
    QT += jsondb
}

SOURCES += tst_qvaluespace.cpp
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
