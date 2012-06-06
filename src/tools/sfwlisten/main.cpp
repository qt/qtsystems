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
    QUdpSocket *socket;
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

        if (intf.name() == "lo" || intf.name().startsWith("wifi")
            || intf.name().startsWith("wl") || intf.name().startsWith("ppp")
            || intf.name().startsWith("tun") || intf.name().startsWith("tap"))
            continue;

        socket = new QUdpSocket(this);
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
        socket = 0;
        makeSockets();
    }
}

void SFWReceiver::socketReadyRead()
{
    QString intf = socket->multicastInterface().name();

    while (socket->hasPendingDatagrams()) {
        QByteArray dgram;
        dgram.resize(socket->pendingDatagramSize());
        socket->readDatagram(dgram.data(), dgram.size());

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

        QTime t = QTime::currentTime();
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
