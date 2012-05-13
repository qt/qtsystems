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
#include "qremoteserviceregister_unix_p.h"
#include "ipcendpoint_p.h"
#include "objectendpoint_p.h"
#include "qserviceclientcredentials_p.h"
#include "qserviceclientcredentials.h"
#include "qservicedebuglog_p.h"

#include <QDataStream>
#include <QTimer>
#include <QProcess>
#include <QFile>
#include <QCoreApplication>
#include <QScopedPointer>
#include <QDir>
#include <QEvent>
#include <QThreadStorage>
#include <QStandardPaths>
#include <QString>

#include <time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>

#include <signal.h>

#ifdef QT_MTCLIENT_PRESENT
#include "qservicemanager.h"
#include <QCoreApplication>
#include <QTime>
#include <QTextStream>
#include <QFileSystemWatcher>
#include <QSocketNotifier>
#include <sys/stat.h>
#include <stdio.h>
#ifndef Q_OS_MAC
#include <sys/inotify.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#endif


#ifdef LOCAL_PEERCRED /* from sys/un.h */
#include <sys/ucred.h>
#endif

QT_BEGIN_NAMESPACE

//IPC based on unix domain sockets

class UnixEndPoint;
class Waiter;

static void qt_ignore_sigpipe()
{
#ifndef Q_NO_POSIX_SIGNALS
     // Set to ignore SIGPIPE once only.
     static QBasicAtomicInt atom = Q_BASIC_ATOMIC_INITIALIZER(0);
     if (atom.testAndSetRelaxed(0, 1)) {
         struct sigaction noaction;
         memset(&noaction, 0, sizeof(noaction));
         noaction.sa_handler = SIG_IGN;
         ::sigaction(SIGPIPE, &noaction, 0);
         QServiceDebugLog::instance()->appendToLog(QString::fromLatin1("---  Disable SIGPIPE due to it being broken in general"));
     }
#else
     // Posix signals are not supported by the underlying platform
     QServiceDebugLog::instance()->appendToLog(QString::fromLatin1("---  Unable to disable SIGPIPE due to no posix signals"));
#endif
}

Q_GLOBAL_STATIC(QThreadStorage<QList<UnixEndPoint *> >, _q_unixendpoints);
Q_GLOBAL_STATIC(QThreadStorage<QList<QRemoteServiceRegisterUnixPrivate *> >, _q_remoteservice);
Q_GLOBAL_STATIC(QThreadStorage<QList<Waiter *> >, _q_connectionfds);

class Waiter
{
public:
    enum Type {
        Reader = 0,
        Writer,
        Timer
    };

    Waiter(Type t, int fd)
        : fd(fd),
          done(false),
          type(t),
          timeout(0),
          enabled(true)
    {
        QList<Waiter *> &conn = _q_connectionfds()->localData();
        conn.append(this);
    }

    ~Waiter()
    {
        QList<Waiter *> &conn = _q_connectionfds()->localData();
        conn.removeAll(this);
    }

    void setEnabled(bool enabled)
    {
        if (enabled == this->enabled)
            return;

        if (enabled) {
            QList<Waiter *> &conn = _q_connectionfds()->localData();
            conn.append(this);
        } else {
            QList<Waiter *> &conn = _q_connectionfds()->localData();
            conn.removeAll(this);
        }
        this->enabled = enabled;
    }

    int fd;
    bool done;
    Type type;
    int timeout;
    QString name;

private:
    bool enabled;
};

static sighandler_t _qt_service_old_winch = 0;
static bool _qt_service_old_winch_override = false;

void dump_op_log(int num) {

    qWarning() << "SFW OP LOG";
    QServiceDebugLog::instance()->dumpLog();

    if (_qt_service_old_winch)
        _qt_service_old_winch(num);
}

class UnixEndPoint : public QServiceIpcEndPoint
{
    Q_OBJECT
public:
    UnixEndPoint(int client_fd, QObject* parent = 0);
    ~UnixEndPoint();

    void getSecurityCredentials(QServiceClientCredentials &creds);

    bool event(QEvent *e);
    void terminateConnection(bool error);

    int waitForData();
    static int last_packet_size;
    static int operation_sequence;
    static QStringList op_log;

    static int runLocalEventLoop(int msec = 5000);
Q_SIGNALS:
    void errorUnrecoverableIPCFault(QService::UnrecoverableIPCError);


protected:
    void flushPackage(const QServicePackage& package);
protected slots:
    void readIncoming();

