TEMPLATE = subdirs

SERVICEFRAMEWORK = \
           qremoteserviceregister \
           qservicefilter \
           qserviceinterfacedescriptor \
           qservicemanager \
           qservicemanager_ipc \
           qservicemetadata \
           servicedeletion
#           qmetaobjectbuilder #(requires test symbols)
#           servicedatabase    #(requires test symbols)


SYSTEMINFO = \
           qdeviceinfo \
#           qdisplayinfo \
           qstorageinfo \
           qscreensaver \
           qbatteryinfo \

PUBLISHSUBSCRIBE += \
           qvaluespace \

# jsondb|contains(config_test_jsondb, yes): PUBLISHSUBSCRIBE += qvaluespace_jsondb


!without-publishsubscribe: SUBDIRS += $$PUBLISHSUBSCRIBE
!without-systeminfo: SUBDIRS += $$SYSTEMINFO
!without-serviceframework: SUBDIRS += $$SERVICEFRAMEWORK
