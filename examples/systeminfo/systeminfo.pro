TEMPLATE = subdirs

SUBDIRS += \
    qml-deviceinfo \
    qml-storageinfo \
    qml-battery


#!isEmpty(QT.widgets.name):SUBDIRS += \
OTHER_FILES = stub.h
