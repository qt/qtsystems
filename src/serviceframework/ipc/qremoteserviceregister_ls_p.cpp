/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qremoteserviceregister_p.h"
#include "qremoteserviceregister_ls_p.h"
#include "ipcendpoint_p.h"
#include "objectendpoint_p.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QDataStream>
#include <QTimer>
#include <QProcess>
#include <QFile>

#include <time.h>
#include <sys/types.h>          /* See NOTES */

#ifndef Q_OS_WIN
#include <sys/un.h>
#include <sys/socket.h>
#else
// Needed for ::Sleep, while we wait for a better solution
#include <Windows.h>
#include <Winbase.h>
#endif

#ifdef LOCAL_PEERCRED /* from sys/un.h */
#include <sys/ucred.h>
#endif

QT_BEGIN_NAMESPACE

//IPC based on QLocalSocket

class LocalSocketEndPoint : public QServiceIpcEndPoint
{
    Q_OBJECT
public:
    LocalSocketEndPoint(QLocalSocket* s, QObject* parent = 0)
        : QServiceIpcEndPoint(parent), socket(s)
    {
        Q_ASSERT(socket);
        socket->setParent(this);
        connect(s, SIGNAL(readyRead()), this, SLOT(readIncoming()));
        connect(s, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(s, SIGNAL(disconnected()), this, SLOT(ipcfault()));

        if (socket->bytesAvailable())
            QTimer::singleShot(0, this, SLOT(readIncoming()));
    }

    ~LocalSocketEndPoint()
    {
        disconnect(this, SLOT(ipcfault()));
        socket->close();
    }

Q_SIGNALS:
    void errorUnrecoverableIPCFault(QService::UnrecoverableIPCError);


protected:
    void flushPackage(const QServicePackage& package)
    {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_6);
        out << package;
        socket->write(block);
    }

protected slots:
    void readIncoming()
    {
        QDataStream in(socket);
        in.setVersion(QDataStream::Qt_4_6);

        while (socket->bytesAvailable()) {
            QServicePackage package;
            in >> package;
            incoming.enqueue(package);
        }

        emit readyRead();
    }
    void ipcfault()
    {
        emit errorUnrecoverableIPCFault(QService::ErrorServiceNoLongerAvailable);
    }

private:
    QLocalSocket* socket;
};

QRemoteServiceRegisterLocalSocketPrivate::QRemoteServiceRegisterLocalSocketPrivate(QObject* parent)
    : QRemoteServiceRegisterPrivate(parent)
{
}

void QRemoteServiceRegisterLocalSocketPrivate::publishServices( const QString& ident)
{
    createServiceEndPoint(ident) ;
}

void QRemoteServiceRegisterLocalSocketPrivate::processIncoming()
{
    if (localServer->hasPendingConnections()) {
        QLocalSocket* s = localServer->nextPendingConnection();
        //LocalSocketEndPoint owns socket
        int fd = s->socketDescriptor();
        if (getSecurityFilter()){
            QRemoteServiceRegisterCredentials qcred;
            memset(&qcred, 0, sizeof(QRemoteServiceRegisterCredentials));
            qcred.fd = fd;

#if defined(LOCAL_PEERCRED)
            struct xucred xuc;
            socklen_t len = sizeof(struct xucred);

            if (getsockopt(fd, SOL_SOCKET, LOCAL_PEERCRED, &xuc, &len) == 0) {
                qcred.pid = -1; // No PID on bsd
                qcred.uid = xuc.cr_uid;
                qcred.gid = xuc.cr_gid;

            }

#elif defined(SO_PEERCRED)
            struct ucred uc;
            socklen_t len = sizeof(struct ucred);

            if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &uc, &len) == 0) {
                qcred.pid = uc.pid;
                qcred.uid = uc.uid;
                qcred.gid = uc.gid;
            }
            else {
                s->close();
                perror("Failed to get peer credential");
                return;
            }
#else
            s->close();
            qWarning("Credentials check unsupprted on this platform");
            return;
#endif
            qDebug() << "Security filter call";
            if (!getSecurityFilter()(reinterpret_cast<const void *>(&qcred))){
                s->close();
                return;
            }
        }
        LocalSocketEndPoint* ipcEndPoint = new LocalSocketEndPoint(s);
        ObjectEndPoint* endpoint = new ObjectEndPoint(ObjectEndPoint::Service, ipcEndPoint, this);
        Q_UNUSED(endpoint);
    }
}

/*
    Creates endpoint on service side.
*/
bool QRemoteServiceRegisterLocalSocketPrivate::createServiceEndPoint(const QString& ident)
{
    //other IPC mechanisms such as dbus may have to publish the
    //meta object definition for all registered service types
    QLocalServer::removeServer(ident);
    localServer = new QLocalServer(this);
    if ( !localServer->listen(ident) ) {
        qWarning() << "Cannot create local socket endpoint";
        return false;
    }
    connect(localServer, SIGNAL(newConnection()), this, SLOT(processIncoming()));
    if (localServer->hasPendingConnections())
        QTimer::singleShot(0, this, SLOT(processIncoming()));

    return true;
}

QRemoteServiceRegisterPrivate* QRemoteServiceRegisterPrivate::constructPrivateObject(QObject *parent)
{
  return new QRemoteServiceRegisterLocalSocketPrivate(parent);
}

/*
    Creates endpoint on client side.
*/
QObject* QRemoteServiceRegisterPrivate::proxyForService(const QRemoteServiceRegister::Entry& entry, const QString& location)
{
    QLocalSocket* socket = new QLocalSocket();
    socket->connectToServer(location);
    if (!socket->waitForConnected()){
        if (!socket->isValid()) {
            QString path = location;
            qWarning() << "Cannot connect to remote service, trying to start service " << path;
            // If we have autotests enable, check for the service in .
#ifdef QTM_BUILD_UNITTESTS
            QFile file("./" + path);
            if (file.exists()){
                path.prepend("./");
            }
#endif
            qint64 pid = 0;
            // Start the service as a detached process
            if (QProcess::startDetached(path, QStringList(), QString(), &pid)){
                int i;
                socket->connectToServer(location);
                for (i = 0; !socket->isValid() && i < 1000; i++){
                    // Temporary hack till we can improve startup signaling
#ifdef Q_OS_WIN
                    ::Sleep(10);
#else
                    struct timespec tm;
                    tm.tv_sec = 0;
                    tm.tv_nsec = 1000000;
                    nanosleep(&tm, 0x0);
#endif
                    socket->connectToServer(location);
                    // keep trying for a while
                }
                if (!socket->isValid()){
                    qWarning() << "Server failed to start within waiting period";
                    return false;
                }
            }
            else
                qWarning() << "Server could not be started";
        }
    }
    if (socket->isValid()){
        LocalSocketEndPoint* ipcEndPoint = new LocalSocketEndPoint(socket);
        ObjectEndPoint* endPoint = new ObjectEndPoint(ObjectEndPoint::Client, ipcEndPoint);

        QObject *proxy = endPoint->constructProxy(entry);
        if (proxy){
            QObject::connect(proxy, SIGNAL(destroyed()), endPoint, SLOT(deleteLater()));
            QObject::connect(ipcEndPoint, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)),
                             proxy, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)));
        }
        ipcEndPoint->setParent(proxy);
        endPoint->setParent(proxy);
        return proxy;
    }
    return 0;
}

#include "moc_qremoteserviceregister_ls_p.cpp"
#include "qremoteserviceregister_ls_p.moc"
QT_END_NAMESPACE
