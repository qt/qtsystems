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

#include "qservicedebuglog_p.h"
#include <QDebug>
#include <QTime>

#include <QMutex>
#include <QMutexLocker>
#include <QUdpSocket>
#include <QCoreApplication>
#include <QFileInfo>
#include <QNetworkInterface>

#ifdef Q_OS_UNIX
#include <signal.h>
#endif

QT_BEGIN_NAMESPACE

QServiceDebugMessage::QServiceDebugMessage()
{
    buffer = new QBuffer();
    buffer->open(QIODevice::WriteOnly);
    ds.setDevice(buffer);
    ds << (quint32)(QCoreApplication::applicationPid());
    QFileInfo fi(QCoreApplication::applicationFilePath());
    QByteArray ba = fi.fileName().toLatin1();
    ds.writeBytes(ba.constData(), ba.size());
}

QServiceDebugMessage::~QServiceDebugMessage()
{
    /* when we're destructed, we're ready to send! */
    buffer->close();
    QServiceDebugLog::instance()->logMessage(this);
}

const static QHostAddress _group_addr("224.0.105.201");

QServiceDebugLog::QServiceDebugLog()
{
    makeSockets();
}

void QServiceDebugLog::makeSockets()
{
#ifdef QT_SFW_IPC_DEBUG
    if (sockets.size() > 0)
        return;

    QList<QNetworkInterface> ifs = QNetworkInterface::allInterfaces();
    foreach (const QNetworkInterface &inf, ifs) {
        /* avoid the loopback or any wireless interfaces
         * (we probably don't want debug over those) */
        if (inf.name() == "lo" || inf.name().startsWith("wifi")
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

        if (sockets.size() > 0) {
            while (!queue.isEmpty()) {
                foreach (QUdpSocket *socket, sockets)
                    socket->writeDatagram(queue.front()->data(), _group_addr, 10520);
                delete queue.front();
                queue.pop_front();
            }
        } else {
            queue << msg->buffer;
        }
    }

    if (sockets.size() > 0) {
        foreach (QUdpSocket *socket, sockets) {
            socket->writeDatagram(msg->buffer->data(), _group_addr, 10520);
        }
        delete msg->buffer;
    }
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
