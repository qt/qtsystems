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

#include <QUdpSocket>
#include <QHostAddress>
#include <QCoreApplication>
#include <QBuffer>
#include <QDataStream>
#include <QTime>
#include <QTimer>
#include <QNetworkInterface>
#include <cstdio>

/* keep this in sync with qtsystems/src/serviceframework/qservicedebuglog_p.h */
enum DataType {
    Int32Type = 1,
    FloatType = 2,
    StringType = 3
};

class SFWReceiver : public QObject
{
    Q_OBJECT
public:
    SFWReceiver(const QString &intfFilter);

public slots:
    void socketReadyRead();
    void socketVerify();
    void makeSockets();

private:
    QString intfFilter;
    QTimer socketCheck;
    QList<QUdpSocket *> sockets;
};

SFWReceiver::SFWReceiver(const QString &intfFilter) : intfFilter(intfFilter)
{
    connect(&socketCheck, SIGNAL(timeout()), this, SLOT(socketVerify()));
    makeSockets();
}

void SFWReceiver::makeSockets()
{
    QList<QNetworkInterface> intfs = QNetworkInterface::allInterfaces();

    bool gotone = false;
    foreach (const QNetworkInterface &intf, intfs) {
        if (!intfFilter.isEmpty() && !intf.name().startsWith(intfFilter))
            continue;

        if (intf.name().startsWith("wifi")
            || intf.name().startsWith("wl") || intf.name().startsWith("ppp")
            || intf.name().startsWith("tun") || intf.name().startsWith("tap"))
            continue;

        QUdpSocket *socket = new QUdpSocket(this);
        printf("Trying interface %s ...", qPrintable(intf.name()));
        if (!socket->bind(QHostAddress::AnyIPv4, 10520, QUdpSocket::ShareAddress)) {
            printf("Couldn't bind: %s\n", qPrintable(socket->errorString()));
            delete socket;
            continue;
        }
        socket->setMulticastInterface(intf);
        if (!socket->joinMulticastGroup(QHostAddress("224.0.105.201"), intf)) {
            printf("Couldn't join group: %s\n", qPrintable(socket->errorString()));
            delete socket;
            continue;
        }

        printf("ok\n");
        sockets.append(socket);
        if (!gotone) {
            connect(socket, SIGNAL(readyRead()),
                    this, SLOT(socketReadyRead()));
            gotone = true;
        }
    }

    if (!gotone) {
        QTimer::singleShot(200, this, SLOT(makeSockets()));
        socketCheck.stop();
    } else {
        socketCheck.setInterval(1000);
        socketCheck.start();
    }
}

void SFWReceiver::socketVerify()
{
    QList<QNetworkInterface> intfs = QNetworkInterface::allInterfaces();
    bool gotone = false;

    foreach (QUdpSocket *socket, sockets) {
        QString name = socket->multicastInterface().name();

        foreach (const QNetworkInterface &intf, intfs) {
            if (intf.name() == name) {
                gotone = true;
                break;
            }
        }

        if (!gotone) {
            printf("Interface down...\n");
            delete socket;
            sockets.removeAll(socket);
            makeSockets();
        }
    }
}

void SFWReceiver::socketReadyRead()
{
    foreach (QUdpSocket *socket, sockets) {
        QString intf = socket->multicastInterface().name();

        static QByteArray last;

        while (socket->hasPendingDatagrams()) {
            QByteArray dgram;
            dgram.resize(socket->pendingDatagramSize());
            socket->readDatagram(dgram.data(), dgram.size());

            if (dgram == last)
                continue;

            QBuffer buff(&dgram);
            buff.open(QIODevice::ReadOnly);
            QDataStream ds(&buff);

            quint8 hour,minute,second;
            quint16 msec;
            ds >> hour;
            ds >> minute;
            ds >> second;
            ds >> msec;

            qint32 pid;
            ds >> pid;

            char *str;
            uint len;

            ds.readBytes(str, len);
            QByteArray appName(str, len);
            delete[] str;

            printf("{%4s} %2d:%02d:%02d.%03d ", qPrintable(intf), hour, minute,
                   second, msec);
            printf("[%5d/%10s] ", pid, appName.constData());
            while (!ds.atEnd()) {
                ds.readBytes(str, len);
                QByteArray termName(str, len);
                delete[] str;

                printf("{%s, ", termName.constData());

                qint8 type;
                ds >> type;
                DataType dt = static_cast<DataType>(type);
                switch (dt) {
                case Int32Type:
                {
                    qint32 data;
                    ds >> data;
                    printf("%d} ", data);
                }
                    break;
                case FloatType:
                {
                    float data;
                    ds >> data;
                    printf("%.4f} ", data);
                }
                    break;
                case StringType:
                {
                    ds.readBytes(str, len);
                    QByteArray ba(str, len);
                    ba.truncate(35);
                    printf("'%s'} ", ba.constData());
                }
                    break;
                }
            }

            printf("\n");
            last = dgram;
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QString filter;
    if (argc > 1)
        filter = QString::fromLatin1(argv[1]);
    SFWReceiver recv(filter);
    return app.exec();
}

#include "main.moc"
