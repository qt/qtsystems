TEMPLATE = subdirs
CONFIG  += ordered

!android: !ios: !blackberry: !wince {
    !without-publishsubscribe: SUBDIRS += publishsubscribe
    !macx:!without-serviceframework: SUBDIRS += serviceframework
    !macx:!without-systeminfo: SUBDIRS += systeminfo

    SUBDIRS += tools

    qtHaveModule(quick): SUBDIRS += imports
}
