TEMPLATE = app
TARGET = qml-deviceinfo
QT += quick
SOURCES = main.cpp

app.files = \
    $$files(*.qml) \
    content

target.path = $$[QT_INSTALL_EXAMPLES]/sysinfo/qml-storageinfo
app.path = $$[QT_INSTALL_EXAMPLES]/sysinfo/qml-storageinfo
INSTALLS += target app
