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
#include "../qserviceclientcredentials_p.h"
#include "../qserviceclientcredentials.h"

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
#include "qservicemanager.h"
#include <QtAddOnJsonDb/QtAddOnJsonDb>
#include <QCoreApplication>
#include <QTextStream>
#include <QFileSystemWatcher>
#include <sys/stat.h>
#include <stdio.h>
#endif

#ifndef Q_OS_WIN
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
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
class ServiceRequestWaiter : public QObject
{
    Q_OBJECT
public:
    ServiceRequestWaiter(QObject *request, QObject *parent = 0)
        : QObject(parent)
    {
        waiting = true;
        loop = new QEventLoop(this);
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), loop, SLOT(quit()));
        connect(request, SIGNAL(launched(QString)), loop, SLOT(quit()));
        connect(request, SIGNAL(failed(QString, QString)), this, SLOT(errorEvent(QString, QString)));
        connect(request, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)), this, SLOT(ipcFault(QService::UnrecoverableIPCError)));
    }
    ~ServiceRequestWaiter() { }

    void reset() {
        waiting = true;
        error.clear();
        timer->stop();
    }

    void wait(int ms) {
        timer->setInterval(ms);
        timer->start();
        loop->exec();
        timer->stop();
    }

    bool waiting;
    QString error;

protected slots:
    void errorEvent(const QString &, const QString& error) {
        this->error = error;
        waiting = false;
        loop->quit();
    }

    void ipcFault(QService::UnrecoverableIPCError) {
        errorEvent(QString(), QLatin1Literal("Unrecoverable IPC fault, unable to request service start"));
    }

    void timeout() {
        errorEvent(QString(), QLatin1Literal("Timeout waiting for reply"));
    }

private:
        QEventLoop *loop;
        QTimer *timer;
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

#ifdef QT_MTCLIENT_PRESENT
/*
    QRemoteServiceRegister::SecurityAccessOptions opts = getSecurityOptions();

    if (opts == QRemoteServiceRegister::NoOptions) {
        localServer->setSocketOptions(QLocalServer::UserAccessOption);
    } else {
        QLocalServer::SocketOptions flags = QLocalServer::NoOptions;

        if (opts == QRemoteServiceRegister::UserAccessOption) {
            flags |= QLocalServer::UserAccessOption;
        }
        if (opts == QRemoteServiceRegister::GroupAccessOption) {
            flags |= QLocalServer::GroupAccessOption;
        }
        if (opts == QRemoteServiceRegister::OtherAccessOption) {
            flags |= QLocalServer::GroupAccessOption;
        }
        if (opts == QRemoteServiceRegister::WorldAccessOption) {
            flags |= QLocalServer::WorldAccessOption;
        }
        localServer->setSocketOptions(flags);
    }
*/
    QString location = ident;
    location = QDir::cleanPath(QDir::tempPath());
    location += QLatin1Char('/') + ident;

    // Note safe, but a temporary code
    QString tempLocation = location + QLatin1Literal(".temp");

    QLocalServer::removeServer(location);
    QLocalServer::removeServer(tempLocation);

    if ( !localServer->listen(tempLocation) ) {
        qWarning() << "SFW Cannot create local socket endpoint";
        return false;
    }

    ::chmod(tempLocation.toLatin1(), S_IWUSR|S_IRUSR|S_IWGRP|S_IRGRP|S_IWOTH|S_IROTH);
    int uid = getuid();
    int gid = getgid();
    bool doChown = false;
    if (isBaseUserIdentifierSet()) {
        uid = getBaseUserIdentifier();
        doChown = true;
    }
    if (isBaseGroupIdentifierSet()) {
        gid = getBaseGroupIdentifier();
        doChown = true;
    }
    if (doChown && (-1 == ::chown(tempLocation.toLatin1(), uid, gid))) {
        qWarning() << "Failed to chown socket to request uid/gid" << uid << gid << qt_error_string(errno);
        return false;
    }
    ::rename(tempLocation.toLatin1(), location.toLatin1());

#else
    //other IPC mechanisms such as dbus may have to publish the
    //meta object definition for all registered service types
    QLocalServer::removeServer(ident);

    if ( !localServer->listen(ident) ) {
        qWarning() << "Cannot create local socket endpoint";
        return false;
    }
