TEMPLATE = app
CONFIG += qt debug warn_on console depend_includepath testcase

qtAddLibrary(QtTest)

QMAKE_LIBS += -Wl,-rpath,$$BUILD_TREE/lib

INCLUDEPATH += $$QT_SYSTEMKIT_SOURCE_TREE/src/systeminfo
LIBS        += -L$$QT_SYSTEMKIT_BUILD_TREE/lib -lQtSystemInfo