    void registerWithThreadData();
    void socketError(const QString &error);
protected slots:
    void ipcfault();

private:

    int write(const char *data, int len);

    int client_fd;
    bool connection_open;
    QSocketNotifier *readNotifier;

    QRemoteServiceRegisterUnixPrivate *serviceRegPriv;
    QByteArray pending_header;
    QByteArray pending_buf;
    quint32 pending_bytes;
};

UnixEndPoint::UnixEndPoint(int client_fd, QObject* parent)
    : QServiceIpcEndPoint(parent),
      client_fd(client_fd),
      connection_open(true),
      pending_bytes(0)

{
    qt_ignore_sigpipe();

#ifdef QT_SFW_IPC_DEBUG
    if (!_qt_service_old_winch_override) {
        _qt_service_old_winch = ::signal(SIGWINCH, dump_op_log);
        _qt_service_old_winch_override = true;
    }
#endif

    registerWithThreadData();
    readNotifier = new QSocketNotifier(client_fd, QSocketNotifier::Read, this);

    QObject::connect(readNotifier, SIGNAL(activated(int)), this, SLOT(readIncoming()));
    QString op = QString::fromLatin1("<-> SFW uepc on %1 service %3").arg(client_fd).arg(objectName());
    QServiceDebugLog::instance()->appendToLog(op);
}

UnixEndPoint::~UnixEndPoint()
{
    QServiceDebugLog::instance()->appendToLog(
                QString::fromLatin1("ddd delete unix endpoint %1")
                .arg(this->objectName()));
    if (connection_open)
        terminateConnection(false);
}

