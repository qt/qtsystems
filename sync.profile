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
    "qtsvg" => "refs/heads/master",
    "qtxmlpatterns" => "refs/heads/master",
    "qtdeclarative" => "refs/heads/master",
);

# FIXME! Only run the compile tests during the syncqt run.
# The correct fix is to stop doing this at all from within sync.profile, which
# should only contain static data and should not call out to other programs.
# $out_basedir is some arbitrary variable which is defined during a syncqt
# run, and (hopefully) not during any other script sourcing this file.
my $in_syncqt = eval q{ defined( $out_basedir ) };
if ($in_syncqt) {
    warn 'FIXME: about to run some compile tests';
    do "$basedir/bin/compiletests" || die "do $basedir/bin/compiletests: $! $@";
}
