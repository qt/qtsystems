/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef SAMPLESERVICEPLUGIN_H
#define SAMPLESERVICEPLUGIN_H

#include <QObject>
#include <qserviceplugininterface.h>
#include <qserviceinterfacedescriptor.h>

QT_USE_NAMESPACE

class SampleServicePlugin : public QObject, public QServicePluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QServicePluginInterface)
    Q_PLUGIN_METADATA(IID "com.nokia.qt.QServicePluginInterface/1.0")

public:
    ~SampleServicePlugin();
    QObject* createInstance(const QServiceInterfaceDescriptor& descriptor);

    virtual void installService();
    virtual void uninstallService();
};


class SampleServicePluginClass : public QObject
{
    Q_OBJECT
public:
    SampleServicePluginClass(const QServiceInterfaceDescriptor& descriptor);
    virtual ~SampleServicePluginClass() {}

    QServiceInterfaceDescriptor descriptor() const;

public slots:
    void testSlotOne();
    void testSlotTwo() {}

protected:
    QServiceInterfaceDescriptor m_descriptor;
};

class DerivedSampleServicePluginClass : public SampleServicePluginClass
{
    Q_OBJECT
public:
    DerivedSampleServicePluginClass(const QServiceInterfaceDescriptor& descriptor);
    virtual ~DerivedSampleServicePluginClass() {}
public slots:
    void testSlotOne();
};

#endif
