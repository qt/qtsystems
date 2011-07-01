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
# Modules and programs, and their dependencies.
# Each of the module version specifiers can take one of the following values:
#   - A specific Git revision.
#   - "LATEST_REVISION", to always test against the latest revision.
#   - "LATEST_RELEASE", to always test against the latest public release.
#   - "THIS_REPOSITORY", to indicate that the module is in this repository.
%dependencies = (
    "QtSystemInfo" => {
        "QtCore" => "LATEST_RELEASE",
        "QtDeclarative" => "LATEST_RELEASE",
        "QtGui" => "LATEST_RELEASE",
    },
    "QtPublishSubscribe" => {
        "QtCore" => "LATEST_RELEASE",
        "QtDeclarative" => "LATEST_RELEASE",
        "QtNetwork" => "LATEST_RELEASE",
    },
    "QtServiceFramework" => {
        "QtCore" => "LATEST_REVISION",
        "QtDeclarative" => "LATEST_RELEASE",
        "QtSql" => "LATEST_REVISION"
    },
);

do "$basedir/bin/compiletests" || die "do $basedir/bin/compiletests: $! $@";
