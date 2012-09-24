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

#ifndef QSERVICE_INSTANCE_MANAGER
#define QSERVICE_INSTANCE_MANAGER

#include <qserviceframeworkglobal.h>
#include "qremoteserviceregister.h"
#include "qremoteserviceregisterentry_p.h"
#include <private/qserviceclientcredentials_p.h>
#include <QHash>
#include <QMutexLocker>
#include <QMetaObject>
#include <QMetaClassInfo>
#include <QUuid>
#include <QDebug>

QT_BEGIN_NAMESPACE

struct ServiceIdentDescriptor
{
    ServiceIdentDescriptor() : globalInstance(0), globalRefCount(0)
    {
    }

    QExplicitlySharedDataPointer<QRemoteServiceRegisterEntryPrivate> entryData;

    QHash<QUuid, QObject*> individualInstances;
    QObject* globalInstance;
    QUuid globalId;
    int globalRefCount;
};

class Q_AUTOTEST_EXPORT InstanceManager : public QObject
{
    Q_OBJECT
public:
    InstanceManager(QObject *parent = 0);
    ~InstanceManager();

    bool addType(const QRemoteServiceRegister::Entry& entry);

    const QMetaObject* metaObject(const QRemoteServiceRegister::Entry& ident) const;
    QList<QRemoteServiceRegister::Entry> allEntries() const;

    int totalInstances() const;

    QObject* createObjectInstance(const QRemoteServiceRegister::Entry& entry, QUuid& instanceId, QServiceClientCredentials& creds);
    void removeObjectInstance(const QRemoteServiceRegister::Entry& entry, const QUuid& instanceId);

    static InstanceManager* instance();

Q_SIGNALS:
    void allInstancesClosed();
    void instanceClosed(const QRemoteServiceRegister::Entry&);
    void instanceClosed(const QRemoteServiceRegister::Entry&, const QUuid&);

private:
    mutable QMutex lock;
    QHash<QRemoteServiceRegister::Entry, ServiceIdentDescriptor> metaMap;
};



QT_END_NAMESPACE


#endif //QSERVICE_INSTANCE_MANAGER
