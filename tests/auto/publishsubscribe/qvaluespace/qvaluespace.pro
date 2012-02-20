include(../../auto.pri)

QT += publishsubscribe

!contains(config_test_gconf, yes) {
    DEFINES += QT_NO_GCONFLAYER
}

!contains(QT_CONFIG, jsondbcompat) {
    DEFINES += QT_NO_JSONDBLAYER
} else {
    QT += jsondbcompat jsondbcompat-private
}

SOURCES += tst_qvaluespace.cpp
