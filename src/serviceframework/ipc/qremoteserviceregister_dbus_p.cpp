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
#include "qremoteserviceregister_dbus_p.h"

#include <QDataStream>
#include <QTimer>


QT_BEGIN_NAMESPACE

class DBusEndPoint : public QServiceIpcEndPoint
{
    Q_OBJECT

public:
    DBusEndPoint(QDBusInterface* iface, int type, QObject* parent = 0)
        : QServiceIpcEndPoint(parent), interface(iface), endType(type)
    {
        Q_ASSERT(interface);
        interface->setParent(this);
        connect(interface, SIGNAL(packageReceived(QByteArray,int,QString)),
                this, SLOT(readPackage(QByteArray,int,QString)));

        if (endType == CLIENT) {
            QDBusServiceWatcher *watcher = new QDBusServiceWatcher(interface->service(),
                                                                   interface->connection(),
                                                    QDBusServiceWatcher::WatchForUnregistration);

            QObject::connect(watcher, SIGNAL(serviceUnregistered(QString)),
                             this, SLOT(serviceRemoved(QString)));
        }
    }

    ~DBusEndPoint()
    {
    }

public slots:
    void closeIncoming()
    {
        QDBusMessage msg = interface->callWithArgumentList(QDBus::AutoDetect, QLatin1String("closeIncoming"),
                                                           QList<QVariant>() << instanceId);
    }

    void setInstanceId(const QString& id)
    {
        instanceId = id;
    }

Q_SIGNALS:
    void ipcFault(QService::UnrecoverableIPCError);

protected:
    void flushPackage(const QServicePackage& package)
    {
        if (!QDBusConnection::sessionBus().isConnected()) {
            qWarning() << "Cannot connect to DBus";
        }

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_6);
        out << package;

        packageId = package.d->messageId.toString();
        interface->asyncCall(QLatin1String("writePackage"), block, endType, packageId);
    }

protected slots:
    void readPackage(const QByteArray &package, int type, const QString &id) {
        // Check that its of a client-server nature
        if (endType != type) {
            // Client to Server
            if (type != SERVER) {
                readIncoming(package);
            } else {
            // Server to Client
                if (id == packageId) {
                    readIncoming(package);
                }
            }
        }
    }

    void readIncoming(const QByteArray &package)
    {
        QDataStream data(package);
        QServicePackage pack;
        data >> pack;

        incoming.enqueue(pack);
        emit readyRead();
    }

    void serviceRemoved(const QString& name)
    {
        Q_UNUSED(name);
        QString serviceName = interface->service();
        QDBusReply<bool> reply = interface->connection().interface()->isServiceRegistered(serviceName);
        if (!reply.value()) {
            emit ipcFault(QService::ErrorServiceNoLongerAvailable);
        }
    }

private:
    QDBusInterface* interface;
    QString packageId;
    int endType;
    QString instanceId;
};

class DBusSessionAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.nokia.qtmobility.sfw.DBusSession")

public:
    DBusSessionAdaptor(QObject *parent);
    ~DBusSessionAdaptor() {}

public slots:
    QByteArray writePackage(const QByteArray &package, int type, const QString &id) {
        QByteArray ret;
        QMetaObject::invokeMethod(parent(), "writePackage",
                                  Q_RETURN_ARG(QByteArray, ret),
                                  Q_ARG(QByteArray, package),
                                  Q_ARG(int, type),
                                  Q_ARG(QString, id));
        return ret;
    }

    bool processIncoming() {
        bool ret;
        QMetaObject::invokeMethod(parent(), "processIncoming",
                                  Q_RETURN_ARG(bool, ret));
        return ret;
    }

    void acceptIncoming(bool accept) {
        QMetaObject::invokeMethod(parent(), "acceptIncoming",
                                  Q_ARG(bool, accept));
    }

    void closeIncoming(const QString& instanceId) {
        QMetaObject::invokeMethod(parent(), "closeIncoming",
                                  Q_ARG(QString, instanceId));
    }

signals:
    void packageReceived(const QByteArray &package, int type, const QString &id);
    void newConnection(int pid, int uid);
};

DBusSessionAdaptor::DBusSessionAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

QRemoteServiceRegisterDBusPrivate::QRemoteServiceRegisterDBusPrivate(QObject* parent)
    : QRemoteServiceRegisterPrivate(parent)
{
}

QRemoteServiceRegisterDBusPrivate::~QRemoteServiceRegisterDBusPrivate()
{
}

void QRemoteServiceRegisterDBusPrivate::publishServices(const QString& ident)
{
    if (!createServiceEndPoint(ident))
        QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
}

