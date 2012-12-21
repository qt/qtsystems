TEMPLATE      = subdirs
SUBDIRS       = \
                voipdialerplugin \
                remotedialerservice \
                dialer

qtHaveModule(widgets) {
    SUBDIRS  += \
                servicebrowser
}

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/statemachine
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS serviceframework.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframework
INSTALLS += target sources
