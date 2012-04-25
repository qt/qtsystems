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

#include <QtCore/qmetaobject.h>

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

extern QMetaMethod proxyToSourceSignal(const QMetaMethod &, QObject *);

void QDeviceProfilePrivate::connectNotify(const QMetaMethod &signal)
{
#if !defined(QT_NO_JSONDB)
    static const QMetaMethod vibrationActivatedChangedSignal = QMetaMethod::fromSignal(&QDeviceProfilePrivate::vibrationActivatedChanged);
    static const QMetaMethod messageRingtoneVolumeChangedSignal = QMetaMethod::fromSignal(&QDeviceProfilePrivate::messageRingtoneVolumeChanged);
    static const QMetaMethod voiceRingtoneVolumeChangedSignal = QMetaMethod::fromSignal(&QDeviceProfilePrivate::voiceRingtoneVolumeChanged);
    static const QMetaMethod currentProfileTypeChangedSignal = QMetaMethod::fromSignal(&QDeviceProfilePrivate::currentProfileTypeChanged);
    if (signal == vibrationActivatedChangedSignal
            || signal == currentProfileTypeChangedSignal) {
        QMetaMethod sourceSignal = proxyToSourceSignal(signal, &jsondbWrapper);
        connect(&jsondbWrapper, sourceSignal, this, signal, Qt::UniqueConnection);
    } else if (signal == messageRingtoneVolumeChangedSignal) {
        connect(&jsondbWrapper, SIGNAL(ringtoneVolumeChanged(int)), this, SIGNAL(messageRingtoneVolumeChangedSignal(int)), Qt::UniqueConnection);
    } else if (signal == voiceRingtoneVolumeChangedSignal) {
        connect(&jsondbWrapper, SIGNAL(ringtoneVolumeChanged(int)), this, SIGNAL(voiceRingtoneVolumeChangedSignal(int)), Qt::UniqueConnection);
    }
#endif // // QT_NO_JSONDB
}

void QDeviceProfilePrivate::disconnectNotify(const QMetaMethod &signal)
{
#if !defined(QT_NO_JSONDB)
    static const QMetaMethod vibrationActivatedChangedSignal = QMetaMethod::fromSignal(&QDeviceProfilePrivate::vibrationActivatedChanged);
    static const QMetaMethod messageRingtoneVolumeChangedSignal = QMetaMethod::fromSignal(&QDeviceProfilePrivate::messageRingtoneVolumeChanged);
    static const QMetaMethod voiceRingtoneVolumeChangedSignal = QMetaMethod::fromSignal(&QDeviceProfilePrivate::voiceRingtoneVolumeChanged);
    static const QMetaMethod currentProfileTypeChangedSignal = QMetaMethod::fromSignal(&QDeviceProfilePrivate::currentProfileTypeChanged);
    if (signal == vibrationActivatedChangedSignal
            || signal == currentProfileTypeChangedSignal) {
        QMetaMethod sourceSignal = proxyToSourceSignal(signal, &jsondbWrapper);
        disconnect(&jsondbWrapper, sourceSignal, this, signal);
    } else if (signal == messageRingtoneVolumeChangedSignal) {
        disconnect(&jsondbWrapper, SIGNAL(ringtoneVolumeChanged(int)), this, SIGNAL(messageRingtoneVolumeChangedSignal(int)));
    } else if (signal == voiceRingtoneVolumeChangedSignal) {
        disconnect(&jsondbWrapper, SIGNAL(ringtoneVolumeChanged(int)), this, SIGNAL(voiceRingtoneVolumeChangedSignal(int)));
    }
#endif // QT_NO_JSONDB
}

QT_END_NAMESPACE
