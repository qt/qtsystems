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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDECLARATIVEDEVICEPROFILE_P_H
#define QDECLARATIVEDEVICEPROFILE_P_H

#include <qdeviceprofile.h>

QT_BEGIN_NAMESPACE

class QDeclarativeDeviceProfile : public QObject
{
    Q_OBJECT

    Q_ENUMS(ProfileType)
    Q_PROPERTY(bool isVibrationActivated READ isVibrationActivated NOTIFY vibrationActivatedChanged)
    Q_PROPERTY(int messageRingtoneVolume READ messageRingtoneVolume NOTIFY messageRingtoneVolumeChanged)
    Q_PROPERTY(int voiceRingtoneVolume READ voiceRingtoneVolume NOTIFY voiceRingtoneVolumeChanged)
    Q_PROPERTY(ProfileType currentProfileType READ currentProfileType NOTIFY currentProfileTypeChanged)

public:
    enum ProfileType {
        UnknownProfile = QDeviceProfile::UnknownProfile,
        SilentProfile = QDeviceProfile::SilentProfile,
        NormalProfile = QDeviceProfile::NormalProfile,
        VibrationProfile = QDeviceProfile::VibrationProfile,
        BeepProfile = QDeviceProfile::BeepProfile
    };

    QDeclarativeDeviceProfile(QObject *parent = 0);
    virtual ~QDeclarativeDeviceProfile();

    bool isVibrationActivated() const;
    int messageRingtoneVolume() const;
    int voiceRingtoneVolume() const;
    ProfileType currentProfileType() const;

Q_SIGNALS:
    void vibrationActivatedChanged();
    void messageRingtoneVolumeChanged();
    void voiceRingtoneVolumeChanged();
    void currentProfileTypeChanged();

private:
    QDeviceProfile *deviceProfile;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEDEVICEPROFILE_P_H
