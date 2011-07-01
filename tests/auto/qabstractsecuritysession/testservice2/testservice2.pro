TEMPLATE      = lib
CONFIG       += plugin

HEADERS       = testserviceplugin.h testservice.h testserviceinterface.h
SOURCES       = testserviceplugin.cpp
TARGET        = tst_sfw_testservice2plugin
DESTDIR = ../plugins

QT += serviceframework

symbian {
    load(data_caging_paths)
    pluginDep.sources = tst_sfw_testservice2plugin.dll
    pluginDep.path = $$QT_PLUGINS_BASE_DIR/plugins
	
    DEPLOYMENT += pluginDep
	
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = ALL -TCB
}
