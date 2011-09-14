TEMPLATE = subdirs

!without-sericeframework: SUBDIRS += serviceframework

SUBDIRS += publishsubscribe

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]
INSTALLS += sources

symbian: CONFIG += qt_example
maemo5: CONFIG += qt_example
