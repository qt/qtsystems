/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystemKit module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
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

#include "qsysteminfo_p.h"
#include <QtCore/qobject.h>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QDeviceProfilePrivate;

class Q_SYSTEMINFO_EXPORT QDeviceProfile : public QObject
{
    Q_OBJECT

    Q_ENUMS(ProfileType)

public:
    enum ProfileType {
        UnknownProfile = 0,
        SilentProfile,
        NormalProfile,
        LoudProfile,
        VibProfile,
        CustomProfile,
        BeepProfile
    };

    QDeviceProfile(QObject *parent = 0);
    QDeviceProfile(const QDeviceProfile &other);
    QDeviceProfile &operator=(const QDeviceProfile &other);
    virtual ~QDeviceProfile();

    static QDeviceProfile current();

    Q_INVOKABLE bool vibrationActived() const;
    Q_INVOKABLE int messageRingtoneVolume() const;
    Q_INVOKABLE int voiceRingtoneVolume() const;
    Q_INVOKABLE QDeviceProfile::ProfileType profileType() const;

Q_SIGNALS:
    void currentProfileChanged(const QDeviceProfile &profile);

private:
    QDeviceProfilePrivate * const d_ptr;
    Q_DECLARE_PRIVATE(QDeviceProfile)
};

QT_END_NAMESPACE
QT_END_HEADER

#endif // QDEVICEPROFILE_H
