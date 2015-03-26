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

#include "qremoteserviceregister.h"
#include "qremoteserviceregisterentry_p.h"
#include "ipc/instancemanager_p.h"
#include "qremoteserviceregister_p.h"
#include "qserviceclientcredentials_p.h"

#include <QtCore/QDataStream>
#include <QtCore/QEvent>

QT_BEGIN_NAMESPACE

/*!
    \class QRemoteServiceRegister::Entry
    \ingroup servicefw
    \inmodule QtServiceFramework
    \brief The Entry class represents a remote service entry to be published on QRemoteServiceRegister.

    This class is created using QRemoteServiceRegister::createEntry to supply remote service
    details matching a valid QServiceInterfaceDescriptor.

    A registration entry can then be published for discovery by remote clients.

*/

/*!
    Constructs a null registration entry.
*/
QRemoteServiceRegister::Entry::Entry()
{
    d = new QRemoteServiceRegisterEntryPrivate;
}

/*!
    Constructs the registration entry that is a copy of \a other.
*/
QRemoteServiceRegister::Entry::Entry(const Entry& other)
    : d(other.d)
{
}

/*!
    Destroys the registration entry.
*/
QRemoteServiceRegister::Entry::~Entry()
{
}

/*!
    Checks if the registration entry is currently a valid remote service entry

    Returns true if the serviceName(), interfaceName() and version() point to
    a valid QServiceInterfaceDescriptor, otherwise false.
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
*/
bool QRemoteServiceRegister::Entry::operator==(const Entry& other) const
{
    return d->service == other.d->service &&
           d->iface == other.d->iface &&
           d->ifaceVersion == other.d->ifaceVersion;
}

/*!
    Returns true if this font is different from \a other; otherwise false.
*/
bool QRemoteServiceRegister::Entry::operator!=(const Entry& other) const
{
    return !(other == *this);
}

/*!
    Assigns \a other to this registration entry and returns a reference to it.
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
*/
QString QRemoteServiceRegister::Entry::interfaceName() const
{
    return d->iface;
}

/*!
    Returns the service  name of the registration entry.

    This should correspond to the service name from the service XML description.

    \sa interfaceName(), version()
*/
QString QRemoteServiceRegister::Entry::serviceName() const
{
    return d->service;
}

/*!
    Returns the version of the registration entry in format x.y.

    This should correspond to the interface version from the service XML description.

    \sa interfaceName(), serviceName()
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
*/
void QRemoteServiceRegister::Entry::setInstantiationType(QRemoteServiceRegister::InstanceType type)
{
    d->instanceType = type;
}

/*!
    Returns the QRemoteServiceRegister::InstanceType of the registration entry.
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

    This class registers and publishes IPC based service objects. It owns the service's
    objects and uses the platform specific IPC mechanism to publish the service.

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

    By default all entries are created as \l QRemoteServiceRegister::PrivateInstance
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
*/

/*!
    \fn void QRemoteServiceRegister::allInstancesClosed()

    This signal is emitted whenever all service instances have been closed. This indicates
    that the last connected client has either shutdown or released the loaded service object.

    \sa instanceClosed()
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
*/
QRemoteServiceRegister::QRemoteServiceRegister(QObject* parent)
    : QObject(parent)
    , d(0)
{
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
*/
void QRemoteServiceRegister::publishEntries(const QString& ident)
{
    if (!d) init();
    d->publishServices(ident);
}

/*!
    \property QRemoteServiceRegister::quitOnLastInstanceClosed

    \brief Terminate the service when all clients have closed all objects. Default value is true.
*/
bool QRemoteServiceRegister::quitOnLastInstanceClosed() const
{
    if (!d) const_cast<QRemoteServiceRegister*>(this)->init();
    return d->quitOnLastInstanceClosed();
}

void QRemoteServiceRegister::setQuitOnLastInstanceClosed(bool quit)
{
    if (!d) init();
    d->setQuitOnLastInstanceClosed(quit);
}

/*!
  \since 5.0

  Set the user id for the socket or pipe.  For backends that use sockets or
  pipes and provide filesystem based access control.

  */

void QRemoteServiceRegister::setBaseUserIdentifier(qintptr uid)
{
    if (!d) init();
    d->setBaseUserIdentifier(uid);
}

/*!
  \since 5.0

  Get the user id set on the socket or pipe.
*/

qintptr QRemoteServiceRegister::getBaseUserIdentifier() const
{
    if (!d) const_cast<QRemoteServiceRegister*>(this)->init();
    return d->getBaseUserIdentifier();
}

/*!
  \since 5.0

  Set the group id for the socket or pipe.  For backends that use sockets or
  pipes and provide filesystem based access control.
  */

void QRemoteServiceRegister::setBaseGroupIdentifier(qintptr gid)
{
    if (!d) init();
    d->setBaseGroupIdentifier(gid);
}

/*!
  \since 5.0

  Get the group id set on the socket or pipe.
*/

qintptr QRemoteServiceRegister::getBaseGroupIdentifier() const
{
    if (!d) const_cast<QRemoteServiceRegister*>(this)->init();
    return d->getBaseGroupIdentifier();
}

/*!
    \since 5.0

    Set the socket access control.  This sets the file
    system permissions on that socket.

 */

void QRemoteServiceRegister::setSecurityAccessOptions(SecurityAccessOptions options)
{
    if (!d) init();
    d->setSecurityOptions(options);
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

*/
QRemoteServiceRegister::SecurityFilter QRemoteServiceRegister::setSecurityFilter(QRemoteServiceRegister::SecurityFilter filter)
{
    if (!d) init();
    return d->setSecurityFilter(filter);
}

/*!
    \fn QRemoteServiceRegister::createEntry(const QString& serviceName, const QString& interfaceName, const QString& version)

    Creates an entry on our remote instance manager. The \a serviceName, \a interfaceName and
    \a version must match the service XML descriptor in order for the remote service to be
    discoverable.

    \sa publishEntries()
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

bool QRemoteServiceRegister::event(QEvent *e)
{
    if (!d && e->type() == QEvent::DynamicPropertyChange) {
        QDynamicPropertyChangeEvent *event = static_cast<QDynamicPropertyChangeEvent*>(e);
        if (event->propertyName() == QByteArray("serviceType")) {
            QService::Type serviceType = static_cast<QService::Type>(property("serviceType").toInt());
            d = QRemoteServiceRegisterPrivate::constructPrivateObject(serviceType, this);
        }
    }

    return QObject::event(e);
}

void QRemoteServiceRegister::init()
{
    d = QRemoteServiceRegisterPrivate::constructPrivateObject(this);
}

#ifndef QT_NO_DATASTREAM
Q_SERVICEFW_EXPORT QDataStream& operator>>(QDataStream& s, QRemoteServiceRegister::Entry& entry) {
    //for now we only serialize version, iface and service name
    //needs to sync with qHash and operator==
    s >> entry.d->service >> entry.d->iface >> entry.d->ifaceVersion;
    return s;
}

Q_SERVICEFW_EXPORT QDataStream& operator<<(QDataStream& s, const QRemoteServiceRegister::Entry& entry) {
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

QT_END_NAMESPACE
