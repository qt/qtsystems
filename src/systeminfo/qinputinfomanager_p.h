/****************************************************************************
**
** Copyright (C) 2016 Canonical, Ltd. and/or its subsidiary(-ies).
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

#ifndef QINPUTDEVICEINFO_LINUX_P_H
#define QINPUTDEVICEINFO_LINUX_P_H

#include <QObject>
#include "qinputinfo.h"

QT_BEGIN_NAMESPACE

class QInputDevicePrivate : public QObject
{
    Q_OBJECT
public:
    explicit QInputDevicePrivate(QObject *parent = 0);

    QString name;
    QString identifier;
    QList <int> buttons; //keys
    QList <int> switches;
    QList <int> relativeAxes;
    QList <int> absoluteAxes;
    QInputDevice::InputTypeFlags type;
};

class QInputInfoManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit QInputInfoManagerPrivate(QObject *parent = 0);
    QVector <QInputDevice *> deviceList;
    QMap <QString, QInputDevice *> deviceMap;
    static QInputInfoManagerPrivate * instance();

signals:
    void deviceAdded( QInputDevice *inputDevice);
    void deviceRemoved(const QString &deviceId);
    void ready();
};

QT_END_NAMESPACE

#endif // QINPUTDEVICEINFO_LINUX_P_H
