QT = core
TEMPLATE = subdirs

SUBDIRS = testservice2
tst_qabstractsecuritysession_pro.depends += sampleservice2 testservice2

SUBDIRS += tst_qabstractsecuritysession.pro

CONFIG += parallel_test
