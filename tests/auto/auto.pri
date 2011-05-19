TEMPLATE = app
CONFIG += qt debug warn_on console depend_includepath testcase

qtAddLibrary(QtTest)

QMAKE_LIBS += -Wl,-rpath,$$BUILD_TREE/lib
