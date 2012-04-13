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

#include "qdeviceprofile_linux_p.h"

QT_BEGIN_NAMESPACE

QDeviceProfilePrivate::QDeviceProfilePrivate(QDeviceProfile *parent)
    : QObject(parent)
    , q_ptr(parent)
{
#if !defined(QT_NO_JSONDB)
    // start querying for profile data
    jsondbWrapper.currentProfileType();
#endif //QT_NO_JSONDB
}

QDeviceProfilePrivate::~QDeviceProfilePrivate()
{
}

bool QDeviceProfilePrivate::isVibrationActivated()
{
#if !defined(QT_NO_JSONDB)
    return jsondbWrapper.isVibrationActivated();
#endif //QT_NO_JSONDB

    return false;
}

int QDeviceProfilePrivate::messageRingtoneVolume()
{
#if !defined(QT_NO_JSONDB)
    return jsondbWrapper.ringtoneVolume();
#endif //QT_NO_JSONDB

    return -1;
}

int QDeviceProfilePrivate::voiceRingtoneVolume()
{
#if !defined(QT_NO_JSONDB)
    return jsondbWrapper.ringtoneVolume();
#endif //QT_NO_JSONDB

    return -1;
}

QDeviceProfile::ProfileType QDeviceProfilePrivate::currentProfileType()
{
#if !defined(QT_NO_JSONDB)
    return jsondbWrapper.currentProfileType();
#endif //QT_NO_JSONDB

    return QDeviceProfile::UnknownProfile;
}

void QDeviceProfilePrivate::connectNotify(const char *signal)
{
#if !defined(QT_NO_JSONDB)
    if (strcmp(signal, SIGNAL(vibrationActivatedChanged(bool))) == 0
            || strcmp(signal, SIGNAL(currentProfileTypeChanged(QDeviceProfile::ProfileType))) == 0) {
        connect(&jsondbWrapper, signal, this, signal, Qt::UniqueConnection);
    } else if (strcmp(signal, SIGNAL(messageRingtoneVolumeChanged(int))) == 0) {
        connect(&jsondbWrapper, SIGNAL(ringtoneVolumeChanged(int)), this, signal, Qt::UniqueConnection);
    } else if (strcmp(signal, SIGNAL(voiceRingtoneVolumeChanged(int))) == 0) {
        connect(&jsondbWrapper, SIGNAL(ringtoneVolumeChanged(int)), this, signal, Qt::UniqueConnection);
    }
#endif // // QT_NO_JSONDB
}

void QDeviceProfilePrivate::disconnectNotify(const char *signal)
{
#if !defined(QT_NO_JSONDB)
    if (strcmp(signal, SIGNAL(vibrationActivatedChanged(bool))) == 0
            || strcmp(signal, SIGNAL(currentProfileTypeChanged(QDeviceProfile::ProfileType))) == 0) {
        disconnect(&jsondbWrapper, signal, this, signal);
    } else if (strcmp(signal, SIGNAL(messageRingtoneVolumeChanged(int))) == 0) {
        disconnect(&jsondbWrapper, SIGNAL(ringtoneVolumeChanged(int)), this, signal);
    } else if (strcmp(signal, SIGNAL(voiceRingtoneVolumeChanged(int))) == 0) {
        disconnect(&jsondbWrapper, SIGNAL(ringtoneVolumeChanged(int)), this, signal);
    }
#endif // QT_NO_JSONDB
}

QT_END_NAMESPACE