#endif


    if (localServer->hasPendingConnections())
        QMetaObject::invokeMethod(this, "processIncoming", Qt::QueuedConnection);

    return true;
}

QRemoteServiceRegisterPrivate* QRemoteServiceRegisterPrivate::constructPrivateObject(QObject *parent)
{
  return new QRemoteServiceRegisterLocalSocketPrivate(parent);
}

#ifdef QT_MTCLIENT_PRESENT

#define SFW_PROCESS_TIMEOUT 5000

void doStart(const QString &location, QLocalSocket *socket) {

    QEventLoop loop;
    QFileSystemWatcher watcher;
    QObject::connect(&watcher, SIGNAL(directoryChanged(QString)), &loop, SLOT(quit()));
    watcher.addPath(QLatin1Literal("/tmp"));

    socket->connectToServer(location);
    if (socket->waitForConnected(SFW_PROCESS_TIMEOUT)) {
        return;
    }

    qWarning() << "SFW unable to connect to service, trying to start it. " << socket->errorString();

    if (location == QLatin1Literal("com.nokia.mt.processmanager.ServiceRequest") ||
        location == QLatin1Literal("com.nokia.mt.processmanager.ServiceRequestSocket")) {
        return;
    }

    QVariantMap map;
    map.insert(QLatin1Literal("interfaceName"), QLatin1Literal("com.nokia.mt.processmanager.ServiceRequest"));
    map.insert(QLatin1Literal("serviceName"), QLatin1Literal("com.nokia.mt.processmanager"));
    map.insert(QLatin1Literal("major"), 1);
    map.insert(QLatin1Literal("minor"), 0);
    map.insert(QLatin1Literal("Location"), QLatin1Literal("com.nokia.mt.processmanager.ServiceRequestSocket"));
    map.insert(QLatin1Literal("ServiceType"), QService::InterProcess);

    QServiceInterfaceDescriptor desc(map);
    QServiceManager manager;
    QObject *serviceRequest = manager.loadInterface(desc);

    qWarning() << "Called loadinterface" << serviceRequest;

    if (!serviceRequest) {
        qWarning() << "Failed to initiate communications with Process manager, can't start service" << location;
        return;
    }

    ServiceRequestWaiter waiter(serviceRequest);
    QMetaObject::invokeMethod(serviceRequest, "startService", Q_ARG(QString, location));

    waiter.wait(SFW_PROCESS_TIMEOUT);
    if (!waiter.error.isEmpty()) {
        delete serviceRequest;
        qWarning() << "Failed to start service, request sent, result" << waiter.error;
        return;
    }

    qWarning() << "SFW Starting to look for socket";

    QFileInfo file(QLatin1Literal("/tmp/") + location);
    qWarning() << "SFW checking in" << file.path() << "for the socket to come into existance" << file.filePath();

    QTimer timeout;
    timeout.start(SFW_PROCESS_TIMEOUT);
    timeout.setSingleShot(true);
    QObject::connect(&timeout, SIGNAL(timeout()), &loop, SLOT(quit()));
    QObject::connect(socket, SIGNAL(connected()), &loop, SLOT(quit()));
    while (timeout.isActive()) {
        loop.exec();
        if (socket->isValid())
            break;
        socket->connectToServer(location);
    }
    qWarning() << "SFW done waiting. Socket exists:" << file.exists() <<
                  "timeout is active" << timeout.isActive() <<
                  "isSocketValid" << socket->isValid();

}
#endif

/*
    Creates endpoint on client side.
*/
QObject* QRemoteServiceRegisterPrivate::proxyForService(const QRemoteServiceRegister::Entry& entry, const QString& location)
{
    qWarning() << "(all ok) starting SFW proxyForService" << location;

    QLocalSocket* socket = new QLocalSocket();

#ifdef QT_MTCLIENT_PRESENT
    doStart(location, socket);
#else

    socket->connectToServer(location);

    if (!socket->waitForConnected()){
        if (!socket->isValid()) {
            QString path = location;
            qWarning() << "Cannot connect to remote service, trying to start service " << path;
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
        }
    }
#endif
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

#include "moc_qremoteserviceregister_ls_p.cpp"
#include "qremoteserviceregister_ls_p.moc"
QT_END_NAMESPACE