void UnixEndPoint::getSecurityCredentials(QServiceClientCredentials &creds)
{
    //LocalSocketEndPoint owns socket
#if defined(LOCAL_PEERCRED)
    struct xucred xuc;
    socklen_t len = sizeof(struct xucred);

    if (getsockopt(client_fd, SOL_SOCKET, LOCAL_PEERCRED, &xuc, &len) == 0) {
        creds.d->pid = -1; // No PID on bsd
        creds.d->uid = xuc.cr_uid;
        creds.d->gid = xuc.cr_gid;

    } else {
        qDebug("SFW getsockopt failed: %s", qPrintable(qt_error_string(errno)));
    }
#elif defined(SO_PEERCRED)
    struct ucred uc;
    socklen_t len = sizeof(struct ucred);

    if (getsockopt(client_fd, SOL_SOCKET, SO_PEERCRED, &uc, &len) == 0) {
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

bool UnixEndPoint::event(QEvent *e)
{
    if (e->type() == QEvent::ThreadChange) {
        QList<UnixEndPoint *> &endp = _q_unixendpoints()->localData();
        endp.removeAll(this);
        QMetaObject::invokeMethod(this, "registerWithThreadData", Qt::QueuedConnection);
    }

    return QServiceIpcEndPoint::event(e);
}

void UnixEndPoint::terminateConnection(bool error)
{
    if (connection_open) {
        QList<UnixEndPoint *> &endp = _q_unixendpoints()->localData();
        QString op = QString::fromLatin1("<-> SFW close on %1 uep %2 interface %3").arg(client_fd)
                .arg(endp.indexOf(this)).arg(objectName());
        QServiceDebugLog::instance()->appendToLog(op);
        endp.removeAll(this);
        readNotifier->setEnabled(false);
        delete readNotifier;
        ::close(client_fd);
        client_fd = -1;
        connection_open = false;
        emit disconnected();
        if (error)
            ipcfault();
    } else {
        QString op = QString::fromLatin1("<-> SFW 2nd close on interface %3").arg(objectName());
        QServiceDebugLog::instance()->appendToLog(op);
    }
}

int UnixEndPoint::waitForData()
{
    return UnixEndPoint::runLocalEventLoop();
}

int UnixEndPoint::runLocalEventLoop(int msec) {
    fd_set reader;
    fd_set writer;
    struct timeval tv;

    QTime total_time;
    total_time.start();

    FD_ZERO(&reader);
    FD_ZERO(&writer);

    QList<UnixEndPoint *> &endp = _q_unixendpoints()->localData();

    int n = 0;

    foreach (UnixEndPoint *e, endp) {
        FD_SET(e->client_fd, &reader);
        if (n <= e->client_fd) {
            n = e->client_fd+1;
        }
    }

    QList<QRemoteServiceRegisterUnixPrivate *> &endpu = _q_remoteservice()->localData();

    foreach (QRemoteServiceRegisterUnixPrivate *e, endpu) {
        FD_SET(e->server_fd, &reader);
        if (n <= e->server_fd) {
            n = e->server_fd+1;
        }
    }

    QList<Waiter *> &conn = _q_connectionfds()->localData();

    foreach (Waiter *w, conn) {
        if (w->type != Waiter::Timer) {
            if (w->type == Waiter::Reader)
                FD_SET(w->fd, &reader);
            else
                FD_SET(w->fd, &writer);
            if (n <= w->fd) {
                n = w->fd+1;
            }
        }
        if (w->timeout && w->timeout < msec) {
            msec = w->timeout;
        }
    }

    tv.tv_usec = msec*1000;
    tv.tv_sec = 0;

//    QServiceDebugLog::instance()->appendToLog(QStringLiteral("<!> select"));

    int ret = ::select(n, &reader, 0, 0, &tv);
    if (ret < 0) {
        qWarning() << Q_FUNC_INFO << "SFW Select failed" << qt_error_string(errno);
        if (errno == EBADF) {
            struct stat buf;
            foreach (UnixEndPoint *e, endp) {
                if (fstat(e->client_fd, &buf) == -1) {
                    QServiceDebugLog::instance()->appendToLog(QString::fromLatin1("### holding a bad endpoint file descriptor %1").arg(e->client_fd));
                    e->terminateConnection(true);
                }
            }

            foreach (QRemoteServiceRegisterUnixPrivate *e, endpu) {
                if (fstat(e->server_fd, &buf) == -1) {
                    QServiceDebugLog::instance()->appendToLog(QString::fromLatin1("### holding a bad service file descriptor %1").arg(e->server_fd));
                    endpu.removeAll(e);
                }
            }

            foreach (Waiter *w, conn) {
                if (w->type != Waiter::Timer) {
                    if (fstat(w->fd, &buf) == -1) {
                        QServiceDebugLog::instance()->appendToLog(QString::fromLatin1("### holding a bad file descriptor %1").arg(w->fd));
                        conn.removeAll(w);
                    }
                }
            }
        }
        return 0;
    } else if (ret == 0) {
        return 0;
    }

    foreach (UnixEndPoint *e, endp) {
        if (FD_ISSET(e->client_fd, &reader)) {
            e->readIncoming();
        }
    }

    foreach (QRemoteServiceRegisterUnixPrivate *e, endpu) {
        if (FD_ISSET(e->server_fd, &reader)) {
            e->processIncoming();
        }
    }

    foreach (Waiter *w, conn) {
        if ((w->type == Waiter::Writer && FD_ISSET(w->fd, &writer)) ||
                (w->type == Waiter::Reader && FD_ISSET(w->fd, &reader))) {
            w->done = true;
            w->setEnabled(false);
        }
    }

#ifdef QT_SFW_IPC_DEBUG
    const char *times_str = ::getenv("SFW_BLOCKING_TIMES");
    if (times_str) {
        int times = QString::fromLatin1(times_str).toInt();
        if (total_time.elapsed() > times) {
            QServiceDebugLog::instance()->appendToLog(QString::fromLatin1("... SFW spun the local eventloop for %1 ms").arg(total_time.elapsed()));
        }
    }
#endif

    return 0;
}


void UnixEndPoint::flushPackage(const QServicePackage& package)
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

#ifdef QT_SFW_IPC_DEBUG
    QString op = QString::fromLatin1("--> %1 SFW write to %2 size %3 name %4 ").arg(package.d->messageId.toString()).
            arg(client_fd).arg(size).arg(objectName());
    if (package.d->packageType == QServicePackage::ObjectCreation)
        op += QStringLiteral("ObjectCreation ");
    else if (package.d->packageType == QServicePackage::MethodCall)
        op += QStringLiteral("MethodCall ");
    else if (package.d->packageType == QServicePackage::PropertyCall)
        op += QStringLiteral("PropertyCall ");
    if (package.d->responseType == QServicePackage::NotAResponse)
        op += QStringLiteral("NotAResponse");
    else if (package.d->responseType == QServicePackage::Success)
        op += QStringLiteral("Success");
    else if (package.d->responseType == QServicePackage::Failed)
        op += QStringLiteral("Failed");
    QServiceDebugLog::instance()->appendToLog(op);
#endif

    if (!connection_open)
        return;

    int bytes = write(sizeblock.constData(), sizeblock.length());
    if (bytes != sizeof(quint32)) {
        qWarning() << "SFW Failed to write length" << client_fd << bytes;
        terminateConnection(true);
        return;
    }
    bytes = write(block.constData(), block.length());
    if (bytes != block.length()){
        qWarning() << "SFW Can't send package, socket error" << block.length() << bytes;
        terminateConnection(true);
        return;
    }
}

void UnixEndPoint::readIncoming()
{
    if (!connection_open) {
        QServiceDebugLog::instance()->appendToLog(QString::fromLatin1("zzz SFW reading incoming on closed socket? %1").arg(this->objectName()));
        return;
    }

    QByteArray raw_data;
    raw_data.resize(4096);
    int bytes = ::read(client_fd, raw_data.data(), 4096);
    if (bytes <= 0) {
        /* Linux can give us a spurious EAGAIN, only check on error */
        /* No Comment */
        if ((bytes < 0) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            QServiceDebugLog::instance()->appendToLog(QString::fromLatin1("zzz SFW spurious EAGAIN for %1").arg(this->objectName()));
            return;
        }

        socketError(qt_error_string(errno));
        terminateConnection(true);
        return;
    }
    raw_data.resize(bytes);

    while (!raw_data.isEmpty()) {

        if (pending_bytes == 0) { /* New packet */
            int bytes = 4 - pending_header.length();
            pending_header.append(raw_data.left(bytes));
            raw_data.remove(0, bytes);
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
            if (raw_data.length() < readsize) {
                readsize = raw_data.length();
            }
            pending_buf.append(raw_data.left(readsize));
            raw_data.remove(0, readsize);
            pending_bytes -= readsize;
        }

        if (pending_bytes == 0 && !pending_buf.isEmpty()) {
            int size = pending_buf.length();
            QDataStream in(pending_buf);
            in.setVersion(QDataStream::Qt_4_6);
            QServicePackage package;
            in >> package;

#ifdef QT_SFW_IPC_DEBUG
            QString op = QString::fromLatin1("<-- %1 SFW read fro %2 size %3 name %4 ").arg(package.d->messageId.toString()).
                    arg(client_fd).arg(size).arg(objectName());
            if (package.d->packageType == QServicePackage::ObjectCreation)
                op += QStringLiteral("ObjectCreation ");
            else if (package.d->packageType == QServicePackage::MethodCall)
                op += QStringLiteral("MethodCall ");
            else if (package.d->packageType == QServicePackage::PropertyCall)
                op += QStringLiteral("PropertyCall ");
            if (package.d->responseType == QServicePackage::NotAResponse)
                op += QStringLiteral("NotAResponse");
            else if (package.d->responseType == QServicePackage::Success)
                op += QStringLiteral("Success");
            else if (package.d->responseType == QServicePackage::Failed)
                op += QStringLiteral("Failed");
            QServiceDebugLog::instance()->appendToLog(op);
#endif

            incoming.enqueue(package);
            pending_buf.clear();
            emit readyRead();
        }
    }
    Q_ASSERT(raw_data.isEmpty());
}

void UnixEndPoint::registerWithThreadData()
{
    QList<UnixEndPoint *> &endp = _q_unixendpoints()->localData();
    endp.append(this);
}

void UnixEndPoint::socketError(const QString &error)
{
    Q_UNUSED(error)
}

void UnixEndPoint::ipcfault()
{
    emit errorUnrecoverableIPCFault(QService::ErrorServiceNoLongerAvailable);
//    QMetaObject::invokeMethod(this, "errorUnrecoverableIPCFault", Qt::QueuedConnection, Q_ARG(QService::UnrecoverableIPCError, QService::ErrorServiceNoLongerAvailable));
}

int UnixEndPoint::write(const char *data, int len)
{
    int left = len;

    while (left > 0) {
        int ret = ::write(client_fd, data+(len-left), left);
        if (ret == len) {
            break;
        } else if ((ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) ||
                   (ret >= 0)) {

            left -= ret;

            fd_set write;

            FD_ZERO(&write);

            FD_SET(client_fd, &write);
            int n = client_fd+1;
            struct timeval tv;
            tv.tv_sec = 5;
            tv.tv_usec = 0;

            ::select(n, 0, &write, 0, &tv);

            if (!FD_ISSET(client_fd, &write)) {
                qWarning() << "Failed to write data to socket" << client_fd << errno << qt_error_string(errno);
                return -1;
            }
        } else {
            qWarning() << "SFW Error, failed to write to socket" << client_fd << qt_error_string(errno);
            return -1;
        }
    }
    return len;
}

#ifdef QT_MTCLIENT_PRESENT
class ServiceRequestWaiter : public QObject
{
    Q_OBJECT
public:
    ServiceRequestWaiter(QObject *request, QObject *parent = 0)
        : QObject(parent), receivedLaunched(false)
    {
        connect(request, SIGNAL(launched(QString)), this, SLOT(launched()));
        connect(request, SIGNAL(failed(QString, QString)), this, SLOT(errorEvent(QString, QString)));
        connect(request, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)),
                this, SLOT(ipcFault(QService::UnrecoverableIPCError)));
    }
    ~ServiceRequestWaiter() { }

    QString errorString;
    bool receivedLaunched;

