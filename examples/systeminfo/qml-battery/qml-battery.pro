TEMPLATE = app
TARGET = qml-battery
QT += quick
SOURCES = main.cpp

app.files = \
    $$files(*.qml) \

target.path = $$[QT_INSTALL_EXAMPLES]/sysinfo/qml-battery
app.path = $$[QT_INSTALL_EXAMPLES]/sysinfo/qml-battery
INSTALLS += target app
