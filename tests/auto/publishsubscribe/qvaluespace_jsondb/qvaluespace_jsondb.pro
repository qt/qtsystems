#-------------------------------------------------
#
# Project created by QtCreator 2011-02-10T13:14:47
#
#-------------------------------------------------

QT       += testlib
QT       += jsondb
QT       += core-private
QT       += concurrent

TARGET = tst_testpublishsubscribe
target.path = /usr/bin
INSTALLS += target

#QMAKE_EXTRA_TARGETS = check
#check.depends = $$TARGET
#check.commands = ./$$TARGET

CONFIG   += console
CONFIG   += qtestlib
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/../../../../include/QtPublishSubscribe $$PWD/../../../../src/publishsubscribe

SOURCES += tst_valuespace_jsondb.cpp \
    $$PWD/../../../../src/publishsubscribe/jsondblayer.cpp \
    $$PWD/../../../../src/publishsubscribe/qvaluespace.cpp \
    $$PWD/../../../../src/publishsubscribe/qvaluespacesubscriber.cpp \
    $$PWD/../../../../src/publishsubscribe/qvaluespacepublisher.cpp \
    $$PWD/../../../../src/publishsubscribe/qvaluespacemanager.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    $$PWD/../../../../src/publishsubscribe/jsondblayer_p.h \
    $$PWD/../../../../src/publishsubscribe/qvaluespace_p.h \
    $$PWD/../../../../src/publishsubscribe/qvaluespacesubscriber.h \
    $$PWD/../../../../src/publishsubscribe/qvaluespacesubscriber_p.h \
    $$PWD/../../../../src/publishsubscribe/qvaluespacepublisher.h \
    $$PWD/../../../../src/publishsubscribe/qvaluespacemanager_p.h

#QMAKE_EXTRA_TARGETS = check
#check.depends = $$TARGET
#check.commands = ./$$TARGET

DEFINES += QT_NO_GCONFLAYER
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
