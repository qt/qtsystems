TEMPLATE = app

INCLUDEPATH += $$[QT_INSTALL_PREFIX]/opt/mt/include
INCLUDEPATH += /opt/mt/include

PKGCONFIG += wayland

TARGET = wayland

SOURCES += main.cpp
