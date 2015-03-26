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

#include "qremoteserviceregister_p.h"
#include "qremoteserviceregister_ls_p.h"
#include "ipcendpoint_p.h"
#include "objectendpoint_p.h"
#include "qserviceclientcredentials_p.h"
#include "qserviceclientcredentials.h"

#include <QLocalServer>
#include <QEventLoop>
#include <QLocalSocket>
#include <QDataStream>
#include <QTimer>
#include <QProcess>
#include <QFile>
#include <QCoreApplication>
#include <QScopedPointer>
#include <QDir>

#include <time.h>
#include <sys/types.h>

#ifndef Q_OS_WIN
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#else
// Needed for ::Sleep, while we wait for a better solution
#include <windows.h>
#include <winbase.h>
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
        : QServiceIpcEndPoint(parent),
          socket(s),
          pending_bytes(0)

    {
        Q_ASSERT(socket);
        socket->setParent(this);

        connect(s, SIGNAL(readyRead()), this, SLOT(readIncoming()));
        connect(s, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(s, SIGNAL(disconnected()), this, SLOT(ipcfault()));
        connect(s, SIGNAL(error(QLocalSocket::LocalSocketError)),
                this, SLOT(socketError(QLocalSocket::LocalSocketError)));

        if (socket->bytesAvailable())
            QTimer::singleShot(0, this, SLOT(readIncoming()));

    }

    ~LocalSocketEndPoint()
    {
        disconnect(this, SLOT(ipcfault()));
        socket->close();
    }

    void getSecurityCredentials(QServiceClientCredentials &creds)
    {
        //LocalSocketEndPoint owns socket
        int fd = socket->socketDescriptor();
#if defined(LOCAL_PEERCRED)
        struct xucred xuc;
        socklen_t len = sizeof(struct xucred);

        if (getsockopt(fd, SOL_SOCKET, LOCAL_PEERCRED, &xuc, &len) == 0) {
            creds.d->pid = -1; // No PID on bsd
            creds.d->uid = xuc.cr_uid;
            creds.d->gid = xuc.cr_gid;

        } else {
            qDebug("SFW getsockopt failed: %s", qPrintable(qt_error_string(errno)));
        }
#elif defined(SO_PEERCRED)
        struct ucred uc;
        socklen_t len = sizeof(struct ucred);

        if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &uc, &len) == 0) {
            creds.d->pid = uc.pid;
            creds.d->uid = uc.uid;
            creds.d->gid = uc.gid;
        } else {
            qDebug("SFW getsockopt failed: %s", qPrintable(qt_error_string(errno)));
        }
#else
        Q_UNUSED(creds);
        Q_UNUSED(fd);
#endif
    }

    void terminateConnection()
    {
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

        QByteArray sizeblock;
        QDataStream outsize(&sizeblock, QIODevice::WriteOnly);
        outsize.setVersion(QDataStream::Qt_4_6);

        quint32 size = block.length();
        outsize << size;

        int bytes = socket->write(sizeblock);
        if (bytes != sizeof(quint32)) {
            qWarning() << "Failed to write length";
            socket->close();
            return;
        }
        bytes = socket->write(block);
        if (bytes != block.length()){
            qWarning() << "Can't send package, socket error" << block.length() << bytes;
            socket->close();
            return;
        }
    }

protected Q_SLOTS:
    void readIncoming()
    {
        while (socket->bytesAvailable()) {

            if (pending_bytes == 0) { /* New packet */
                QByteArray data = socket->read(4 - pending_header.length());
                pending_header.append(data);
                if (pending_header.length() == 4) {
                    QDataStream in_size(&pending_header, QIODevice::ReadOnly);
                    in_size.setVersion(QDataStream::Qt_4_6);
                    in_size >> pending_bytes;
                    pending_buf.clear();
                    pending_header.clear();
                }
            }

            if (pending_bytes) { /* read any new data and add to buffer */
                int readsize = pending_bytes;
                if (socket->bytesAvailable() < readsize) {
                    readsize = socket->bytesAvailable();
                }
                pending_buf.append(socket->read(readsize));
                pending_bytes -= readsize;
            }

            if (pending_bytes == 0 && !pending_buf.isEmpty()) {
                QDataStream in(pending_buf);
                in.setVersion(QDataStream::Qt_4_6);
                QServicePackage package;
                in >> package;
                incoming.enqueue(package);
                pending_buf.clear();
                emit readyRead();
            }
        }
    }

    void socketError(QLocalSocket::LocalSocketError err)
    {
        Q_UNUSED(err)
//        qWarning() << "Socket error!" << err << socket->errorString();
    }

