%modules = ( # path to module name map
    "QtSystemInfo" => "$basedir/src/systeminfo",
    "QtPublishSubscribe" => "$basedir/src/publishsubscribe",
    "QtServiceFramework" => "$basedir/src/serviceframework",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
);
%classnames = (
    "qtsysteminfoversion.h" => "QtSystemInfoVersion",
    "qtpublishsubscribeversion.h" => "QtPublishSubscribeVersion",
    "qtserviceframeworkversion.h" => "QtServiceFrameworkVersion",
);
%mastercontent = (
    "systeminfo" => "#include <QtSystemInfo/QtSystemInfo>\n",
    "publishsubscribe" => "#include <QtPublishSubscribe/QtPublishSubscribe>\n",
    "serviceframework" => "#include <QtServiceFramework/QtServiceFramework>\n",
);
%modulepris = (
    "QtSystemInfo" => "$basedir/modules/qt_systeminfo.pri",
    "QtPublishSubscribe" => "$basedir/modules/qt_publishsubscribe.pri",
    "QtServiceFramework" => "$basedir/modules/qt_serviceframework.pri",
);
# Module dependencies.
# Every module that is required to build this module should have one entry.
# Each of the module version specifiers can take one of the following values:
#   - A specific Git revision.
#   - any git symbolic ref resolvable from the module's repository (e.g. "refs/heads/master" to track master branch)
#
%dependencies = (
    "qtbase" => "refs/heads/master",
    "qtscript" => "refs/heads/master",
    "qtxmlpatterns" => "refs/heads/master",
    "qtdeclarative" => "refs/heads/master",
    "qtjsbackend" => "refs/heads/master",
);

# Compile tests
%configtests = (
    "gconf" => {},
    "bluez" => {},
    "udev" => {},
    "mtlib" => {},
    "x11" => {}
);