signals:
    void ok();
    void error();

protected slots:
    void errorEvent(const QString &, const QString& errorString) {
        qDebug() << "Got error evernt";
        this->errorString = errorString;
        emit error();
    }

    void ipcFault(QService::UnrecoverableIPCError) {
        errorEvent(QString(), QStringLiteral("Unrecoverable IPC fault, unable to request service start"));
    }

    void timeout() {
        qWarning() << "SFW Timeout talking to the process manager?? exect failure";
        errorEvent(QString(), QStringLiteral("Timeout waiting for reply"));
    }

    void launched() {
        qWarning() << "SFW got laucnhed from PM";
        receivedLaunched = true;
        emit ok();
    }
};

#endif

QRemoteServiceRegisterUnixPrivate::QRemoteServiceRegisterUnixPrivate(QObject* parent)
    : QRemoteServiceRegisterPrivate(parent), server_fd(-1), server_notifier(0)
{
}

QRemoteServiceRegisterUnixPrivate::~QRemoteServiceRegisterUnixPrivate()
{
    QServiceDebugLog::instance()->appendToLog(
                QString::fromLatin1("ddd delete remote service register private object %1")
                .arg(this->objectName()));
    QList<QRemoteServiceRegisterUnixPrivate *> &endp = _q_remoteservice()->localData();
    endp.removeAll(this);
    QServiceDebugLog::instance()->appendToLog(
                QString::fromLatin1("ddd delete done %1")
                .arg(this->objectName()));
}

