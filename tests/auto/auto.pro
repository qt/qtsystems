TEMPLATE = subdirs

!without-publishsubscribe: SUBDIRS += publishsubscribe
!macx:!without-serviceframework: SUBDIRS += serviceframework
!macx:!without-systeminfo: SUBDIRS += systeminfo

#SUBDIRS += cmake
