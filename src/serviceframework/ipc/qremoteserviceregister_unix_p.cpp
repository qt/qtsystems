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
#include <fcntl.h>

#include <signal.h>

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
         qServiceLog() << "disable_sigpipe" << 1;
     }
#else
     // Posix signals are not supported by the underlying platform
     qServiceLog() << "disable_sigpipe" << 0
                   << "reason" << QString::fromLatin1("no posix signals");
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
        if (_q_connectionfds()) {
            QList<Waiter *> &conn = _q_connectionfds()->localData();
            conn.removeAll(this);
        } else {
            qWarning("%s:%d: waiter destroyed after _q_connectionfds",
                     __FILE__, __LINE__);
        }
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
protected Q_SLOTS:
    void readIncoming();
    void flushWriteBuffer();

    void registerWithThreadData();
    void socketError(const QString &error);
protected Q_SLOTS:
    void ipcfault();

private:

    int write(const char *data, int len);

    int client_fd;
    bool connection_open;
    QSocketNotifier *readNotifier;
    QSocketNotifier *writeNotifier;
    QByteArray pending_write;

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

    registerWithThreadData();
    readNotifier = new QSocketNotifier(client_fd, QSocketNotifier::Read, this);
    QObject::connect(readNotifier, SIGNAL(activated(int)), this, SLOT(readIncoming()));

    writeNotifier = new QSocketNotifier(client_fd, QSocketNotifier::Write, this);
    QObject::connect(writeNotifier, SIGNAL(activated(int)), this, SLOT(flushWriteBuffer()));

    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags|O_NONBLOCK);
    fcntl(client_fd, F_SETFD, FD_CLOEXEC);

    qServiceLog() << "class" << "unixep"
                  << "event" << "new"
                  << "client_fd" << client_fd
                  << "name" << objectName();
}

UnixEndPoint::~UnixEndPoint()
{
    qServiceLog() << "class" << "unixep"
                  << "event" << "delete"
                  << "client_fd" << client_fd
                  << "name" << objectName()
                  << "was_open" << (connection_open ? 1 : 0);
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
        qServiceLog() << "class" << "unixep"
                      << "event" << "getsockopt failed"
                      << "errno" << qt_error_string(errno);
        qDebug("SFW getsockopt failed: %s", qPrintable(qt_error_string(errno)));
    }
#elif defined(SO_PEERCRED)
    struct ucred uc;
    socklen_t len = sizeof(struct ucred);

    if (getsockopt(client_fd, SOL_SOCKET, SO_PEERCRED, &uc, &len) == 0) {
        creds.d->pid = uc.pid;
        creds.d->uid = uc.uid;
        creds.d->gid = uc.gid;
        qServiceLog() << "class" << "unixep"
                      << "event" << "identified"
                      << "client_fd" << client_fd
                      << "pid" << uc.pid;
    } else {
        qServiceLog() << "class" << "unixep"
                      << "event" << "getsockopt failed"
                      << "errno" << qt_error_string(errno);
        qDebug("SFW getsockopt failed: %s", qPrintable(qt_error_string(errno)));
    }
#else
    Q_UNUSED(creds);
    Q_UNUSED(fd);
#endif
}

bool UnixEndPoint::event(QEvent *e)
{
    if (e->type() == QEvent::ThreadChange && _q_unixendpoints()) {
        QList<UnixEndPoint *> &endp = _q_unixendpoints()->localData();
        endp.removeAll(this);
        QMetaObject::invokeMethod(this, "registerWithThreadData", Qt::QueuedConnection);
    }

    return QServiceIpcEndPoint::event(e);
}

void UnixEndPoint::terminateConnection(bool error)
{
    if (connection_open) {
        if (_q_unixendpoints()) {
            QList<UnixEndPoint *> &endp = _q_unixendpoints()->localData();
            endp.removeAll(this);
        }
        qServiceLog() << "class" << "unixep"
                      << "event" << "terminate"
                      << "client_fd" << client_fd
                      << "name" << objectName();
        readNotifier->setEnabled(false);
        delete readNotifier;
        writeNotifier->setEnabled(false);
        delete writeNotifier;
        ::close(client_fd);
        client_fd = -1;
        connection_open = false;
        emit disconnected();
        if (error)
            ipcfault();
    } else {
        qServiceLog() << "class" << "unixep"
                      << "event" << "double terminate"
                      << "name" << objectName();
    }
}

