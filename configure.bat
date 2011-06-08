:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:: Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
:: All rights reserved.
:: Contact: Nokia Corporation (qt-info@nokia.com)
::
:: This file is part of the QtSystemKit module of the Qt Toolkit.
::
:: $QT_BEGIN_LICENSE:LGPL$
:: No Commercial Usage
:: This file contains pre-release code and may not be distributed.
:: You may use this file in accordance with the terms and conditions
:: contained in the Technology Preview License Agreement accompanying
:: this package.
::
:: GNU Lesser General Public License Usage
:: Alternatively, this file may be used under the terms of the GNU Lesser
:: General Public License version 2.1 as published by the Free Software
:: Foundation and appearing in the file LICENSE.LGPL included in the
:: packaging of this file.  Please review the following information to
:: ensure the GNU Lesser General Public License version 2.1 requirements
:: will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
::
:: In addition, as a special exception, Nokia gives you certain additional
:: rights.  These rights are described in the Nokia Qt LGPL Exception
:: version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
::
:: If you have questions regarding the use of this file, please contact
:: Nokia at qt-info@nokia.com.
::
::
::
::
::
::
::
::
:: $QT_END_LICENSE$
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


@echo off

set QT_SYSTEMKIT_BUILD_PATH=%CD%
set QT_SYSTEMKIT_SOURCE_PATH= %~dp0
cd /D %QT_SYSTEMKIT_SOURCE_PATH%
set QT_SYSTEMKIT_SOURCE_PATH=%CD%
cd /D %QT_SYSTEMKIT_BUILD_PATH%

set CONFIG_IN=%QT_SYSTEMKIT_BUILD_PATH%\config.in
set CONFIG_LOG=%QT_SYSTEMKIT_BUILD_PATH%\config.log

set QT_SYSTEMKIT_PREFIX=%QT_SYSTEMKIT_BUILD_PATH%\install
set QT_SYSTEMKIT_IMPORTS=%QT_SYSTEMKIT_PREFIX%\imports
set QT_SYSTEMKIT_INCLUDE=%QT_SYSTEMKIT_PREFIX%\include
set QT_SYSTEMKIT_LIB=%QT_SYSTEMKIT_PREFIX%\lib

set BUILD_SILENT=
set BUILD_UNITTESTS=
set RELEASEMODE=

set QMAKE_CACHE=%QT_SYSTEMKIT_BUILD_PATH%\.qmake.cache
set QMAKE_EXEC=qmake


:: Remove old stuffs
if exist %QMAKE_CACHE% del /Q %CONFIG_IN%
if exist %CONFIG_LOG% del /Q %CONFIG_LOG%
if exist %QMAKE_CACHE% del /Q %QMAKE_CACHE%


:: Parse command line parameters
:parseCommandLineParameters
if "%1" == ""                   goto startProcessing
if "%1" == "-prefix"            goto setPrefix
if "%1" == "-headerdir"         goto setHeaderDir
if "%1" == "-importdir"         goto setImportDir
if "%1" == "-libdir"            goto setLibDir
if "%1" == "-release"           goto setRelease
if "%1" == "-debug"             goto setDebug
if "%1" == "-developer-build"   goto setDeveloperBuild
if "%1" == "-qmake-exec"        goto setQmakeExec
if "%1" == "-silent"            goto setSilent
if "%1" == "-help"              goto showUsage
echo Unknown option: "%1"
goto showUsage

:showUsage
echo Configure options:
echo.
echo The defaults (*) are usually acceptable. A plus (+) denotes a default value
echo that needs to be evaluated. If the evaluation succeeds, the feature is
echo included. Here is a short explanation of each option:
echo.
echo    -prefix (dir) ...... This will install everything relative to (dir)
echo                         (default: %QT_SYSTEMKIT_PREFIX%)
echo    -headerdir (dir) ... Header files will be installed to (dir)
echo                         (default: %QT_SYSTEMKIT_INCLUDE%)
echo    -importdir (dir) ... QML plugins will be installed to (dir)
echo                         (default: %QT_SYSTEMKIT_IMPORTS%)
echo    -libdir (dir) ...... Libraries will be installed to (dir)
echo                         (default: %QT_SYSTEMKIT_LIB%)
echo.
echo *  -release ........... Compile and link QtSystemKit with debugging turned off.
echo    -debug ............. Compile and link QtSystemKit with debugging turned on.
echo    -developer-build ... Compile and link QtSystemKit with developer options (including auto-tests exporting).
echo.
echo    -qmake-exec (name) . Sets custom qmake binary.
echo    -silent ............ Reduces build output.
echo.
goto exitScript

