TEMPLATE = subdirs
CONFIG  += ordered

!android: !ios: !blackberry {
    !without-publishsubscribe: SUBDIRS += publishsubscribe
    !without-serviceframework: SUBDIRS += serviceframework
    !without-systeminfo: SUBDIRS += systeminfo

    SUBDIRS += tools

    qtHaveModule(quick): SUBDIRS += imports
}
