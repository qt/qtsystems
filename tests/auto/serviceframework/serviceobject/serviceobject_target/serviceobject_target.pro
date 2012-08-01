# QML tests in this directory must not depend on an OpenGL context.
# QML tests that do require an OpenGL context must go in ../declarative_ui.

TEMPLATE = app
TARGET = serviceobject_target
SOURCES += serviceobject_target.cpp

QT += serviceframework

OTHER_FILES += serviceobject.xml

HEADERS += \
    serviceobject_target.h

RESOURCES += \
    serviceobject.qrc
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
