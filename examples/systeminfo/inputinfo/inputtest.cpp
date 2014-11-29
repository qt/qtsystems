/****************************************************************************
**
** Copyright (C) 2016 Canonical, Ltd. and/or its subsidiary(-ies).
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
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

#include "inputtest.h"
#include <qinputinfo.h>
#include <QDebug>

Inputtest::Inputtest(QObject *parent) :
    QObject(parent)
{
    inputDeviceManager = new QInputInfoManager(this);
    connect(inputDeviceManager, &QInputInfoManager::deviceAdded,this,&Inputtest::deviceAdded);
    connect(inputDeviceManager, &QInputInfoManager::deviceRemoved,this,&Inputtest::deviceRemoved);
    connect(inputDeviceManager, &QInputInfoManager::ready,this,&Inputtest::ready);
    connect(inputDeviceManager, &QInputInfoManager::filterChanged,this,&Inputtest::filterChanged);
    connect(inputDeviceManager, &QInputInfoManager::countChanged,this,&Inputtest::countChanged);

    QMap <QString, QInputDevice *> map = inputDeviceManager->deviceMap();
    qDebug() << map.count();
}

void Inputtest::deviceAdded(const QInputDevice *devicePath)
{
    qDebug() << Q_FUNC_INFO << devicePath;

 //   QInputDevice *device = inputDeviceInfo->deviceMap().value(devicePath);

//    qDebug() <<  inputDeviceInfo->deviceMap().count();
//    qDebug() << device->name() << device->devicePath();
//    qDebug() << "buttons count"<< device->buttons().count();
//    qDebug() << "switch count"<< device->switches().count();
//    qDebug() << "relativeAxes count"<< device->relativeAxes().count();
//    qDebug() << "absoluteAxes count"<< device->absoluteAxes().count();
//    qDebug() << "type" << device->type();
//    qDebug();

//    QMapIterator<QString, QInputDevice*> i(inputDeviceInfo->deviceMap());
//    while (i.hasNext()) {
//        i.next();
//   //    qDebug() << i.value()->name();
//    }
}

void Inputtest::deviceRemoved(const QString &devicePath)
{
    qDebug() << Q_FUNC_INFO << devicePath;
//    QMapIterator<QString, QInputDevice*> i(inputDeviceInfo->deviceMap());
//    while (i.hasNext()) {
//        i.next();
///       qDebug() << i.value()->name();
//    }
}

void Inputtest::ready()
{
    qDebug() << "ready <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" ;
    QMap <QString, QInputDevice *> map = inputDeviceManager->deviceMap();
    qDebug() << map.count();

    inputDeviceManager->setFilter(QInputDevice::Mouse | QInputDevice::Keyboard | QInputDevice::TouchScreen);
}

QString Inputtest::typeToString(QInputDevice::InputTypeFlags type)
{
    qDebug() << type;
    QStringList typeString;
    if (type.testFlag(QInputDevice::Button))
        typeString << QStringLiteral("Button");
    if (type.testFlag(QInputDevice::Mouse))
        typeString << QStringLiteral("Mouse");
    if (type.testFlag(QInputDevice::TouchPad))
        typeString << QStringLiteral("TouchPad");
    if (type.testFlag(QInputDevice::TouchScreen))
        typeString << QStringLiteral("TouchScreen");
    if (type.testFlag(QInputDevice::Keyboard))
        typeString << QStringLiteral("Keyboard");
    if (type.testFlag(QInputDevice::Switch))
        typeString << QStringLiteral("Switch");

    if (typeString.isEmpty())
        typeString << QStringLiteral("Unknown");
    return typeString.join((", "));
}

 void Inputtest::filterChanged(QInputDevice::InputTypeFlags filterFlags)
 {
     qDebug() << Q_FUNC_INFO << filterFlags;

     qDebug() <<"Found"<<  inputDeviceManager->deviceMap().count() << "input devices";
     QMapIterator<QString, QInputDevice*> i(inputDeviceManager->deviceMap());
     while (i.hasNext()) {
         i.next();
             qDebug() << i.value()->name() << i.value()->identifier();
             qDebug() << "buttons count"<< i.value()->buttons().count();
             qDebug() << "switch count"<< i.value()->switches().count();
             qDebug() << "relativeAxes count"<< i.value()->relativeAxes().count();
             qDebug() << "absoluteAxes count"<< i.value()->absoluteAxes().count();
             qDebug() << "type" << typeToString(i.value()->types());

             qDebug();
 //       qDebug() << i.value()->name();
     }
     qDebug() << "Number of keyboards:" << inputDeviceManager->count(QInputDevice::Keyboard);
     qDebug() << "Number of mice:" << inputDeviceManager->count(QInputDevice::Mouse);
     qDebug() << "Number of touchscreens:" << inputDeviceManager->count(QInputDevice::TouchScreen);
 }

 void Inputtest::countChanged(int newCount)
 {
     qWarning() << Q_FUNC_INFO << newCount;
 }

