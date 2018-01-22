/****************************************************************************
**
** Copyright (C) 2018 Canonical, Ltd. and/or its subsidiary(-ies).
** Copyright (C) 2018 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
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

