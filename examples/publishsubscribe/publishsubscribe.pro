TEMPLATE = subdirs

qtHaveModule(widgets) {
    SUBDIRS += \
        battery-charge \
        publish-subscribe
}
