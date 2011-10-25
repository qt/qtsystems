TEMPLATE = subdirs
CONFIG  += ordered

!without-systeminfo: SUBDIRS += systeminfo
!without-publishsubscribe: SUBDIRS += publishsubscribe
!without-serviceframework: SUBDIRS += serviceframework

SUBDIRS += imports tools
