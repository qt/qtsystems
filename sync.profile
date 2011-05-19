%modules = ( # path to module name map
    "QtSystemInfo" => "$basedir/src/systeminfo",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
);
%classnames = (
);
%mastercontent = (
    "systeminfo" => "#include <QtSystemInfo/QtSystemInfo>\n",
);
%modulepris = (
    "QtSystemInfo" => "$basedir/modules/qt_systeminfo.pri",
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
    },
);
