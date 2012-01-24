/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtSystems module of the Qt Toolkit.
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qremoteserviceregister_p.h"
#include "qremoteserviceregister_ls_p.h"
#include "ipcendpoint_p.h"
#include "objectendpoint_p.h"

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
#include <sys/types.h>          /* See NOTES */

#ifdef QT_MTCLIENT_PRESENT
#include <mt-client/notionclient.h>
#include <QtAddOnJsonDb/QtAddOnJsonDb>
#include <QCoreApplication>
#include <QTextStream>
#include <QFileSystemWatcher>
#include "qsecuritypackage_p.h"
#endif

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

protected slots:
    void readIncoming()
    {
        while (socket->bytesAvailable()) {

            if (pending_bytes == 0) { /* New packet */
                QDataStream in_size(socket);
                in_size >> pending_bytes;
                pending_buf.clear();
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

protected slots:
    void ipcfault()
    {
        emit errorUnrecoverableIPCFault(QService::ErrorServiceNoLongerAvailable);
    }

private:
    QLocalSocket* socket;
    QRemoteServiceRegisterLocalSocketPrivate *serviceRegPriv;
    QByteArray pending_buf;
    quint32 pending_bytes;
};

#ifdef QT_MTCLIENT_PRESENT
class NotionWaiter : public QObject
{
    Q_OBJECT
public:
    NotionWaiter(NotionClient *client, QObject *parent = 0)
        : QObject(parent),
          client(client)
    {
        waitingOnNotion = true;
        loop = new QEventLoop(this);
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), loop, SLOT(quit()));
        connect(client, SIGNAL(notionEvent(QVariantMap)), this, SLOT(notionEvent(QVariantMap)));
        connect(client, SIGNAL(errorEvent(QString,QString)), this, SLOT(errorEvent(QString,QString)));
    }
    ~NotionWaiter() { }

    void reset() {
        waitingOnNotion = true;
        timer->stop();
        notion.clear();
        errorNotion.clear();
        errorText.clear();
    }

    void wait(int ms) {
        timer->setInterval(ms);
        timer->start();
        loop->exec();
        timer->stop();
    }

    bool waitingOnNotion;
    QVariantMap notion;
    QString errorNotion;
    QString errorText;

protected slots:
    void notionEvent(const QVariantMap& map ) {
        waitingOnNotion = false;
        notion = map;
        loop->quit();
    }

    void errorEvent(const QString& event, const QString& error) {
        waitingOnNotion = false;
        errorNotion = event;
        errorText = error;
        loop->quit();
    }

private:
        QEventLoop *loop;
        QTimer *timer;
        NotionClient *client;
};

#endif

QRemoteServiceRegisterLocalSocketPrivate::QRemoteServiceRegisterLocalSocketPrivate(QObject* parent)
    : QRemoteServiceRegisterPrivate(parent)
{    
}

void QRemoteServiceRegisterLocalSocketPrivate::publishServices( const QString& ident)
{
    createServiceEndPoint(ident);

#ifdef QT_MTCLIENT_PRESENT
    // Must write the outputMatch to stdout to tell it we're ready
    // We must complete this without 10 seconds
    QTextStream out(stdout); out<<"Ready";out.flush();
#endif

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
        LocalSocketEndPoint* ipcEndPoint = new LocalSocketEndPoint(s, this);
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

#ifdef QT_MTCLIENT_PRESENT
void doStart(const QString &location, const QString &connectionToken) {

    QScopedPointer<NotionClient> client(new NotionClient(connection.data()));
    QScopedPointer<NotionWaiter> waiter(new NotionWaiter(client.data()));
    client->setToken(connectionToken);
    client->setActive(true);

    QVariantMap notion;
    notion.insert(QLatin1String("notion"), QLatin1String("ServiceRequest"));
    notion.insert(QLatin1String("service"), location);
    client->send(notion);

    QEventLoop loop;
    QFileSystemWatcher watcher;
    QFileInfo file(QLatin1Literal("/tmp/") + location);
    watcher.addPath(QLatin1Literal("/tmp"));
    qWarning() << "SFW checking in" << file.path() << "for the socket to come into existance";
    QObject::connect(&watcher, SIGNAL(directoryChanged(QString)), &loop, SLOT(quit()));
    QTimer timeout;
    timeout.start(5000);
    QObject::connect(&timeout, SIGNAL(timeout()), &loop, SLOT(quit()));
    while (!file.exists() && timeout.isActive()) {
        qWarning() << "SFW waiting for file deamon to start....";
        loop.exec();
    }
    qWarning() << "SFW done waiting";

}
#endif

/*
    Creates endpoint on client side.
*/
QObject* QRemoteServiceRegisterPrivate::proxyForService(const QRemoteServiceRegister::Entry& entry, const QString& location)
{
    qDebug() << "SFW proxyForService" << location;
    QLocalSocket* socket = new QLocalSocket();
    socket->connectToServer(location);

    if (!socket->waitForConnected()){
        if (!socket->isValid()) {
            QString path = location;
            qWarning() << "Cannot connect to remote service, trying to start service " << path;
#ifdef QT_MTCLIENT_PRESENT
            qDebug() << "SFW Sending notion to start service";
            doStart(location, entry.d->connectionToken);

            socket->connectToServer(location);
            if (!socket->isValid()){
                qWarning() << "Server failed to start within waiting period";
                return false;
            }
#else
            // If we have autotests enable, check for the service in .
#ifdef QT_BUILD_INTERNAL
            QFile file(QStringLiteral("./") + path);
            if (file.exists()){
                path.prepend(QStringLiteral("./"));
            }
#endif /* QT_BUILD_INTERNAL */
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
#endif
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
        qDebug() << "SFW create object";
        return proxy;
    }
    return 0;
}

#include "moc_qremoteserviceregister_ls_p.cpp"
#include "qremoteserviceregister_ls_p.moc"
QT_END_NAMESPACE
