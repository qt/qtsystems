TEMPLATE = subdirs
CONFIG  += ordered

!without-publishsubscribe: SUBDIRS += publishsubscribe
!without-serviceframework: SUBDIRS += serviceframework
!without-systeminfo: SUBDIRS += systeminfo # systeminfo has dependencies to serviceframework, thus build it after sfw

SUBDIRS += tools

qtHaveModule(quick): SUBDIRS += imports
