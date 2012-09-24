/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativedeviceprofile_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype DeviceProfile
    \instantiates QDeclarativeDeviceProfile
    \inqmlmodule QtSystemInfo
    \ingroup qml-systeminfo
    \brief The DeviceProfile element provides information about the profile of the device.
*/

/*!
    \internal
*/
QDeclarativeDeviceProfile::QDeclarativeDeviceProfile(QObject *parent)
    : QObject(parent)
    , deviceProfile(new QDeviceProfile(this))
{
}

/*!
    \internal
 */
QDeclarativeDeviceProfile::~QDeclarativeDeviceProfile()
{
}

/*!
    \qmlproperty bool DeviceProfile::isVibrationActivated

    This property holds whether the vibration is currently activated or deactivated.
 */

bool QDeclarativeDeviceProfile::isVibrationActivated() const
{
    connect(deviceProfile, SIGNAL(vibrationActivatedChanged(bool)),
            this, SIGNAL(vibrationActivatedChanged()), Qt::UniqueConnection);
    return deviceProfile->isVibrationActivated();
}

/*!
    \qmlproperty int DeviceProfile::messageRingtoneVolume

    This property holds the current message ringtone volume, from 0 to 100.
    If this information is unknown, voice message volume requested but no result
    received yet or error occurs, -1 is returned.
 */

int QDeclarativeDeviceProfile::messageRingtoneVolume() const
{
    connect(deviceProfile, SIGNAL(messageRingtoneVolumeChanged(int)),
            this, SIGNAL(messageRingtoneVolumeChanged()), Qt::UniqueConnection);
    return deviceProfile->messageRingtoneVolume();
}

/*!
    \qmlproperty int DeviceProfile::voiceRingtoneVolume

    This property holds the current voice ringtone volume, from 0 to 100.
    If this information is unknown, voice ringtone volume requested but no result
    received yet or error occurs, -1 is returned.
 */

int QDeclarativeDeviceProfile::voiceRingtoneVolume() const
{
    connect(deviceProfile, SIGNAL(voiceRingtoneVolumeChanged(int)),
            this, SIGNAL(voiceRingtoneVolumeChanged()), Qt::UniqueConnection);
    return deviceProfile->voiceRingtoneVolume();
}

/*!
    \qmlproperty enumeration DeviceProfile::currentProfileType

    Returns the type of the current profile, possible types are:
    \list
    \li DeviceProfile.UnknownProfile    - Profile unknown, profile type requested but no result received yet or an error occured.
    \li DeviceProfile.SilentProfile     - Neither sound nor vibration is on.
    \li DeviceProfile.NormalProfile     - Normal sound is on.
    \li DeviceProfile.VibrationProfile  - Only vibration is on, and sound is off.
    \li DeviceProfile.BeepProfile       - Only beep is on.
    \endlist
 */

QDeclarativeDeviceProfile::ProfileType QDeclarativeDeviceProfile::currentProfileType() const
{
    connect(deviceProfile, SIGNAL(currentProfileTypeChanged(QDeviceProfile::ProfileType)),
            this, SIGNAL(currentProfileTypeChanged()), Qt::UniqueConnection);
    return static_cast<QDeclarativeDeviceProfile::ProfileType>(deviceProfile->currentProfileType());
}

QT_END_NAMESPACE
