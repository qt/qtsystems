/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtDeclarative/qdeclarativeextensionplugin.h>
#include <QtDeclarative/qdeclarative.h>

#include "qdeclarativebatteryinfo_p.h"
#include "qdeclarativedeviceinfo_p.h"
#include <qdeviceprofile.h>
#include "qdeclarativedisplayinfo_p.h"
#include "qdeclarativenetworkinfo_p.h"
#include <qscreensaver.h>
#include "qdeclarativestorageinfo_p.h"

QT_BEGIN_NAMESPACE

class QSystemInfoDeclarativeModule : public QDeclarativeExtensionPlugin
{
    Q_OBJECT

public:
    virtual void registerTypes(const char *uri)
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtSystemInfo"));

        int major = 5;
        int minor = 0;
        qmlRegisterType<QDeclarativeBatteryInfo>(uri, major, minor, "BatteryInfo");
        qmlRegisterType<QDeclarativeDeviceInfo>(uri, major, minor, "DeviceInfo");
        qmlRegisterType<QDeviceProfile>(uri, major, minor, "DeviceProfile");
        qmlRegisterType<QDeclarativeDisplayInfo>(uri, major, minor, "DisplayInfo");
        qmlRegisterType<QDeclarativeNetworkInfo>(uri, major, minor, "NetworkInfo");
        qmlRegisterType<QScreenSaver>(uri, major, minor, "ScreenSaver");
        qmlRegisterType<QDeclarativeStorageInfo>(uri, major, minor, "StorageInfo");
    }
};

QT_END_NAMESPACE

#include "qsysteminfo.moc"

Q_EXPORT_PLUGIN2(qsysteminfodeclarativemodule, QT_PREPEND_NAMESPACE(QSystemInfoDeclarativeModule))

/*!
    \qmlclass ScreenSaver QScreenSaver
    \inqmlmodule QtSystemInfo
    \ingroup qml-systeminfo
    \brief The ScreenSaver element provides information about the screen saver.
*/

/*!
    \qmlproperty bool ScreenSaver::screenSaverEnabled

    This property holds whether or not the screen saver is enabled.

    On certain platforms, if screen saver is disabled, deep system sleep won't be automatically triggered,
    and the display won't be automatically turned off, etc.
 */


/*!
    \qmlclass DeviceProfile QDeviceProfile
    \inqmlmodule QtSystemInfo
    \ingroup qml-systeminfo
    \brief The DeviceProfile element provides information about the profile of the device.
*/

/*!
    \qmlmethod bool DeviceProfile::isVibrationActivated()

    Returns true if the vibration is currently activated, or false otherwise.
 */

/*!
    \qmlmethod int DeviceProfile::messageRingtoneVolume()

    Returns the current message ringtone volume, from 0 to 100. If this information is unknown, or
    error occurs, -1 is returned.
 */

/*!
    \qmlmethod int DeviceProfile::voiceRingtoneVolume()

    Returns the current voice ringtone volume, from 0 to 100. If this information is unknown, or error
    occurs, -1 is returned.
 */

/*!
    \qmlproperty enum DeviceProfile::profileType

    Returns the type of the current profile, possible types are:
    \list
    \o UnknownProfile    Profile unknown or on error.
    \o SilentProfile     Neither sound nor vibration is on.
    \o NormalProfile     Normal sound is on.
    \o VibrationProfile  Only vibration is on, and sound is off.
    \o BeepProfile       Only beep is on.
    \endlist
 */
