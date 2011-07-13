TEMPLATE      = subdirs
SUBDIRS       = \
                remotedialerservice \
                dialer

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/statemachine
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS serviceframework.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/qtsystems/serviceframeeork
INSTALLS += target sources