int UnixEndPoint::waitForData()
{
    /* no point waiting around for a dead client */
    if (client_fd == -1)
        return -1;

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

    if (!_q_unixendpoints() || !_q_remoteservice() || !_q_connectionfds()) {
        qWarning("%s:%d: runLocalEventLoop called but global statics are invalid!",
                 __FILE__, __LINE__);
    }

    QList<UnixEndPoint *> &endp = _q_unixendpoints()->localData();

    int n = 0;

    foreach (UnixEndPoint *e, endp) {
        FD_SET(e->client_fd, &reader);
        if (!e->pending_write.isEmpty()) {
            FD_SET(e->client_fd, &writer);
        }
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
    // OSX does not support usec > 1 million
    tv.tv_sec = tv.tv_usec%1000000;
    tv.tv_usec /= 1000000;

//    QServiceDebugLog::instance()->appendToLog(QStringLiteral("<!> select"));

    int ret = ::select(n, &reader, &writer, 0, &tv);
    if (ret < 0) {
        qServiceLog() << "class" << "unixep:static"
                      << "event" << "select failed"
                      << "errno" << qt_error_string(errno);
        if (errno == EBADF) {
            struct stat buf;
            foreach (UnixEndPoint *e, endp) {
                if (fstat(e->client_fd, &buf) == -1) {
                    qServiceLog() << "class" << "unixep:static"
                                  << "event" << "holding uep with bad fd"
                                  << "client_fd" << e->client_fd;
                    e->terminateConnection(true);
                }
            }

            foreach (QRemoteServiceRegisterUnixPrivate *e, endpu) {
                if (fstat(e->server_fd, &buf) == -1) {
                    qServiceLog() << "class" << "unixep:static"
                                  << "event" << "holding qrsrup with bad fd"
                                  << "server_fd" << e->server_fd;
                    endpu.removeAll(e);
                }
            }

            foreach (Waiter *w, conn) {
                if (w->type != Waiter::Timer) {
                    if (fstat(w->fd, &buf) == -1) {
                        qServiceLog() << "class" << "unixep:static"
                                      << "event" << "holding waiter with bad fd"
                                      << "fd" << w->fd;
                        conn.removeAll(w);
                    }
                }
            }
        } else if (errno == EINTR) {
            /* no error, play it again sam */
            return 0;
        }
        return 0;
    } else if (ret == 0) {
        /* timeout */
        return 0;
    }

    foreach (UnixEndPoint *e, endp) {
        if (FD_ISSET(e->client_fd, &reader)) {
            e->readIncoming();
        }
        if (!e->pending_write.isEmpty() && FD_ISSET(e->client_fd, &writer)) {
            e->flushWriteBuffer();
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
            qServiceLog() << "class" << "unixep:static"
                          << "event" << "spun local loop"
                          << "time_ms" << total_time.elapsed();
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
    const QMetaObject *mo = &QServicePackage::staticMetaObject;

    const QMetaEnum typeEnum = mo->enumerator(
                mo->indexOfEnumerator("Type"));
    const char *type = typeEnum.valueToKey(package.d->packageType);

    const QMetaEnum rtypeEnum = mo->enumerator(
                mo->indexOfEnumerator("ResponseType"));
    const char *rtype = rtypeEnum.valueToKey(package.d->responseType);

    qServiceLog() << "class" << "unixep"
                  << "event" << "write"
                  << "fd" << client_fd
                  << "size" << (qint32)size
                  << "name" << objectName()
                  << "packageType" << type
                  << "respType" << rtype;
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
        qServiceLog() << "class" << "unixep"
                      << "event" << "read on closed socket"
                      << "client_fd" << client_fd
                      << "name" << objectName();
        return;
    }

    readNotifier->setEnabled(false);

    QByteArray raw_data;
    raw_data.resize(4096);
    int bytes = ::read(client_fd, raw_data.data(), 4096);
    if (bytes <= 0) {
        /* Linux can give us a spurious EAGAIN, only check on error */
        /* No Comment */
        if ((bytes < 0) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            qServiceLog() << "class" << "unixep"
                          << "event" << "spurious eagain"
                          << "client_fd" << client_fd
                          << "name" << objectName();
            readNotifier->setEnabled(true);
            return;
        }

        socketError(qt_error_string(errno));
        terminateConnection(true);
        return;
    }
    readNotifier->setEnabled(true);
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
            QDataStream in(pending_buf);
            in.setVersion(QDataStream::Qt_4_6);
            QServicePackage package;
            in >> package;

#ifdef QT_SFW_IPC_DEBUG
            int size = pending_buf.length();
            const QMetaObject *mo = &QServicePackage::staticMetaObject;

            const QMetaEnum typeEnum = mo->enumerator(
                        mo->indexOfEnumerator("Type"));
            const char *type = typeEnum.valueToKey(package.d->packageType);

            const QMetaEnum rtypeEnum = mo->enumerator(
                        mo->indexOfEnumerator("ResponseType"));
            const char *rtype = rtypeEnum.valueToKey(package.d->responseType);

            qServiceLog() << "class" << "unixep"
                          << "event" << "read"
                          << "fd" << client_fd
                          << "size" << (qint32)size
                          << "name" << objectName()
                          << "packageType" << type
                          << "respType" << rtype;
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
    if (!_q_unixendpoints()) {
        qWarning("%s:%d: registerWithThreadData called, no thread local data!",
                 __FILE__, __LINE__);
        return;
    }

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
    pending_write.append(data, len);
    flushWriteBuffer();
    return len;
}

void UnixEndPoint::flushWriteBuffer()
{
    writeNotifier->setEnabled(false);

    if (!pending_write.isEmpty()) {
        int ret = ::write(client_fd, pending_write.constData(), pending_write.length());

        if (ret > 0) {
            pending_write.remove(0, ret);
            if (!pending_write.isEmpty()) {
                writeNotifier->setEnabled(true);
            }

            qServiceLog() << "class" << "unixep"
                          << "event" << "flush ok"
                          << "client_fd" << client_fd
                          << "wrote" << ret
                          << "pending" << pending_write.size();

        } else if ((ret == -1) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            /* no data writen, but socket is open */
            qServiceLog() << "class" << "unixep"
                          << "event" << "flush eagain"
                          << "client_fd" << client_fd
                          << "pending" << pending_write.size();

            writeNotifier->setEnabled(true);
        } else {
            qServiceLog() << "class" << "unixep"
                          << "event" << "flush FAIL"
                          << "client_fd" << client_fd
                          << "pending" << pending_write.size()
                          << "errno" << qt_error_string(errno);
        }
    } else {
        writeNotifier->setEnabled(false);
    }
}

QRemoteServiceRegisterUnixPrivate::QRemoteServiceRegisterUnixPrivate(QObject* parent)
    : QRemoteServiceRegisterPrivate(parent), server_fd(-1), server_notifier(0)
{
}

QRemoteServiceRegisterUnixPrivate::~QRemoteServiceRegisterUnixPrivate()
{
    qServiceLog() << "class" << "qrsrup"
                  << "event" << "delete start"
                  << "name" << objectName();

    if (_q_remoteservice()) {
        QList<QRemoteServiceRegisterUnixPrivate *> &endp =
                _q_remoteservice()->localData();
        endp.removeAll(this);
    }

    qServiceLog() << "class" << "qrsrup"
                  << "event" << "delete done"
                  << "name" << objectName();
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

    int flags = ::fcntl(client_fd, F_GETFL, 0);
    ::fcntl(client_fd, F_SETFL, flags|O_NONBLOCK);
    ::fcntl(client_fd, F_SETFD, FD_CLOEXEC);

    qServiceLog() << "class" << "qrsrup"
                  << "event" << "accept"
                  << "client_fd" << client_fd
                  << "name" << objectName();

    if (client_fd != -1) {
        //LocalSocketEndPoint owns socket
        UnixEndPoint* ipcEndPoint = new UnixEndPoint(client_fd, this);

        ipcEndPoint->setObjectName(objectName() + QString(QLatin1String(" instance on fd %1")).arg(client_fd));

        QServiceClientCredentials creds;
        ipcEndPoint->getSecurityCredentials(creds);

        if (getSecurityFilter()){
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

    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags|O_NONBLOCK);
    fcntl(server_fd, F_SETFD, FD_CLOEXEC);

    QString location = ident;
    location = QDir::cleanPath(QDir::tempPath());
    location += QLatin1Char('/') + ident;

    qServiceLog() << "class" << "qrsrup"
                  << "event" << "createservice start"
                  << "server_fd" << server_fd
                  << "name" << objectName();

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

    if (-1 == ::chmod(tempLocation.toLatin1(),
                S_IWUSR|S_IRUSR|S_IWGRP|S_IRGRP|S_IWOTH|S_IROTH)) {
        qWarning("SFW failed to chmod %s: %s", qPrintable(tempLocation),
                 ::strerror(errno));
    }
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

    if (-1 == ::rename(tempLocation.toLatin1(), location.toLatin1())) {
        qWarning("Failed to rename %s to %s", qPrintable(tempLocation),
                 qPrintable(location));
    }

    server_notifier = new QSocketNotifier(server_fd, QSocketNotifier::Read, this);
    connect(server_notifier, SIGNAL(activated(int)), this, SLOT(processIncoming()));

    registerWithThreadData();

    qServiceLog() << "class" << "qrsrup"
                  << "event" << "createservice done"
                  << "server_fd" << server_fd
                  << "name" << objectName();

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
    if (e->type() == QEvent::ThreadChange && server_fd != -1 && _q_remoteservice()) {
        QList<QRemoteServiceRegisterUnixPrivate *> &endp = _q_remoteservice()->localData();
        endp.removeAll(this);
        QMetaObject::invokeMethod(this, "registerWithThreadData", Qt::QueuedConnection);
    }

    return QRemoteServiceRegisterPrivate::event(e);
}

void QRemoteServiceRegisterUnixPrivate::registerWithThreadData()
{
    if (server_fd != -1 && _q_remoteservice()) {
        QList<QRemoteServiceRegisterUnixPrivate *> &endp = _q_remoteservice()->localData();
        endp.append(this);
    }
}

QRemoteServiceRegisterPrivate* QRemoteServiceRegisterPrivate::constructPrivateObject(QObject *parent)
{
  return new QRemoteServiceRegisterUnixPrivate(parent);
}

#define SFW_PROCESS_TIMEOUT 10000

int doStart(const QString &location) {

    QLatin1String fmt("hh:mm:ss.zzz");
    QTime total_time;
    total_time.start();

    qServiceLog() << "class" << "doStart"
                  << "event" << "start"
                  << "location" << location;

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
    if (socketfd < 0) {
        qWarning("socket(2) failed: %s", ::strerror(errno));
        delete w_inotify;
        return -1;
    }
    // set non blocking so we can try to connect and it wont wait
    int flags = ::fcntl(socketfd, F_GETFL, 0);
    if (flags < 0) {
        qWarning("fcntl(F_GETFL) failed: %s", ::strerror(errno));
        delete w_inotify;
        return -1;
    }
    if (fcntl(socketfd, F_SETFL, flags | O_NONBLOCK)) {
        qWarning("fcntl(F_SETFL) failed: %s", ::strerror(errno));
        delete w_inotify;
        return -1;
    }
    fcntl(socketfd, F_SETFD, FD_CLOEXEC);

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

        qServiceLog() << "class" << "doStart"
                      << "event" << "found process"
                      << "location" << location;

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

    QString path = location;

    int pipefd[2];

    if (-1 == (pipe(pipefd))) {
        qWarning("pipe2 failed: %s", ::strerror(errno));
#ifndef Q_OS_MAC
        ::inotify_rm_watch(fd, wd);
        ::close(fd);
#endif
        delete w;
        delete w_inotify;
        return socketfd;
    }

    fcntl(pipefd[0], F_SETFD, FD_CLOEXEC);
    fcntl(pipefd[1], F_SETFD, FD_CLOEXEC);

    qint64 pid = 0;

    // Start the service as a detached process
    if (!(pid = fork())) {
        char buffer[] = "FAIL";
        close(pipefd[0]);

        qServiceLog() << "class" << "doStart"
                      << "event" << "client starting"
                      << "location" << path;

        // child, move it away from the parent
        if (-1 == (setsid())) {
            qWarning("setsit failed: %s", ::strerror(errno));
            write(pipefd[1], buffer, 5);
            exit(-1);
        }

        setpgid(getpid(), getpid());

        execlp(path.toLatin1(), path.toLatin1(), NULL);

        qWarning() << "exec of process" << path.toLatin1() << ::strerror(errno);

        qServiceLog() << "class" << "doStart"
                      << "event" << "failed start"
                      << "error" << ::strerror(errno);

        write(pipefd[1], buffer, 5);
        exit(-1);
    }

    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        qWarning("process start failed %s", ::strerror(errno));
#ifndef Q_OS_MAC
        ::inotify_rm_watch(fd, wd);
        ::close(fd);
#endif
        delete w;
        delete w_inotify;
        return socketfd;
    }

    close(pipefd[1]);

    char buffer[5];
    if (read(pipefd[0], buffer, 5) > 0) {
        qServiceLog() << "class" << "doStart"
                      << "event" << "ddeamon error reported";
        qWarning("process start failed");
#ifndef Q_OS_MAC
        ::inotify_rm_watch(fd, wd);
        ::close(fd);
#endif
        delete w;
        delete w_inotify;
        return socketfd;
    }
    close(pipefd[0]);

    qServiceLog() << "class" << "doStart"
                  << "event" << "deamon started";

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

        ret = ::connect(socketfd, (struct sockaddr *)&name, sizeof(name));
        qServiceLog() << "class" << "qrsrup"
                      << "event" << "connect woke up"
                      << "return" << ret
                      << "errno" << qt_error_string(errno);

        if (ret == 0 || (ret == -1 && errno == EISCONN)) {
            success = true;
            break;
        } else if (ret == -1 && errno == EINPROGRESS) {
            w->setEnabled(true);
        } else {
            w->setEnabled(false);
        }
    }

    qServiceLog() << "func" << __FUNCTION__
                  << "event" << "connect done"
                  << "time" << total_time.elapsed()
                  << "fd" << socketfd;

    qWarning() << QTime::currentTime().toString(fmt) <<
                 "SFW doStart done. Total time" << total_time.elapsed() <<
                 "Socket exists:" << file.exists() <<
                 "isSocketValid" << socketfd;

    if (success == false) {
        ::close(socketfd);
        socketfd = -1;
    }

    delete w;
    delete w_inotify;
#ifndef Q_OS_MAC
    ::inotify_rm_watch(fd, wd);
    ::close(fd);
#endif
    return socketfd;
}

/*
    Creates endpoint on client side.
*/
QObject* QRemoteServiceRegisterPrivate::proxyForService(const QRemoteServiceRegister::Entry& entry, const QString& location)
{
    qWarning() << "(all ok) starting SFW proxyForService" << location;

#ifdef QT_SFW_IPC_DEBUG
    qServiceLog() << "class" << "qrsrp"
                  << "event" << "proxy start"
                  << "iface" << entry.d->iface
                  << "name" << location;
#endif

    int socketfd = doStart(location);

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
            qServiceLog() << "class" << "qrsrp"
                          << "event" << "create object"
                          << "iface" << entry.interfaceName()
                          << "name" << proxy->objectName();
            ipcEndPoint->setParent(proxy);
            endPoint->setParent(proxy);
            qWarning() << "SFW created object for" << entry.interfaceName();
        }
        else {
            qWarning() << "SFW failed to create object for" << entry.interfaceName();
            qServiceLog() << "class" << "qrsrp"
                          << "event" << "create object FAIL"
                          << "iface" << entry.interfaceName();
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
