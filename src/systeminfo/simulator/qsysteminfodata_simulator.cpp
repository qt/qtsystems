/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Copyright (C) 2014 BlackBerry Limited. All rights reserved.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsysteminfodata_simulator_p.h"
#include <QtCore/QDataStream>

QT_BEGIN_NAMESPACE

void qt_registerSystemInfoTypes()
{
    qRegisterMetaTypeStreamOperators<QBatteryInfoData>("QBatteryInfoData");
}

QDataStream &operator<<(QDataStream &out, const QBatteryInfoData &s)
{
    out << static_cast<qint32>(s.chargingState) << static_cast<qint32>(s.chargerType)
        << static_cast<qint32>(s.levelStatus);

    out << static_cast<qint32>(s.currentFlow) << static_cast<qint32>(s.maximumCapacity)
        << static_cast<qint32>(s.remainingCapacity) << static_cast<qint32>(s.remainingChargingTime)
        << static_cast<qint32>(s.voltage);

    return out;
}

QDataStream &operator>>(QDataStream &in, QBatteryInfoData &s)
{
    qint32 chargingState, chargerType, batteryStatus;
    in >> chargingState >> chargerType >> batteryStatus;

    s.chargingState = static_cast<QBatteryInfo::ChargingState>(chargingState);
    s.chargerType = static_cast<QBatteryInfo::ChargerType>(chargerType);
    s.levelStatus = static_cast<QBatteryInfo::LevelStatus>(batteryStatus);

    qint32 currentFlow, maximumCapacity, remainingCapacity, remainingChargingTime, voltage;
    in >> currentFlow >> maximumCapacity >> remainingCapacity >> remainingChargingTime >> voltage;

    s.currentFlow = currentFlow;
    s.maximumCapacity = maximumCapacity;
    s.remainingCapacity = remainingCapacity;
    s.remainingChargingTime = remainingChargingTime;
    s.voltage = voltage;

    return in;
}

QT_END_NAMESPACE


