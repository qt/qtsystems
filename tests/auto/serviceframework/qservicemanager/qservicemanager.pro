QT = core
TEMPLATE = subdirs

SUBDIRS = sampleservice sampleservice2 testservice2
tst_qservicemanager_pro.depends += sampleservice sampleservice2 testservice2

SUBDIRS += tst_qservicemanager.pro
