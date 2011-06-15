/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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

#include "qremoteserviceregister.h"
#include "qremoteserviceregisterentry_p.h"
#include "ipc/instancemanager_p.h"
#include "qremoteserviceregister_p.h"

QTM_BEGIN_NAMESPACE

/*!
    \class QRemoteServiceRegister::Entry
    \ingroup servicefw
    \inmodule QtServiceFramework
    \brief The Entry class represents a remote service entry to be published on QRemoteServiceRegister.

    This class is created using QRemoteServiceRegister::createEntry to supply remote service
    details matching a valid QServiceInterfaceDescriptor.

    A registration entry can then be published for discovery by remote clients.

    \since 1.1
*/

/*!
    Constructs a null registration entry.
    \since 1.1
*/
QRemoteServiceRegister::Entry::Entry()
{
    d = new QRemoteServiceRegisterEntryPrivate;
}

/*!
    Constructs the registration entry that is a copy of \a other.
    \since 1.1
*/
QRemoteServiceRegister::Entry::Entry(const Entry& other)
    : d(other.d)
{
}

/*!
    Destroys the registration entry.
    \since 1.1
*/
QRemoteServiceRegister::Entry::~Entry()
{
}

/*!
    Checks if the registration entry is currently a valid remote service entry

    Returns true if the serviceName(), interfaceName() and version() point to
    a valid QServiceInterfaceDescriptor, otherwise false.
    \since 1.1
*/
bool QRemoteServiceRegister::Entry::isValid() const
{
    if (!d->iface.isEmpty() && !d->service.isEmpty()
            && !d->ifaceVersion.isEmpty() && d->cptr!=0 && d->meta!=0)
        return true;
    return false;
}

/*!
    Returns true if this font is equal to \a other; otherwise false.
    \since 1.1
*/
bool QRemoteServiceRegister::Entry::operator==(const Entry& other) const
{
    return d->service == other.d->service &&
           d->iface == other.d->iface &&
           d->ifaceVersion == other.d->ifaceVersion;
}

/*!
    Returns true if this font is different from \a other; otherwise false.
    \since 1.1
*/
bool QRemoteServiceRegister::Entry::operator!=(const Entry& other) const
{
    return !(other == *this);
}

/*!
    Assigns \a other to this registration entry and returns a reference to it.
    \since 1.1
*/
QRemoteServiceRegister::Entry &QRemoteServiceRegister::Entry::operator=(const Entry& other)
{
    d = other.d;
    return *this;
}

/*!
    Returns the interface name of the registration entry.

    This should correspond to the interface name from the service XML description.

    \sa serviceName(), version()
    \since 1.1
*/
QString QRemoteServiceRegister::Entry::interfaceName() const
{
    return d->iface;
}

/*!
    Returns the service  name of the registration entry.

    This should correspond to the service name from the service XML description.

    \sa interfaceName(), version()
    \since 1.1
*/
QString QRemoteServiceRegister::Entry::serviceName() const
{
    return d->service;
}

/*!
    Returns the version of the registration entry in format x.y.

    This should correspond to the interface version from the service XML description.

    \sa interfaceName(), serviceName()
    \since 1.1
*/
QString QRemoteServiceRegister::Entry::version() const
{
    return d->ifaceVersion;
}

const QMetaObject * QRemoteServiceRegister::Entry::metaObject() const
{
    return d->meta;
}

/*!
    Sets the QRemoteServiceRegister::InstanceType of the registration entry.

    If this is not explicitly called, the default instance \a type for the registration entry
    is QRemoteServiceRegister::PrivateInstance.
    \since 1.1
*/
void QRemoteServiceRegister::Entry::setInstantiationType(QRemoteServiceRegister::InstanceType type)
{
    d->instanceType = type;
}

/*!
    Returns the QRemoteServiceRegister::InstanceType of the registration entry.
    \since 1.1
*/
QRemoteServiceRegister::InstanceType QRemoteServiceRegister::Entry::instantiationType() const
{
    return d->instanceType;
}

