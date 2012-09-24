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

#include <qdeviceprofile.h>

QT_BEGIN_NAMESPACE
class QDeviceProfilePrivate
{
public:
    QDeviceProfilePrivate(QDeviceProfile *) {}

    bool isVibrationActivated() { return false; }
    int messageRingtoneVolume() { return -1; }
    int voiceRingtoneVolume() { return -1; }
    QDeviceProfile::ProfileType currentProfileType() { return QDeviceProfile::UnknownProfile; }
};
QT_END_NAMESPACE

#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

/*!
    \class QDeviceProfile
    \inmodule QtSystemInfo
    \brief The QDeviceProfile class provides details for the profile of the device.
    \ingroup systeminfo
*/

/*!
    \enum QDeviceProfile::ProfileType
    This enum describes the type of the current profile.

    \value UnknownProfile    Profile unknown, profile type requested but no result received yet or an error occured.
    \value SilentProfile     Neither sound nor vibration is on.
    \value NormalProfile     Normal sound is on.
    \value VibrationProfile  Only vibration is on, and sound is off.
    \value BeepProfile       Only beep is on.
*/

/*!
    \fn void QDeviceProfile::vibrationActivatedChanged(bool activated)

    This signal is emitted whenever vibration has been changed to \a activated.
 */

/*!
    \fn void QDeviceProfile::messageRingtoneVolumeChanged(int volume)

    This signal is emitted whenever the message ringtone volume has been changed to \a volume.
 */

/*!
    \fn void QDeviceProfile::voiceRingtoneVolumeChanged(int volume)

    This signal is emitted whenever the voice ringtone volume has been changed to \a volume.
 */

/*!
    \fn void QDeviceProfile::currentProfileTypeChanged(ProfileType profile)

    This signal is emitted whenever the current profile type has been changed to \a profile.
 */

/*!
    Constructs a QDeviceProfile object with the given \a parent.
*/
QDeviceProfile::QDeviceProfile(QObject *parent)
    : QObject(parent)
    , d_ptr(new QDeviceProfilePrivate(this))
{
}

/*!
    Destroys the object
*/
QDeviceProfile::~QDeviceProfile()
{
    delete d_ptr;
}

/*!
    \property QDeviceProfile::isVibrationActivated
    \brief Vibration activated or deactivated.

    Returns whether the vibration is active for this profile.
*/
bool QDeviceProfile::isVibrationActivated() const
{
    return d_ptr->isVibrationActivated();
}

/*!
    \property QDeviceProfile::messageRingtoneVolume
    \brief The message ringtone volume.

    Returns the message ringtone volume for this profile, from 0 to 100. If this information is unknown,
    message volume requested but no result received yet or error occurs the  -1 is returned.
*/
int QDeviceProfile::messageRingtoneVolume() const
{
    return d_ptr->messageRingtoneVolume();
}

/*!
    \property QDeviceProfile::voiceRingtoneVolume
    \brief The voice ringtone volume.

    Returns the voice ringtone volume for this profile, from 0 to 100. If this information is unknown,
    voice volume requested but no result received yet or error occurs the  -1 is returned.
*/
int QDeviceProfile::voiceRingtoneVolume() const
{
    return d_ptr->voiceRingtoneVolume();
}

/*!
    \property QDeviceProfile::currentProfileType
    \brief The current activated profile.

    Returns the type for this profile.
*/
QDeviceProfile::ProfileType QDeviceProfile::currentProfileType() const
{
    return d_ptr->currentProfileType();
}

QT_END_NAMESPACE
