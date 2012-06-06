TEMPLATE = subdirs

!contains(QT_CONFIG, no-widgets) {
    SUBDIRS += \
        battery-charge \
        publish-subscribe
}
