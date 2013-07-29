TEMPLATE = subdirs

SUBDIRS = \
           qremoteserviceregister \
           qservicefilter \
           qserviceinterfacedescriptor \
           qservicemanager \
           qservicemanager_ipc \
           qservicemetadata \
           servicedeletion
#           serviceobject
#           servicedatabase    #(requires test symbols)

win32:SUBDIRS -= \
    qservicemanager_ipc \ # QTBUG-32662
    servicedeletion \ # QTBUG-32667
    qremoteserviceregister # QTBUG-32707
