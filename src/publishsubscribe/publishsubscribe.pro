TARGET = QtPublishSubscribe
QT = core-private

PUBLIC_HEADERS = qpublishsubscribeglobal.h \
                 qvaluespace.h \
                 qvaluespacepublisher.h \
                 qvaluespacesubscriber.h

PRIVATE_HEADERS = qvaluespace_p.h \
                  qvaluespacemanager_p.h \
                  qvaluespacesubscriber_p.h

SOURCES = qvaluespace.cpp \
          qvaluespacemanager.cpp \
          qvaluespacepublisher.cpp \
          qvaluespacesubscriber.cpp

unix {
    linux-* {
        config_gconf {
            PRIVATE_HEADERS += gconfitem_p.h \
                               gconflayer_p.h

            SOURCES += gconflayer.cpp \
                       gconfitem.cpp

            CONFIG += link_pkgconfig
            PKGCONFIG += gobject-2.0 gconf-2.0
        } else {
            DEFINES += QT_NO_GCONFLAYER
        }
    }
} else {
    DEFINES += QT_NO_GCONFLAYER
}

win32: {
    PRIVATE_HEADERS += qsystemreadwritelock_p.h \
                       registrylayer_win_p.h
    SOURCES += qsystemreadwritelock_win.cpp \
               registrylayer_win.cpp

    LIBS += -ladvapi32
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

QMAKE_DOCS = $$PWD/../../doc/config/publishsubscribe/qtpublishsubscribe.qdocconf

load(qt_module)

