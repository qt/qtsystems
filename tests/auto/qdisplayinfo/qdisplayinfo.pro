include(../auto.pri)

INCLUDEPATH += $$SOURCE_TREE/src/systeminfo
LIBS        += -L$$BUILD_TREE/lib -lQtSystemInfo

SOURCES += tst_qdisplayinfo.cpp
