TEMPLATE = subdirs

!macx:!without-systeminfo: SUBDIRS += systeminfo
!boot2qt:!without-publishsubscribe: SUBDIRS += publishsubscribe
!macx:!boot2qt:!without-serviceframework: SUBDIRS += serviceframework
