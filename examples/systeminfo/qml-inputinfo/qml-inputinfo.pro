TEMPLATE = app
TARGET = qml-inputinfo
QT += quick
SOURCES = main.cpp

app.files = \
    $$files(*.qml) \

target.path = $$[QT_INSTALL_EXAMPLES]/systeminfo/qml-inputinfo
app.path = $$[QT_INSTALL_EXAMPLES]/systeminfo/qml-inputinfo
INSTALLS += target app

RESOURCES += \
    qml-inputinfo.qrc
