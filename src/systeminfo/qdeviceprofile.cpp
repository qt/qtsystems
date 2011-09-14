/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
    QDeviceProfile::ProfileType profileType() { return QDeviceProfile::UnknownProfile; }
};
QT_END_NAMESPACE

QT_BEGIN_NAMESPACE

/*!
    \class QDeviceProfile
    \inmodule QtSystems
    \brief The QDeviceProfile class provides details for the profile of the device.
    \ingroup systeminfo
*/

/*!
    \enum QDeviceProfile::ProfileType
    This enum describes the type of the current profile.

    \value UnknownProfile   Profile unknown or on error.
    \value SilentProfile    Silent profile.
    \value NormalProfile    Normal profile.
    \value LoudProfile      Loud profile.
    \value VibProfile       Vibrate profile.
    \value BeepProfile      Beep profile.
    \value CustomProfile    Custom profile.
*/

/*!
    \fn  void QDeviceProfile::currentProfileChanged(const QDeviceProfile &profile)

    This signal is emitted whenever the activated profile has changed to \a profile.
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
    Returns the whether the vibration is active for this profile.
*/
bool QDeviceProfile::isVibrationActivated() const
{
    return d_ptr->isVibrationActivated();
}

/*!
    Returns the message ringtone volume for this profile, from 0 to 100. If this information is unknown,
    or error occurs, -1 is returned.
*/
int QDeviceProfile::messageRingtoneVolume() const
{
    return d_ptr->messageRingtoneVolume();
}

/*!
    Returns the voice ringtone volume for this profile, from 0 to 100. If this information is unknown,
    or error occurs, -1 is returned.
*/
int QDeviceProfile::voiceRingtoneVolume() const
{
    return d_ptr->voiceRingtoneVolume();
}

/*!
    Returns the type for this profile.
*/
QDeviceProfile::ProfileType QDeviceProfile::profileType() const
{
    return d_ptr->profileType();
}

QT_END_NAMESPACE
