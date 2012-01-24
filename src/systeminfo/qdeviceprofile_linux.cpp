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

#if !defined(QT_NO_JSONDB)
#  include "qjsondbwrapper_p.h"
#endif //QT_NO_JSONDB

QT_BEGIN_NAMESPACE

QDeviceProfilePrivate::QDeviceProfilePrivate(QDeviceProfile *parent)
    : q_ptr(parent)
#if !defined(QT_NO_JSONDB)
    , jsondbWrapper(0)
#endif //QT_NO_JSONDB
{
}

QDeviceProfilePrivate::~QDeviceProfilePrivate()
{
#if !defined(QT_NO_JSONDB)
    if (jsondbWrapper)
        delete jsondbWrapper;
#endif //QT_NO_JSONDB
}

bool QDeviceProfilePrivate::isVibrationActivated()
{
#if !defined(QT_NO_JSONDB)
    if (!jsondbWrapper)
        jsondbWrapper = new QJsonDbWrapper();
    return jsondbWrapper->isVibrationActivated();
#endif //QT_NO_JSONDB

    return false;
}

int QDeviceProfilePrivate::messageRingtoneVolume()
{
#if !defined(QT_NO_JSONDB)
    if (!jsondbWrapper)
        jsondbWrapper = new QJsonDbWrapper();
    return jsondbWrapper->getRingtoneVolume();
#endif //QT_NO_JSONDB

    return -1;
}

int QDeviceProfilePrivate::voiceRingtoneVolume()
{
#if !defined(QT_NO_JSONDB)
    if (!jsondbWrapper)
        jsondbWrapper = new QJsonDbWrapper();
    return jsondbWrapper->getRingtoneVolume();
#endif //QT_NO_JSONDB

    return -1;
}

QDeviceProfile::ProfileType QDeviceProfilePrivate::profileType()
{
#if !defined(QT_NO_JSONDB)
    if (!jsondbWrapper)
        jsondbWrapper = new QJsonDbWrapper();

    if (jsondbWrapper->getRingtoneVolume() > 0) {
        return QDeviceProfile::NormalProfile;
    } else {
        if (jsondbWrapper->isVibrationActivated())
            return QDeviceProfile::VibrationProfile;
        else
            return QDeviceProfile::SilentProfile;
    }
#endif //QT_NO_JSONDB

    return QDeviceProfile::UnknownProfile;
}

QT_END_NAMESPACE
