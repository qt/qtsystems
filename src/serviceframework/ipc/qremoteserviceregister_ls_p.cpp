/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
#include <QDir>

#include <time.h>
#include <sys/types.h>          /* See NOTES */

#ifdef QT_JSONDB
#include <mtcore/notion-client.h>

#include <QCoreApplication>
#include <QTextStream>

#include "qsecuritypackage_p.h"
#include "qservicesecurity_p.h"
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
    LocalSocketEndPoint(QLocalSocket* s, QServiceSecurity *sec, QObject* parent = 0)
        : QServiceIpcEndPoint(parent),
          socket(s),
          securityService(sec)
    {
        Q_ASSERT(socket);
        socket->setParent(this);

        connect(s, SIGNAL(readyRead()), this, SLOT(readIncomingSecurity()));
        connect(s, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(s, SIGNAL(disconnected()), this, SLOT(ipcfault()));

#if defined(QT_JSONDB) && defined(QT_WAYLAND_PRESENT)
        if (sec->getSessionType() == QServiceSecurity::Client) {
            // write hello package
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_6);
            out << sec->getSecurityPackage();
            socket->write(block);
        }
#endif

        if (socket->bytesAvailable())
            QTimer::singleShot(0, this, SLOT(readIncomingSecurity()));

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
    void readIncomingSecurity()
    {
        if (!securityService || !securityService->waitingOnToken()) {
            disconnect(socket, SIGNAL(readyRead()), this, SLOT(readIncomingSecurity()));
            connect(socket, SIGNAL(readyRead()), this, SLOT(readIncoming()));
            readIncoming();
            return;
        }

        QDataStream in(socket);
        in.setVersion(QDataStream::Qt_4_6);
        if (socket->bytesAvailable()) {
            QSecurityPackage pkg;
            in >> pkg;
            if (!securityService->isTokenValid(pkg)) {
                qWarning() << Q_FUNC_INFO << "FAILED AUTH, serivce didn't have a token, or client didn't send a valid one.";
                qWarning() << Q_FUNC_INFO << "Token received was" << pkg.token().toString();
                socket->disconnect();
            }
            else {
                connect(socket, SIGNAL(readyRead()), this, SLOT(readIncoming()));
                disconnect(socket, SIGNAL(readyRead()), this, SLOT(readIncomingSecurity()));
                if (socket->bytesAvailable())
                    readIncoming();
            }
        }
    }

protected slots:
    void ipcfault()
    {
        emit errorUnrecoverableIPCFault(QService::ErrorServiceNoLongerAvailable);
    }

private:
    QLocalSocket* socket;
    QServiceSecurity *securityService;
    QRemoteServiceRegisterLocalSocketPrivate *serviceRegPriv;
};

#ifdef QT_JSONDB

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
    : QRemoteServiceRegisterPrivate(parent),
      notionClient(new NotionClient)
{    
}

void QRemoteServiceRegisterLocalSocketPrivate::publishServices( const QString& ident)
{
    createServiceEndPoint(ident);

#ifdef QT_JSONDB    
    connect(notionClient, SIGNAL(notionEvent(QVariantMap)), this, SLOT(notionEvent(QVariantMap)));
    notionClient->setActive(true);

    // Must write the outputMatch to stdout to tell it we're ready
    // We must complete this without 10 seconds
    QTextStream out(stdout); out<<"Ready";out.flush();
#endif

}