/*!
    \class QRemoteServiceRegister
    \inmodule QtServiceFramework
    \ingroup servicefw
    \brief The QRemoteServiceRegister class manages instances of remote service objects.
    \since 1.1

    This class registers and publishes IPC based service objects. It owns the service's
    objects and uess the platform specific IPC mechanism to publish the service.

    In order for the remote services to be discoverable by QServiceManager each
    QRemoteServiceRegister::Entry must be registered with the same information in
    the XML description, otherwise no corresponding QServiceInterfaceDescriptor can be
    found.

    The following XML descriptor is used for subsequent examples.

    \code
    <SFW version="1.1">
    <service>
        <name>MyService</name>
        <ipcaddress>my_executable</ipcaddress>
        <description>My service example</description>
        <interface>
            <name>com.nokia.qt.example.myService</name>
            <version>1.0</version>
            <description>My private service</description>
            <capabilities></capabilities>
        </interface>
    </service>
    </SFW>
    \endcode

    The snippet belows demonstrates how an application can register the class \a MyClass
    as a remote service, which is published and accessible to clients who wish to load
    service object instances.

    \code
    int main(int argc, char** argv)
    {
        QCoreApplication app(argc, argv);

        QRemoteServiceRegister *serviceRegister = new QRemoteServiceRegister();

        QRemoteServiceRegister::Entry myService;
        myService = serviceRegister->createEntry<MyClass>(
            "MyService", "com.nokia.qt.example.myservice", "1.0");

        serviceRegister->publishEntries("my_service");

        return app.exec();
        delete serviceRegister;
    }
    \endcode

    By default all entries are created as \l QRemoteServiceRegister::GlobalInstance
    types. This can be changed by calling QRemoteServiceRegister::Entry::setInstantiationType()
    on the entry. Once the service register has been published the associated service entries
    can no longer be changed.

    \sa QRemoteServiceRegister::Entry
*/

/*!
    \enum QRemoteServiceRegister::InstanceType
    Defines the two types of instances for a registration entry
    \value GlobalInstance     New requests for a service gets the same service instance
    \value PrivateInstance    New requests for a service gets a new service instance
*/

/*!
    \fn void QRemoteServiceRegister::instanceClosed(const QRemoteServiceRegister::Entry& entry)

    This signal is emitted whenever a created instance has been closed. This indicates
    that a connected client has either shutdown or released the loaded service object.

    \a entry is supplied to identify which registered service
    entry the closed instance belonged to.

    \sa allInstancesClosed()
    \since 1.1
*/

/*!
    \fn void QRemoteServiceRegister::allInstancesClosed()

    This signal is emitted whenever all service instances have been closed. This indicates
    that the last connected client has either shutdown or released the loaded service object.

    \sa instanceClosed()
    \since 1.1
*/

/*!
    \typedef QRemoteServiceRegister::CreateServiceFunc
    \internal
    Denotes a function pointer returning a service instance
*/

/*!
    \typedef QRemoteServiceRegister::SecurityFilter
    \internal
    Denotes a function pointer used for the security filter feature
*/

/*!
    Creates a service register instance with the given \a parent.
    \since 1.1
*/
QRemoteServiceRegister::QRemoteServiceRegister(QObject* parent)
    : QObject(parent)
{
    d = QRemoteServiceRegisterPrivate::constructPrivateObject(this);

    connect(InstanceManager::instance(), SIGNAL(allInstancesClosed()),
            this, SIGNAL(allInstancesClosed()));
    connect(InstanceManager::instance(), SIGNAL(instanceClosed(QRemoteServiceRegister::Entry)),
            this, SIGNAL(instanceClosed(QRemoteServiceRegister::Entry)));
}

/*!
    Destroys the service register instance
*/
QRemoteServiceRegister::~QRemoteServiceRegister()
{
}

