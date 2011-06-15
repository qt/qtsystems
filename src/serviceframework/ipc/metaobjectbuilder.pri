#check version for 4.7 ...
contains(QT_MAJOR_VERSION, 4):lessThan(QT_MINOR_VERSION, 8) {
  OBJECTBUILDER_INCLUDEPATH += ipc/metaobjectbuilder47
  OBJECTBUILDER_DEPENDPATH += ipc/metaobjectbuilder47
  OBJECTBUILDER_HEADERS += ipc/metaobjectbuilder47/qmetaobjectbuilder_p.h
  OBJECTBUILDER_SOURCES += ipc/metaobjectbuilder47/qmetaobjectbuilder.cpp
} else {
  OBJECTBUILDER_INCLUDEPATH += ipc/metaobjectbuilder
  OBJECTBUILDER_DEPENDPATH += ipc/metaobjectbuilder
  OBJECTBUILDER_HEADERS += ipc/metaobjectbuilder/qmetaobjectbuilder_p.h
  OBJECTBUILDER_SOURCES += ipc/metaobjectbuilder/qmetaobjectbuilder.cpp
}
