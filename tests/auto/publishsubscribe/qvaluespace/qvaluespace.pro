include(../../auto.pri)

QT += publishsubscribe

!config_gconf {
    DEFINES += QT_NO_GCONFLAYER
}

!contains(QT_CONFIG, jsondb) {
    DEFINES += QT_NO_JSONDBLAYER
} else {
    QT += jsondb
}

SOURCES += tst_qvaluespace.cpp
