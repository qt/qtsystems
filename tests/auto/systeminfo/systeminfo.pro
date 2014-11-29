TEMPLATE = subdirs

SUBDIRS = \
    qdeviceinfo \
    qscreensaver \
    qbatteryinfo

linux-*: !simulator: {
    SUBDIRS += \
    qinputdeviceinfo
}
