TEMPLATE = subdirs

#!without-sericeframework: SUBDIRS += serviceframework

#!without-publishsubscribe: SUBDIRS += publishsubscribe
!macx:!without-systeminfo: SUBDIRS += systeminfo


# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]
INSTALLS += sources

maemo5: CONFIG += qt_example