void QRemoteServiceRegisterUnixPrivate::publishServices( const QString& ident)
{
    createServiceEndPoint(ident);
}

void QRemoteServiceRegisterUnixPrivate::processIncoming()
{
    struct sockaddr_un name;
    int len = sizeof(struct sockaddr_un);

    int client_fd = ::accept(server_fd, (struct sockaddr *) &name, (socklen_t *) &len);

    QString op = QString::fromLatin1("<-> SFW accep on %1 for %2 service %5").arg(server_fd).
            arg(client_fd).arg(objectName());
    QServiceDebugLog::instance()->appendToLog(op);

    if (client_fd != -1) {
        //LocalSocketEndPoint owns socket
        UnixEndPoint* ipcEndPoint = new UnixEndPoint(client_fd, this);

        ipcEndPoint->setObjectName(objectName() + QString(QLatin1String(" instance on fd %1")).arg(client_fd));

        if (getSecurityFilter()){
            QServiceClientCredentials creds;
            ipcEndPoint->getSecurityCredentials(creds);

            getSecurityFilter()(&creds);
            if (!creds.isClientAccepted()) {
                ipcEndPoint->terminateConnection(true);
                return;
            }
        }
        ObjectEndPoint* endpoint = new ObjectEndPoint(ObjectEndPoint::Service, ipcEndPoint, this);
        endpoint->setObjectName(objectName() + QString(QLatin1String(" instance on fd %1")).arg(client_fd));
        Q_UNUSED(endpoint);
    }
}

