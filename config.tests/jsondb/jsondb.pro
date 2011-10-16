TEMPLATE = app

INCLUDEPATH += $$[QT_INSTALL_PREFIX]/opt/mt/include
INCLUDEPATH += /opt/mt/include

PKGCONFIG += mtcore QtAddOnJsonDb

QT += jsondb jsondb-private

TARGET = jsondb

SOURCES += main.cpp
