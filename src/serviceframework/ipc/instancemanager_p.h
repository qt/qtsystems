/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSERVICE_INSTANCE_MANAGER_P_H
#define QSERVICE_INSTANCE_MANAGER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

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


#endif //QSERVICE_INSTANCE_MANAGER_P_H