protected Q_SLOTS:
    void ipcfault()
    {
        emit errorUnrecoverableIPCFault(QService::ErrorServiceNoLongerAvailable);
    }

private:
    QLocalSocket* socket;
    QRemoteServiceRegisterLocalSocketPrivate *serviceRegPriv;
    QByteArray pending_header;
    QByteArray pending_buf;
    quint32 pending_bytes;
};

QRemoteServiceRegisterLocalSocketPrivate::QRemoteServiceRegisterLocalSocketPrivate(QObject* parent)
    : QRemoteServiceRegisterPrivate(parent)
{
}

void QRemoteServiceRegisterLocalSocketPrivate::publishServices( const QString& ident)
{
    createServiceEndPoint(ident);
}

void QRemoteServiceRegisterLocalSocketPrivate::processIncoming()
{
    if (localServer->hasPendingConnections()) {
        QLocalSocket* s = localServer->nextPendingConnection();

        //LocalSocketEndPoint owns socket
        LocalSocketEndPoint* ipcEndPoint = new LocalSocketEndPoint(s, this);

        if (getSecurityFilter()){
            QServiceClientCredentials creds;
            ipcEndPoint->getSecurityCredentials(creds);

            qDebug() << "Security filter call";
            getSecurityFilter()(&creds);
            if (!creds.isClientAccepted()) {
                s->close();
                return;
            }
        }
        ObjectEndPoint* endpoint = new ObjectEndPoint(ObjectEndPoint::Service, ipcEndPoint, this);
        Q_UNUSED(endpoint);
    }
}

/*
    Creates endpoint on service side.
*/
bool QRemoteServiceRegisterLocalSocketPrivate::createServiceEndPoint(const QString& ident)
{

    localServer = new QLocalServer(this);
    connect(localServer, SIGNAL(newConnection()), this, SLOT(processIncoming()));

    //other IPC mechanisms such as dbus may have to publish the
    //meta object definition for all registered service types
    QLocalServer::removeServer(ident);

    if ( !localServer->listen(ident) ) {
        qWarning() << "Cannot create local socket endpoint";
        return false;
    }

    if (localServer->hasPendingConnections())
        QMetaObject::invokeMethod(this, "processIncoming", Qt::QueuedConnection);

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
    qWarning() << "(all ok) starting SFW proxyForService" << location;

    QLocalSocket* socket = new QLocalSocket();

    socket->connectToServer(location);

    if (!socket->waitForConnected()){
        if (!socket->isValid()) {
            QString path = location;
            qWarning() << "Cannot connect to remote service, trying to start service " << path;

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
#endif /* Q_OS_WIN */
                    socket->connectToServer(location);
                    // keep trying for a while
                }
                if (!socket->isValid()){
                    qWarning() << "Server failed to start within waiting period";
                    return false;
                }
            }
            else {
                qWarning() << "Server could not be started";
            }
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
        qWarning() << "SFW create object";
        return proxy;
    }
    return 0;
}

/*!
    Returns true if the service is running
*/

bool QRemoteServiceRegisterPrivate::isServiceRunning(const QRemoteServiceRegister::Entry & /*entry*/, const QString &location)
{
    QLocalSocket* socket = new QLocalSocket();
    socket->connectToServer(location);

    // give a short timeout to block, no running services should fail almost instantly
    return socket->waitForConnected(1000);
}

#include "moc_qremoteserviceregister_ls_p.cpp"
#include "qremoteserviceregister_ls_p.moc"
QT_END_NAMESPACE
