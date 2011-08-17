load(qt_module)

TARGET = QtPublishSubscribe
QPRO_PWD = $PWD

CONFIG += module
MODULE_PRI = ../../modules/qt_publishsubscribe.pri

QT = core

DEFINES += QT_BUILD_PUBLISHSUBSCRIBE_LIB QT_MAKEDLL

load(qt_module_config)

PUBLIC_HEADERS = qvaluespace.h \
                 qvaluespacepublisher.h \
                 qvaluespacesubscriber.h

PRIVATE_HEADERS = qpublishsubscribe_p.h \
                  qvaluespace_p.h \
                  qvaluespacemanager_p.h \
                  qvaluespacesubscriber_p.h

SOURCES = qvaluespace.cpp \
          qvaluespacemanager.cpp \
          qvaluespacepublisher.cpp \
          qvaluespacesubscriber.cpp

unix {
    linux-* {
        contains(config_test_gconf, yes) {
            PRIVATE_HEADERS += gconfitem_p.h \
                               gconflayer_p.h

            SOURCES += gconflayer.cpp \
                       gconfitem.cpp

            CONFIG += link_pkgconfig
            PKGCONFIG += gobject-2.0 gconf-2.0
        } else {
            DEFINES += QT_NO_GCONFLAYER
        }

        contains(config_test_jsondb, yes) {
            QT += jsondb jsondb-private
            PRIVATE_HEADERS += jsondblayer_p.h
            SOURCES += jsondblayer.cpp
        } else {
            DEFINES += QT_NO_JSONDBLAYER
        }

        contains(QT_CONFIG, dbus): {
            contains(config_test_contextkit, yes) {
                QT += dbus

                PRIVATE_HEADERS += contextkitlayer_p.h
                SOURCES += contextkitlayer.cpp

                CONFIG += link_pkgconfig
                PKGCONFIG += contextsubscriber-1.0 contextprovider-1.0
            } else {
                DEFINES += QT_NO_CONTEXTKIT
            }
        } else {
            DEFINES += QT_NO_CONTEXTKIT
        }
    }
}

win32: {
    PRIVATE_HEADERS += qsystemreadwritelock_p.h \
                       registrylayer_win_p.h
    SOURCES += qsystemreadwritelock_win.cpp \
               registrylayer_win.cpp

    LIBS += -ladvapi32
}

HEADERS = qtpublishsubscribeversion.h $$PUBLIC_HEADERS $$PRIVATE_HEADERS
