TEMPLATE = app
QT += publishsubscribe widgets
TARGET = publish-subscribe

HEADERS = publisherdialog.h \
          subscriberdialog.h

SOURCES = main.cpp \
          publisherdialog.cpp \
          subscriberdialog.cpp

FORMS = publisherdialog.ui \
            subscriberdialog.ui
