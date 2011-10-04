TEMPLATE = subdirs
SUBDIRS += qdeviceinfo \
#           qdisplayinfo \
           qstorageinfo \
           qscreensaver \
           qbatteryinfo \
           qvaluespace \
           qremoteserviceregister \
           qservicefilter \
           qserviceinterfacedescriptor \
           qservicemanager \
           qservicemetadata \
           servicedeletion
#           qmetaobjectbuilder #(requires test symbols)
#           servicedatabase    #(requires test symbols)

# contains(config_test_jsondb, yes): SUBDIRS += qvaluespace_jsondb
