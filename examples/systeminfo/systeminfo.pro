TEMPLATE = subdirs

SUBDIRS += \
    qml-deviceinfo \
    qml-battery

linux-*: !simulator: {
    SUBDIRS += \
        qml-inputinfo \
        inputinfo
}
#qtHaveModule(widgets): SUBDIRS += \

OTHER_FILES = stub.h
