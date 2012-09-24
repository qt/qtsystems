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

#include "qservicedebuglog_p.h"
#include <QDebug>
#include <QTime>

#include <QMutex>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QFileInfo>

#ifdef QT_SFW_IPC_DEBUG
#ifdef Q_OS_UNIX
#include <signal.h>
#include <errno.h>
#endif
#include <QNetworkInterface>
#include <QUdpSocket>
#endif

QT_BEGIN_NAMESPACE

QServiceDebugMessage::QServiceDebugMessage()
{
#ifdef QT_SFW_IPC_DEBUG
    buffer = new QBuffer();
    buffer->open(QIODevice::WriteOnly);
    ds.setDevice(buffer);
    QTime t = QTime::currentTime();
    ds << (quint8)t.hour();
    ds << (quint8)t.minute();
    ds << (quint8)t.second();
    ds << (quint16)t.msec();
    ds << (quint32)(QCoreApplication::applicationPid());
    QFileInfo fi(QCoreApplication::applicationFilePath());
    QByteArray ba = fi.fileName().toLatin1();
    ds.writeBytes(ba.constData(), ba.size());
#endif
}

QServiceDebugMessage::~QServiceDebugMessage()
{
#ifdef QT_SFW_IPC_DEBUG
    /* when we're destructed, we're ready to send! */
    buffer->close();
    QServiceDebugLog::instance()->logMessage(this);
#endif
}

#ifdef QT_SFW_IPC_DEBUG
#include <QHostAddress>
const static QHostAddress _group_addr(QLatin1String("224.0.105.201"));
#endif

QServiceDebugLog::QServiceDebugLog()
{
    makeSockets();
}

#ifdef QT_SFW_IPC_DEBUG
#include <fcntl.h>
#endif

void QServiceDebugLog::makeSockets()
{
#ifdef QT_SFW_IPC_DEBUG
    if (sockets.size() > 0)
        return;

    QList<QNetworkInterface> ifs = QNetworkInterface::allInterfaces();
    foreach (const QNetworkInterface &inf, ifs) {
        /* avoid the loopback or any wireless interfaces
         * (we probably don't want debug over those) */
        if (inf.name().startsWith("wifi")
                || inf.name().startsWith("wl") || inf.name().startsWith("ppp")
                || inf.name().startsWith("tun") || inf.name().startsWith("tap"))
            continue;

        QUdpSocket *socket = new QUdpSocket();
        if (!socket->bind(QHostAddress::AnyIPv4, 10520, QAbstractSocket::ShareAddress)) {
            delete socket;
            continue;
        }
        socket->setMulticastInterface(inf);
        if (!socket->joinMulticastGroup(_group_addr, inf)) {
            delete socket;
            continue;
        }

        int fd = socket->socketDescriptor();
        int flags = ::fcntl(fd, F_GETFL, 0);
        if (flags < 0) {
            delete socket;
            continue;
        }
        flags |= O_NONBLOCK;
        if (::fcntl(fd, F_SETFL, flags)) {
            delete socket;
            continue;
        }

        qDebug("SFW udp debug on interface %s", qPrintable(inf.name()));
        sockets << socket;
    }
#endif
}

void QServiceDebugLog::logMessage(QServiceDebugMessage *msg)
{
#ifdef QT_SFW_IPC_DEBUG
    QMutexLocker m(&socketLock);

    if (sockets.size() == 0) {
        makeSockets();

        if (sockets.size() == 0) {
            queue << msg->buffer;
            return;
        }
    }

    if (sockets.size() > 0) {
        while (!queue.isEmpty()) {
            foreach (QUdpSocket *socket, sockets) {
                int ret = socket->writeDatagram(queue.front()->data(), _group_addr, 10520);
                if (ret == -1 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
                    queue << msg->buffer;
                    return;
                }
            }
            delete queue.front();
            queue.pop_front();
        }
        foreach (QUdpSocket *socket, sockets) {
            int ret = socket->writeDatagram(msg->buffer->data(), _group_addr, 10520);
            if (ret == -1 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
                queue << msg->buffer;
                return;
            }
        }
        delete msg->buffer;
    }
#else
    Q_UNUSED(msg)
#endif
}

QServiceDebugLog *QServiceDebugLog::instance()
{
    static QServiceDebugLog *dbg = 0;
    static QMutex m;
    QMutexLocker l(&m);

    if (!dbg)
        dbg = new QServiceDebugLog();
    return dbg;
}

QT_END_NAMESPACE
