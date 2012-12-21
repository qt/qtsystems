TEMPLATE = subdirs

SUBDIRS += \
    qml-deviceinfo \
    qml-storageinfo \
    qml-battery

#qtHaveModule(widgets): SUBDIRS += \

OTHER_FILES = stub.h
