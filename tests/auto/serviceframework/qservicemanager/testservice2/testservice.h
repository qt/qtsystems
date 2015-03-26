/****************************************************************************
**
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

#ifndef TESTSERVICE_H_
#define TESTSERVICE_H_

#include <QObject>
#include <QDebug>
#include "testserviceinterface.h"

class EmbeddedTestService : public QObject
{
    Q_OBJECT
public:
    EmbeddedTestService(QObject* parent = 0) : QObject(parent) {}

    Q_INVOKABLE int callWithInt(int number) { return number; }
};

class TestService : public QObject,
                    public ISimpleTypeTest,
                    public IComplexTypeTest
{
    Q_OBJECT
    Q_INTERFACES(ISimpleTypeTest IComplexTypeTest)
    Q_PROPERTY(QString name READ name WRITE setName)

public:
    QString name() const { return serviceName; }
    void setName(const QString &name) { serviceName = name; }

    Q_INVOKABLE void callInvokable() {}
    void callNormalMethod() {}

signals:
    void someSignal();

public slots:
    virtual void callSlot() {}
    virtual void callSlotAndSetName(const QString &s) { serviceName = s; }
    virtual QString callSlotAndReturnName() const { return serviceName; }

    QObject* embeddedTestService() { return new EmbeddedTestService(this); }

private:
    QString serviceName;
};


#endif //TESTSERVICE_H_