/*
    Creates endpoint on service side.
*/
bool QRemoteServiceRegisterUnixPrivate::createServiceEndPoint(const QString& ident)
{
    setObjectName(ident);
    server_fd = ::socket(PF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        qWarning() << "SFW Failed to create server socket" << ident << qt_error_string(errno);
        return false;
    }

    QString location = ident;
    location = QDir::cleanPath(QDir::tempPath());
    location += QLatin1Char('/') + ident;

    QString op = QString::fromLatin1("<-> SFW listen on %1 for %2 op %4 service %5").arg(server_fd).arg(objectName());
    QServiceDebugLog::instance()->appendToLog(op);


    // Note safe, but temporary code
    QString tempLocation = location + QStringLiteral(".temp");
    QString pidLocation = location + QStringLiteral(".pid");

    ::unlink(location.toLatin1());
    ::unlink(tempLocation.toLatin1());
    ::unlink(pidLocation.toLatin1());

    struct sockaddr_un name;
    name.sun_family = PF_UNIX;

    ::memcpy(name.sun_path, tempLocation.toLatin1().data(),
             tempLocation.toLatin1().size() + 1);

    if (-1 == ::bind(server_fd, (const sockaddr *)&name, sizeof(struct sockaddr_un))) {
        qWarning() << "Failed to bind to server socket" << tempLocation << qt_error_string(errno);
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

    if (-1 == ::listen(server_fd, 50)) {
        qWarning() << "Failed to listen on server socket" << tempLocation << qt_error_string(errno);
        return false;
    }

    ::rename(tempLocation.toLatin1(), location.toLatin1());

    server_notifier = new QSocketNotifier(server_fd, QSocketNotifier::Read, this);
    connect(server_notifier, SIGNAL(activated(int)), this, SLOT(processIncoming()));

    registerWithThreadData();

    op = QString::fromLatin1("<-> %1 SFW creat to %2 size %3 name %4").arg(QString()).
            arg(server_fd).arg(0).arg(objectName());
    QServiceDebugLog::instance()->appendToLog(op);

    FILE *f = ::fopen(pidLocation.toLatin1(), "w");
    if (f) {
        ::fprintf(f, "%d\n", ::getpid());
        ::fclose(f);
    } else {
        qWarning() << "Failed to create pid file for location" << pidLocation;
    }

    return true;
}

bool QRemoteServiceRegisterUnixPrivate::event(QEvent *e)
{
    if (e->type() == QEvent::ThreadChange && server_fd != -1) {
        QList<QRemoteServiceRegisterUnixPrivate *> &endp = _q_remoteservice()->localData();
        endp.removeAll(this);
        QMetaObject::invokeMethod(this, "registerWithThreadData", Qt::QueuedConnection);
    }

    return QRemoteServiceRegisterPrivate::event(e);
}

void QRemoteServiceRegisterUnixPrivate::registerWithThreadData()
{
    if (server_fd != -1) {
        QList<QRemoteServiceRegisterUnixPrivate *> &endp = _q_remoteservice()->localData();
        endp.append(this);
    }
}

QRemoteServiceRegisterPrivate* QRemoteServiceRegisterPrivate::constructPrivateObject(QObject *parent)
{
  return new QRemoteServiceRegisterUnixPrivate(parent);
}

#ifdef QT_MTCLIENT_PRESENT

#define SFW_PROCESS_TIMEOUT 10000

int doStart(const QString &location) {

    QLatin1String fmt("hh:mm:ss.zzz");
    QTime total_time;
    total_time.start();

#ifndef Q_OS_MAC
    int fd = ::inotify_init1(IN_NONBLOCK|IN_CLOEXEC);
    int wd = ::inotify_add_watch(fd, "/tmp/", IN_MOVED_TO|IN_CREATE);

    Waiter *w_inotify = new Waiter(Waiter::Reader, fd);
    w_inotify->name = QStringLiteral("inotifier waiter for") + location;
#else
    Waiter *w_inotify = new Waiter(Waiter::Timer, -1);
    w_inotify->timeout = 100;
#endif

    int socketfd = ::socket(PF_UNIX, SOCK_STREAM, 0);
    // set non blocking so we can try to connect and it wont wait
    int flags = ::fcntl(socketfd, F_GETFL, 0);
    fcntl(socketfd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_un name;
    name.sun_family = PF_UNIX;

    QString fullPath = QDir::cleanPath(QDir::tempPath());
    fullPath += QLatin1Char('/') + location;

    ::memcpy(name.sun_path, fullPath.toLatin1().data(),
             fullPath.toLatin1().size() + 1);

    Waiter *w = new Waiter(Waiter::Reader, socketfd);
    w->name = QStringLiteral("connect waiter for") + location;

    int ret = ::connect(socketfd, (struct sockaddr *) &name, sizeof(struct sockaddr_un));

    if (ret == -1 && errno == EAGAIN) {
        QTime e;
        e.start();

        while ((e.elapsed() < 5000) && !w->done) {
            UnixEndPoint::runLocalEventLoop();
        }

        if (w->done) {
#ifndef Q_OS_MAC
          ::inotify_rm_watch(fd, wd);
            ::close(fd);
#endif
            delete w;
            delete w_inotify;
            return socketfd;
        }
    } else if (ret == 0) {
#ifndef Q_OS_MAC
        ::inotify_rm_watch(fd, wd);
        ::close(fd);
#endif
        delete w;
        delete w_inotify;
        return socketfd;
    }

    qWarning() << QTime::currentTime().toString(fmt)
               << "SFW unable to connect to service, trying to start it." << ret << qt_error_string(errno);

    if (location == QStringLiteral("com.nokia.mt.processmanager.ServiceRequest") ||
        location == QStringLiteral("com.nokia.mt.processmanager.ServiceRequestSocket")) {
#ifndef Q_OS_MAC
        ::inotify_rm_watch(fd, wd);
        ::close(fd);
#endif
        delete w;
        delete w_inotify;
        close(socketfd);
        return -1;
    }

    QVariantMap map;
    map.insert(QStringLiteral("interfaceName"), QStringLiteral("com.nokia.mt.processmanager.ServiceRequest"));
    map.insert(QStringLiteral("serviceName"), QStringLiteral("com.nokia.mt.processmanager"));
    map.insert(QStringLiteral("major"), 1);
    map.insert(QStringLiteral("minor"), 0);
    map.insert(QStringLiteral("Location"), QStringLiteral("com.nokia.mt.processmanager.ServiceRequestSocket"));
    map.insert(QStringLiteral("ServiceType"), QService::InterProcess);

    QServiceInterfaceDescriptor desc(map);
    QServiceManager manager;
    QObject *serviceRequest = manager.loadInterface(desc);

    qWarning() << QTime::currentTime().toString(fmt)
               << "SFW Called loadinterface" << serviceRequest;

    if (!serviceRequest) {
        qWarning() << "Failed to initiate communications with Process manager, can't start service" << location;
#ifndef Q_OS_MAC
        ::inotify_rm_watch(fd, wd);
        ::close(fd);
#endif
        delete w;
        delete w_inotify;
        close(socketfd);
        return -1;
    }

    ServiceRequestWaiter waiter(serviceRequest);

    QMetaObject::invokeMethod(serviceRequest, "startService", Qt::DirectConnection, Q_ARG(QString, location));

    QFileInfo file(QStringLiteral("/tmp/") + location);
    qWarning() << QTime::currentTime().toString(fmt)
               << "SFW checking in" << file.path() << "for the socket to come into existance" << file.filePath();

    ret = ::connect(socketfd, (struct sockaddr *)&name, sizeof(name));
    if (ret == -1 && errno == EINPROGRESS) {
        qWarning() << "SFW got conect in progress";
        w->setEnabled(true);
    } else {
        qWarning() << QTime::currentTime().toString(fmt)
                   << "SFW Failed to connect" << qt_error_string(errno) << total_time.elapsed();
        w->setEnabled(false);
    }

    bool success = false;

    while (total_time.elapsed() < SFW_PROCESS_TIMEOUT) {
        UnixEndPoint::runLocalEventLoop(SFW_PROCESS_TIMEOUT-total_time.elapsed());

#ifndef Q_OS_MAC
#define INOTIFY_SIZE (sizeof(struct inotify_event)+1024)
        char buffer[INOTIFY_SIZE];
        int n;
        while ((n = ::read(fd, buffer, INOTIFY_SIZE)) == INOTIFY_SIZE) {
        }
        if (n == -1 && errno != EAGAIN) {
            qWarning() << QTime::currentTime().toString(fmt)
                       << "Failed to read inotity, fall back to timeout" << qt_error_string(errno);
        } else {
            w_inotify->setEnabled(true);
        }
#endif

        QServiceDebugLog::instance()->appendToLog(QString::fromLatin1("ccc connect woke up"));

        ret = ::connect(socketfd, (struct sockaddr *)&name, sizeof(name));
        if (ret == 0 || (ret == -1 && errno == EISCONN)) {
            QServiceDebugLog::instance()->appendToLog(QString::fromLatin1("ccc is connected"));
            if (!waiter.receivedLaunched) {
                qWarning() << "SFW asked the PM for a start, PM never replied, application started...something appears broken";
            }
            success = true;
            break;
        } else if (ret == -1 && errno == EINPROGRESS) {
            QServiceDebugLog::instance()->appendToLog(QString::fromLatin1("ccc is in progress"));
            w->setEnabled(true);
        } else {
            w->setEnabled(false);
        }

        if (!waiter.errorString.isEmpty()) {
            qWarning() << "SFW Error talking to PM" << socketfd << waiter.errorString;
            QServiceDebugLog::instance()->appendToLog(QString::fromLatin1("ccc error received from PM %1").arg(waiter.errorString));
            success = false;
            break;
        }
    }

    QServiceDebugLog::instance()->appendToLog(QString::fromLatin1("ccc done connect %1 %2 %3").arg(total_time.elapsed()).arg(socketfd).arg(waiter.receivedLaunched));
    qWarning() << QTime::currentTime().toString(fmt) <<
                 "SFW doStart done. Total time" << total_time.elapsed() <<
                 "Socket exists:" << file.exists() <<
                 "isSocketValid" << socketfd <<
                 "pm reply" << waiter.receivedLaunched;

    if (success == false) {
        ::close(socketfd);
        socketfd = -1;
    }

    delete w;
    delete w_inotify;
    delete serviceRequest;
#ifndef Q_OS_MAC
    ::inotify_rm_watch(fd, wd);
    ::close(fd);
#endif
    return socketfd;
}
#endif

/*
    Creates endpoint on client side.
*/
QObject* QRemoteServiceRegisterPrivate::proxyForService(const QRemoteServiceRegister::Entry& entry, const QString& location)
{
    qWarning() << "(all ok) starting SFW proxyForService" << location;

#ifdef QT_SFW_IPC_DEBUG
    QString op = QString::fromLatin1("<-> SFW proxyx to %2 size %3 name %4").
            arg(entry.d->iface).arg(0).arg(location);
    QServiceDebugLog::instance()->appendToLog(op);
#endif

#ifdef QT_MTCLIENT_PRESENT
    int socketfd = doStart(location);
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
        }
    }
#endif
    if (socketfd >= 0){
        UnixEndPoint* ipcEndPoint = new UnixEndPoint(socketfd);
        ObjectEndPoint* endPoint = new ObjectEndPoint(ObjectEndPoint::Client, ipcEndPoint);

        ipcEndPoint->setObjectName(entry.interfaceName() + QString(QStringLiteral(" client end %1")).arg(socketfd));
        endPoint->setObjectName(entry.interfaceName() + QString(QStringLiteral(" client end %1")).arg(socketfd));

        QObject *proxy = endPoint->constructProxy(entry);
        if (proxy){
            QObject::connect(proxy, SIGNAL(destroyed()), endPoint, SLOT(deleteLater()));
            QObject::connect(ipcEndPoint, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)),
                             proxy, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)));
            QServiceDebugLog::instance()->appendToLog(
                        QString::fromLatin1("+++ SFW created object for %1 %2")
                        .arg(entry.interfaceName()).arg(proxy->objectName()));
            ipcEndPoint->setParent(proxy);
            endPoint->setParent(proxy);
            qWarning() << "SFW created object for" << entry.interfaceName();
        }
        else {
            qWarning() << "SFW failed to create object for" << entry.interfaceName();
            QServiceDebugLog::instance()->appendToLog(
                        QString::fromLatin1("+++ SFW create failed object for %1")
                        .arg(entry.interfaceName()));
            delete endPoint;
        }
        return proxy;
    }
    return 0;
}

bool QRemoteServiceRegisterPrivate::isServiceRunning(const QRemoteServiceRegister::Entry &, const QString &location)
{
    QString path = QDir::cleanPath(QDir::tempPath());
    path += QLatin1Char('/') + location + QStringLiteral(".pid");

    FILE *f = fopen(path.toLatin1(), "r");
    if (f) {
        int pid;
        if (fscanf(f, "%d", &pid) == 1 && pid > 0) {
            int ret = ::kill(pid, 0);
            if (ret == 0) {
                return true; // signal was sent
            }
        }
    }

    return false;
}

#include "moc_qremoteserviceregister_unix_p.cpp"
#include "qremoteserviceregister_unix_p.moc"
QT_END_NAMESPACE
