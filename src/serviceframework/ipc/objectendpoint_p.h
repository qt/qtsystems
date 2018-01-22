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

#ifndef OBJECT_ENDPOINT_H
#define OBJECT_ENDPOINT_H

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

#include "qserviceframeworkglobal.h"
#include "ipcendpoint_p.h"
#include "qremoteserviceregister.h"
#include "qservice.h"
#include <QPointer>
#include <QHash>

QT_BEGIN_NAMESPACE

class ObjectEndPointPrivate;
class Response;
class ObjectEndPoint : public QObject
{
    Q_OBJECT
public:
    enum Type {
        Service = 0,
        Client
    };

    ObjectEndPoint(Type type, QServiceIpcEndPoint* comm, QObject* parent = 0);
    ~ObjectEndPoint();
    QObject* constructProxy(const QRemoteServiceRegister::Entry& entry);

    void objectRequest(const QServicePackage& p);
    void methodCall(const QServicePackage& p);
    void propertyCall(const QServicePackage& p);

    QVariant invokeRemote(int metaIndex, const QVariantList& args, int returnType);
    QVariant invokeRemoteProperty(int metaIndex, const QVariant& arg, int returnType, QMetaObject::Call c);

    void setLookupTable(int *local, int *remote);

Q_SIGNALS:
    // deprecated, only left for dbus backend internal use
    void pendingRequestFinished();

public Q_SLOTS:
    void newPackageReady();
    void disconnected();

private:
    void waitForResponse(const QUuid& requestId);

    QServiceIpcEndPoint* dispatch;
    QPointer<QObject> service;
    ObjectEndPointPrivate* d;
    int *localToRemote;
    int *remoteToLocal;
    QHash<QUuid, Response*> openRequests;
};

QT_END_NAMESPACE

#endif //OBJECT_ENDPOINT_H
