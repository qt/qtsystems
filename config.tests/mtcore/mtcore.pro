TEMPLATE = app

INCLUDEPATH += $$[QT_INSTALL_PREFIX]/opt/mt/include
INCLUDEPATH += /opt/mt/include

PKGCONFIG += mtcore

TARGET = mtcore

SOURCES += main.cpp