void QRemoteServiceRegisterLocalSocketPrivate::notionEvent(const QVariantMap &notion)
{
#ifdef QT_JSONDB
    if (notion.value(QLatin1String("notion")).toString() == QLatin1String("ServiceAuthorizationEvent")) {
        QString token = notion.value(QLatin1String("token")).toString();
        QStringList authorized = notion.value(QLatin1String("authorized")).toStringList();

        if(token.isEmpty() || authorized.isEmpty()) {
            qWarning() << "Invalid ServiceAuthorizedEvent notion received";
            return;
        }

        authorizedClients.insert(token, authorized);

        QVariantMap map(notion);
        map.remove(QLatin1String("notion"));
        map.insert(QLatin1String("notion"), QLatin1String("ServiceAuthorizationReply"));
        notionClient->send(map);
    }

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
        QServiceSecurity *sec = 0x0;
#if defined(QT_JSONDB) && defined(QT_WAYLAND_PRESENT)
        sec = new QServiceSecurity(QServiceSecurity::Service, this);
        sec->setAuthorizedClients(&authorizedClients);
#endif
        LocalSocketEndPoint* ipcEndPoint = new LocalSocketEndPoint(s, sec, this);
        ObjectEndPoint* endpoint = new ObjectEndPoint(ObjectEndPoint::Service, ipcEndPoint, this);
        endpoint->setServiceSecurity(sec);
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

#ifdef QT_WAYLAND_PRESENT
static QUuid doAuth(const QString &location) {
    QUuid secToken;
    NotionClient *client = new NotionClient();
    NotionWaiter *waiter = new NotionWaiter(client);
    client->setActive(true);

    QVariantMap notion;
    notion.insert(QLatin1String("notion"), QLatin1String("ServiceRequest"));
    notion.insert(QLatin1String("service"), location);
    client->send(notion);


    bool serviceRequestEventReceived = false;

    while (!serviceRequestEventReceived) {
        waiter->wait(30000);

        if (!waiter->errorNotion.isEmpty() &&
                (waiter->errorNotion == QLatin1String("ServiceRequest"))) {
            qWarning() << "Error on ServiceRequest!" << waiter->errorText;
            delete waiter;
            delete client;
            return false;
        }

        if (waiter->waitingOnNotion == true) {
            qWarning() << "Notions failed to return within waiting period";
            delete waiter;
            delete client;
            return false;
        }

        if (waiter->notion.value(QLatin1String("notion")) == QLatin1String("ServiceRequestEvent")) {
            serviceRequestEventReceived = true;
        }
        else {
            waiter->reset();
        }
    }

    qDebug() << "Got ServiceRequestEvent" << waiter->notion;

    secToken = waiter->notion.value(QLatin1String("token")).toString();

    delete waiter;
    delete client;

    return secToken;
}
#endif

/*
    Creates endpoint on client side.
*/
QObject* QRemoteServiceRegisterPrivate::proxyForService(const QRemoteServiceRegister::Entry& entry, const QString& location)
{
    QLocalSocket* socket = new QLocalSocket();
    socket->connectToServer(location);
    QUuid secToken;

#ifdef QT_WAYLAND_PRESENT
    secToken = doAuth(location);
#endif

    if (!socket->waitForConnected()){
        if (!socket->isValid()) {
            QString path = location;
            qWarning() << "Cannot connect to remote service, trying to start service " << path;
            // If we have autotests enable, check for the service in .
#ifndef QT_JSONDB
#ifdef QT_BUILD_UNITTESTS
            QFile file("./" + path);
            if (file.exists()){
                path.prepend("./");
            }
#endif /* QT_BUILD_UNITTESTS */
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
#else
#ifdef QT_WAYLAND_PRESENT

            secToken = doAuth(location);

            socket->connectToServer(location);
            if (!socket->isValid()){
                qWarning() << "Server failed to start within waiting period";
                return false;
            }

#else /* QT_WAYLAND_PRESENT */
            // XXX Work around for single process systems
            qWarning() << "SFW Using single process hack to start service";
            QStringList paths;
            QString root = QCoreApplication::applicationDirPath() + QLatin1String("/../");
            paths += root;
            paths += root + QLatin1String("../opt/mt/applications/");
            paths += root + QLatin1String("opt/mt/applications/");
            paths += root + QLatin1String("applications/");
            paths += QLatin1String("/opt/mt/applications/");
            if (getenv("APP_PATH")) {
                qWarning() << "Found APP_PATH!";
                paths += QString::fromLatin1(getenv("APP_PATH")).split(QChar::fromLatin1(':'));
            }
            qWarning() << "Will look in dirs" << paths;
            foreach (QString base, paths) {
                QDir dir(base);
                dir.setFilter(QDir::Dirs);
                QFileInfoList list = dir.entryInfoList();
                for (int i = 0; i < list.size(); i++){
                    QFileInfo fileInfo = list.at(i);
                    QFile info(fileInfo.filePath() + QLatin1String("/info.json"));
                    if (info.exists() && info.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        QString id;
                        QString app;
                        while (!info.atEnd()) {
                            QByteArray line = info.readLine();
                            QString l = QLatin1String(line.constData());
                            if (l.contains(QLatin1String("Identifier"))) {
                                l = l.simplified();
                                id = l.mid(l.indexOf(QLatin1Char(':'))+3);
                                id.chop(1);
                            }
                            else if (l.contains(QLatin1String("Application"))) {
                                l = l.simplified();
                                app = l.mid(l.indexOf(QLatin1Char(':'))+3);
                                app.chop(1);
                            }
                        }
                        if (id.contains(location)) {
                            path = fileInfo.filePath() + QLatin1String("/") + app;
                            goto done;
                        }
                    }
                }
            }
done:

            qWarning() << "SFW trying to start" << path;
            qint64 pid = 0;
            // Start the service as a detached process
            if (QProcess::startDetached(path, QStringList(), QString(), &pid)){
                int i;
                socket->connectToServer(location);
                for (i = 0; !socket->isValid() && i < 1000; i++){
                    // Temporary hack till we can improve startup signaling
                    struct timespec tm;
                    tm.tv_sec = 0;
                    tm.tv_nsec = 1000000;
                    nanosleep(&tm, 0x0);
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
#endif /* QT_JSONDB */
        }
    }
    if (socket->isValid()){
        QServiceSecurity *sec = new QServiceSecurity(QServiceSecurity::Client, socket);
#if defined(QT_JSONDB) && defined(QT_WAYLAND_PRESENT)
        if (secToken.isNull()){
            qWarning() << "No security token found, client will be refused connection";
        }
        sec->setAuthToken(secToken);
#endif
        LocalSocketEndPoint* ipcEndPoint = new LocalSocketEndPoint(socket, sec);
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
