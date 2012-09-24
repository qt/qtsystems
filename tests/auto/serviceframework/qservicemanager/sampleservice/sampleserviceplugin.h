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