/*!
    Publishes every service QRemoteServiceRegister::Entry that has been created using
    \l createEntry(). The \a ident is the service specific IPC address under which
    the service can be reached.

    This address must match the address provided in the services XML descriptor, otherwise
    the service will not be discoverable. In some cases this may also cause the IPC
    rendezvous feature to fail.

    \sa createEntry()
    \since 1.1
*/
void QRemoteServiceRegister::publishEntries(const QString& ident)
{
    d->publishServices(ident);
}

/*!
    \property QRemoteServiceRegister::quitOnLastInstanceClosed

    \brief Terminate the service when all clients have closed all objects. Default value is true.
    \since 1.1
*/
bool QRemoteServiceRegister::quitOnLastInstanceClosed() const
{
    return d->quitOnLastInstanceClosed();
}

void QRemoteServiceRegister::setQuitOnLastInstanceClosed(bool quit)
{
    d->setQuitOnLastInstanceClosed(quit);
}

/*!
    Allows a security filter to be set which can access
    QRemoteServiceRegister::QRemoteServiceRegisterCredentials.

    The \a filter is a function pointer where the function code implements possible
    permission checks and returns true or false. If a connecting client fails the security
    filter it will be denied access and unable to obtain a valid service instance.

    The following snippet is an example of how to use the security filter feature.

    \code
    bool myFunction(const void *p)
    {
        const QRemoteServiceRegisterCredentials *cred =
            (const struct QRemoteServiceRegisterCredentials *)p;

        // allow the superuser
        if (cred->uid == 0)
            return true;

        return false;
    }

    int main(int argc, char** argv)
    {
        ...

        QRemoteServiceRegister* serviceRegister = new QRemoteServiceRegister();
        service->setSecurityFilter(myFunction);

        ...
    }
    \endcode

    \since 1.1
*/
QRemoteServiceRegister::SecurityFilter QRemoteServiceRegister::setSecurityFilter(QRemoteServiceRegister::SecurityFilter filter)
{
    return d->setSecurityFilter(filter);
}

/*!
    \fn QRemoteServiceRegister::createEntry(const QString& serviceName, const QString& interfaceName, const QString& version)

    Creates an entry on our remote instance manager. The \a serviceName, \a interfaceName and
    \a version must match the service XML descriptor in order for the remote service to be
    discoverable.

    \sa publishEntries()
    \since 1.1
*/
QRemoteServiceRegister::Entry QRemoteServiceRegister::createEntry(const QString& serviceName, const QString& interfaceName, const QString& version, QRemoteServiceRegister::CreateServiceFunc cptr, const QMetaObject* meta)
{
    if (serviceName.isEmpty()
            || interfaceName.isEmpty()
            || version.isEmpty() ) {
        qWarning() << "QRemoteServiceRegister::registerService: service name, interface name and version must be specified";
        return Entry();
    }

    Entry e;
    e.d->service = serviceName;
    e.d->iface = interfaceName;
    e.d->ifaceVersion = version;
    e.d->cptr = cptr;
    e.d->meta = meta;

    Q_ASSERT(InstanceManager::instance());
    InstanceManager::instance()->addType(e);

    return e;
}


#ifndef QT_NO_DATASTREAM
QDataStream& operator>>(QDataStream& s, QRemoteServiceRegister::Entry& entry) {
    //for now we only serialize version, iface and service name
    //needs to sync with qHash and operator==
    s >> entry.d->service >> entry.d->iface >> entry.d->ifaceVersion;
    return s;
}

QDataStream& operator<<(QDataStream& s, const QRemoteServiceRegister::Entry& entry) {
    //for now we only serialize version, iface and service name
    //needs to sync with qHash and operator==
    s << entry.d->service << entry.d->iface << entry.d->ifaceVersion;
    return s;
}
#endif

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QRemoteServiceRegister::Entry& entry) {
    dbg.nospace() << "QRemoteServiceRegister::Entry("
                  << entry.serviceName() << ", "
                  << entry.interfaceName() << ", "
                  << entry.version() << ")";
    return dbg.space();
}
#endif

#include "moc_qremoteserviceregister.cpp"

QTM_END_NAMESPACE
