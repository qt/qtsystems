TEMPLATE = subdirs

!boot2qt:!without-publishsubscribe: SUBDIRS += publishsubscribe
!macx:!boot2qt:!without-serviceframework: SUBDIRS += serviceframework
!macx:!without-systeminfo: SUBDIRS += systeminfo

#SUBDIRS += cmake
