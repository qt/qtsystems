TEMPLATE = app
TARGET = qml-deviceinfo
QT += quick
SOURCES = main.cpp

app.files = \
    $$files(*.qml)

target.path = $$[QT_INSTALL_EXAMPLES]/systeminfo/qml-deviceinfo
app.path = $$[QT_INSTALL_EXAMPLES]/systeminfo/qml-deviceinfo
INSTALLS += target app

RESOURCES += \
    qml-deviceinfo.qrc