/*!
    Creates endpoint on service side.
*/
bool QRemoteServiceRegisterDBusPrivate::createServiceEndPoint(const QString& ident)
{
    int endPoints = 0;

    InstanceManager *iManager = InstanceManager::instance();
    QList<QRemoteServiceRegister::Entry> list = iManager->allEntries();

    if (list.size() < 1)
        return false;

    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.isConnected()) {
        qWarning() << "Cannot connect to DBus";
        return 0;
    }

    // Registers the service and session object on DBus if needed
    for (int i=0; i<list.size(); i++) {
        QString serviceName = "com.nokia.qtmobility.sfw." + list[i].serviceName();
        QDBusReply<bool> reply = connection.interface()->isServiceRegistered(serviceName);
        if (reply.value())
            continue;

        if (!connection.registerService(serviceName)) {
            qWarning() << "Cannot register service to DBus:" << serviceName;
            continue;
        }

        // Create and register our DBusSession server/client
        session = new DBusSession(this);
        new DBusSessionAdaptor(session);
        QObject::connect(session, SIGNAL(newConnection(int,int)),
                this, SLOT(processIncoming(int,int)));

        QString path = "/" + list[i].interfaceName() + "/" + ident;
        path.replace(QLatin1String("."), QLatin1String("/"));
        if (!connection.objectRegisteredAt(path)) {
            if (!connection.registerObject(path, session)) {
                qWarning() << "Cannot register service session to DBus:" << path;
                continue;
            }

            iface = new QDBusInterface(serviceName, path, QLatin1String(""), QDBusConnection::sessionBus());
            if (!iface->isValid()) {
                qWarning() << "Cannot connect to remote service" << serviceName << path;;
                continue;
            }

            DBusEndPoint* ipcEndPoint = new DBusEndPoint(iface, SERVER);
            ObjectEndPoint* endPoint = new ObjectEndPoint(ObjectEndPoint::Service, ipcEndPoint, this);

            // Connect session process disconnections
            QObject::connect(session, SIGNAL(closeConnection(QString,QString)),
                             endPoint, SLOT(disconnected(QString,QString)));

            endPoints++;
        }
    }

    if (endPoints > 0)
        return true;

    return false;
}

void QRemoteServiceRegisterDBusPrivate::processIncoming(int pid, int uid)
{
    if (getSecurityFilter()) {
        QRemoteServiceRegisterCredentials cred;
        cred.fd = -1;
        cred.pid = pid;
        cred.uid = uid;
        cred.gid = -1;

        if (!getSecurityFilter()(reinterpret_cast<const void *>(&cred))) {
            session->acceptIncoming(false);

            // Close service if no instances
            if (quitOnLastInstanceClosed() &&
                    InstanceManager::instance()->totalInstances() < 1)
                QCoreApplication::exit();

            return;
        }
    }

    session->acceptIncoming(true);
}

QRemoteServiceRegisterPrivate* QRemoteServiceRegisterPrivate::constructPrivateObject(QObject *parent)
{
    return new QRemoteServiceRegisterDBusPrivate(parent);
}

/*!
    Creates endpoint on client side.
*/
QObject* QRemoteServiceRegisterPrivate::proxyForService(const QRemoteServiceRegister::Entry& entry, const QString& location)
{
    const QString serviceName = "com.nokia.qtmobility.sfw." + entry.serviceName();
    QString path = "/" + entry.interfaceName() + "/" + location;
    path.replace(QLatin1String("."), QLatin1String("/"));

    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.isConnected()) {
        qWarning() << "Cannot connect to DBus";
        return 0;
    }

    // Dummy call to autostart the service if not running
    connection.call(QDBusMessage::createMethodCall(serviceName, path, QLatin1String(""), QLatin1String("q_autostart")));

    QDBusInterface *iface = new QDBusInterface(serviceName, path, QLatin1String(""), QDBusConnection::sessionBus());
    if (!iface->isValid()) {
        qWarning() << "Cannot connect to remote service" << serviceName << path;
        return 0;
    }

    QDBusReply<bool> reply = iface->call(QDBus::Block, QLatin1String("processIncoming"));
    if (reply.value()) {
        DBusEndPoint* ipcEndPoint = new DBusEndPoint(iface, CLIENT);
        ObjectEndPoint* endPoint = new ObjectEndPoint(ObjectEndPoint::Client, ipcEndPoint);

        QObject *proxy = endPoint->constructProxy(entry);
        ipcEndPoint->setInstanceId(endPoint->getInstanceId());

        if (proxy) {
            QObject::connect(proxy, SIGNAL(destroyed()), endPoint, SLOT(deleteLater()));
            QObject::connect(proxy, SIGNAL(destroyed()), ipcEndPoint, SLOT(closeIncoming()));
            QObject::connect(ipcEndPoint, SIGNAL(ipcFault(QService::UnrecoverableIPCError)),
                             proxy, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)));
        }
        return proxy;
    }

    qDebug() << "Insufficient credentials to load a service instance";
    return 0;
}

#include "moc_qremoteserviceregister_dbus_p.cpp"
#include "qremoteserviceregister_dbus_p.moc"
QT_END_NAMESPACE
