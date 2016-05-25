TEMPLATE = app
TARGET = qml-battery
QT += quick
SOURCES = main.cpp

app.files = \
    $$files(*.qml) \

target.path = $$[QT_INSTALL_EXAMPLES]/systeminfo/qml-battery
app.path = $$[QT_INSTALL_EXAMPLES]/systeminfo/qml-battery
INSTALLS += target app

RESOURCES += \
    qml-battery.qrc