:setPrefix
shift
set QT_SYSTEMKIT_PREFIX=%1\
shift
goto parseCommandLineParameters

:setHeaderDir
shift
set QT_SYSTEMKIT_INCLUDE=%1\
shift
goto parseCommandLineParameters

:setImportDir
shift
set QT_SYSTEMKIT_IMPORTS=%1\
shift
goto parseCommandLineParameters

:setLibDir
shift
set QT_SYSTEMKIT_LIB=%1\
shift
goto parseCommandLineParameters

:setRelease
shift
set RELEASEMODE=release
goto parseCommandLineParameters

:setDebug
shift
set RELEASEMODE=debug
goto parseCommandLineParameters

:setDeveloperBuild
shift
set BUILD_UNITTESTS=yes
set RELEASEMODE=debug
goto parseCommandLineParameters

:setQmakeExec
shift
set QMAKE_EXEC=%1
shift
goto parseCommandLineParameters

:setSilent
shift
set BUILD_SILENT=yes
goto parseCommandLineParameters

:startProcessing
echo QT_SYSTEMKIT_SOURCE_TREE = %QT_SYSTEMKIT_SOURCE_PATH:\=/% >> %QMAKE_CACHE%
echo QT_SYSTEMKIT_BUILD_TREE = %QT_SYSTEMKIT_BUILD_PATH:\=/% >> %QMAKE_CACHE%

if "%RELEASEMODE%" == "" (
    set RELEASEMODE=debug
)
echo CONFIG += %RELEASEMODE% >> %CONFIG_IN%

if "%BUILD_SILENT%" == "" (
    echo CONFIG += silent >> %CONFIG_IN%
)

if "%BUILD_UNITTESTS%" == "" (
    echo build_unit_tests = no >> %CONFIG_IN%
) else (
    echo build_unit_tests = yes >> %CONFIG_IN%
)

echo QT_SYSTEMKIT_PREFIX = %QT_SYSTEMKIT_PREFIX:\=/% >> %CONFIG_IN%
echo QT_SYSTEMKIT_IMPORTS = %QT_SYSTEMKIT_IMPORTS:\=/% >> %CONFIG_IN%
echo QT_SYSTEMKIT_INCLUDE = %QT_SYSTEMKIT_INCLUDE:\=/% >> %CONFIG_IN%
echo QT_SYSTEMKIT_LIB = %QT_SYSTEMKIT_LIB:\=/% >> %CONFIG_IN%

ren %CONFIG_IN% config.pri

echo.
echo Running %QMAKE_EXEC%...
call %QMAKE_EXEC% -recursive %VC_TEMPLATE_OPTION% %QT_SYSTEMKIT_SOURCE_PATH%\qtsystemkit.pro
if errorlevel 1 goto exitOnError
echo.
echo QtSystemKit is now configured for building. Just run 'make' to build.
echo Once everything is built, you can run 'make install' to install.
echo.
goto exitScript

:exitOnError
set QT_SYSTEMKIT_BUILD_PATH=
set QT_SYSTEMKIT_SOURCE_PATH=
set CONFIG_IN=
set CONFIG_LOG=
set QT_SYSTEMKIT_PREFIX=
set QT_SYSTEMKIT_IMPORTS=
set QT_SYSTEMKIT_INCLUDE=
set QT_SYSTEMKIT_LIB=
set BUILD_SILENT=
set BUILD_UNITTESTS=
set RELEASEMODE=
set QMAKE_CACHE=
set QMAKE_EXEC=
exit /b 1

:exitScript
set QT_SYSTEMKIT_BUILD_PATH=
set QT_SYSTEMKIT_SOURCE_PATH=
set CONFIG_IN=
set CONFIG_LOG=
set QT_SYSTEMKIT_PREFIX=
set QT_SYSTEMKIT_IMPORTS=
set QT_SYSTEMKIT_INCLUDE=
set QT_SYSTEMKIT_LIB=
set BUILD_SILENT=
set BUILD_UNITTESTS=
set RELEASEMODE=
set QMAKE_CACHE=
set QMAKE_EXEC=
exit /b 0
