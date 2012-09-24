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

#ifndef QREMOTESERVICEREGISTER_DBUS_P_H
#define QREMOTESERVICEREGISTER_DBUS_P_H

#include <QUuid>
#include <QtDBus/QtDBus>

#include "qremoteserviceregister.h"
#include "instancemanager_p.h"
#include "qserviceinterfacedescriptor.h"
#include "ipcendpoint_p.h"
#include "objectendpoint_dbus_p.h"

QT_BEGIN_NAMESPACE

#define SERVER 0
#define CLIENT 1

class ObjectEndPoint;

class DBusSession: public QObject, protected QDBusContext
{
    Q_OBJECT
public:
    DBusSession(QObject* parent = 0)
        : QObject(parent)
    {
        m_accept = true;
    }
    ~DBusSession() {}

public Q_SLOTS:
    QByteArray writePackage(const QByteArray &package, int type, const QString &id) {

        QDataStream data(package);
        QServicePackage pack;
        data >> pack;

        if (type == CLIENT && pack.d->packageType == 0) {
            // Use the client DBus connection as the Id
            QDBusReply<QString> reply =
                connection().interface()->serviceOwner(message().service());
            QString clientId = reply.value();
            pack.d->payload = QVariant(clientId);

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_6);
            out << pack;

            emit packageReceived(block, type, id, -1, -1);
            return block;

        }

        if (!m_accept)
            return QByteArray();

        int pid = connection().interface()->servicePid(message().service());
        int uid = connection().interface()->serviceUid(message().service());

        emit packageReceived(package, type, id, pid, uid);
        return package;
    }

    bool processIncoming() {
        int pid = connection().interface()->servicePid(message().service());
        int uid = connection().interface()->serviceUid(message().service());
        emit newConnection(pid, uid);
        return m_accept;
    }

    void acceptIncoming(bool accept) {
        m_accept = accept;
    }

    void closeIncoming(const QString& instanceId) {
        QDBusReply<QString> reply =
            connection().interface()->serviceOwner(message().service());
        const QString& clientId = reply.value();
        emit closeConnection(clientId, instanceId);
    }

Q_SIGNALS:
    void packageReceived(const QByteArray &package, int type, const QString &id, int pid, int uid);
    void newConnection(int pid, int uid);
    void closeConnection(const QString& clientId, const QString& instanceId);

private:
    bool m_accept;
};


class QRemoteServiceRegisterDBusPrivate: public QRemoteServiceRegisterPrivate
{
    Q_OBJECT
public:
    QRemoteServiceRegisterDBusPrivate(QObject* parent);
    ~QRemoteServiceRegisterDBusPrivate();
    void publishServices(const QString& ident );

public Q_SLOTS:
    void processIncoming(int pid, int uid);

private:
    bool createServiceEndPoint(const QString& ident);

    QList<ObjectEndPoint*> pendingConnections;
    QDBusInterface *iface;
    DBusSession *session;
    QDBusConnection *connection;
};

QT_END_NAMESPACE

#endif
