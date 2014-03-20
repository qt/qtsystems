TEMPLATE = subdirs

!macx:!without-systeminfo: SUBDIRS += systeminfo
!without-publishsubscribe: SUBDIRS += publishsubscribe
!macx:!without-serviceframework: SUBDIRS += serviceframework
