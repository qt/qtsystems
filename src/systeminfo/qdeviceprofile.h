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

#ifndef QDEVICEPROFILE_H
#define QDEVICEPROFILE_H

#include <qsysteminfoglobal.h>
#include <QtCore/qobject.h>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QDeviceProfilePrivate;

class Q_SYSTEMINFO_EXPORT QDeviceProfile : public QObject
{
    Q_OBJECT

    Q_ENUMS(ProfileType)
    Q_PROPERTY(bool isVibrationActivated READ isVibrationActivated NOTIFY vibrationActivatedChanged)
    Q_PROPERTY(int messageRingtoneVolume READ messageRingtoneVolume NOTIFY messageRingtoneVolumeChanged)
    Q_PROPERTY(int voiceRingtoneVolume READ voiceRingtoneVolume NOTIFY voiceRingtoneVolumeChanged)
    Q_PROPERTY(ProfileType currentProfileType READ currentProfileType NOTIFY currentProfileTypeChanged)

public:
    enum ProfileType {
        UnknownProfile = 0,
        SilentProfile,
        NormalProfile,
        VibrationProfile,
        BeepProfile
    };

    QDeviceProfile(QObject *parent = 0);
    virtual ~QDeviceProfile();

    bool isVibrationActivated() const;
    int messageRingtoneVolume() const;
    int voiceRingtoneVolume() const;
    ProfileType currentProfileType() const;

Q_SIGNALS:
    void vibrationActivatedChanged(bool activated);
    void messageRingtoneVolumeChanged(int volume);
    void voiceRingtoneVolumeChanged(int volume);
    void currentProfileTypeChanged(QDeviceProfile::ProfileType profile);

protected:
    void connectNotify(const QMetaMethod &signal);
    void disconnectNotify(const QMetaMethod &signal);

private:
    Q_DISABLE_COPY(QDeviceProfile)
    QDeviceProfilePrivate * const d_ptr;
    Q_DECLARE_PRIVATE(QDeviceProfile)
};

QT_END_NAMESPACE
QT_END_HEADER

#endif // QDEVICEPROFILE_H
