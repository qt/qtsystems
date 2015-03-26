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

#include "qservicemanager.h"
#include "qserviceplugininterface.h"
#include "qserviceinterfacedescriptor_p.h"
#include "qremoteserviceregister_p.h"
#include "qremoteserviceregisterentry_p.h"
#include "qserviceoperations_p.h"
#include "qservicereply.h"
#include "qservicerequest_p.h"
#include "qservicedebuglog_p.h"

#include "databasemanager_p.h"

#include <QObject>
#include <QMetaMethod>
#include <QPluginLoader>
#include <QFile>
#include <QCoreApplication>
#include <QDir>
#include <QSystemSemaphore>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

QString QServiceManager::resolveLibraryPath(const QString &libNameOrPath)
{
    if (QFile::exists(libNameOrPath))
        return libNameOrPath;

    // try to find plug-in via QLibrary
    QStringList paths = QCoreApplication::libraryPaths();
#ifdef QTM_PLUGIN_PATH
    paths << QLatin1String(QTM_PLUGIN_PATH)+QLatin1String("/serviceframework");
#endif
    for (int i=0; i<paths.count(); i++) {
        QString libPath = QDir::toNativeSeparators(paths[i]) + QDir::separator() + libNameOrPath;

        QLibrary lib(libPath);
        if (lib.load()) {
            lib.unload();
            return lib.fileName();
        }
    }
    return QString();
}

class QServicePluginCleanup : public QObject
{
    Q_OBJECT
public:
    QServicePluginCleanup(QPluginLoader *loader, QObject *parent = 0)
        : QObject(parent),
          m_loader(loader)
    {
    }

    ~QServicePluginCleanup()
    {
        if (m_loader) {
            //m_loader->unload();
            delete m_loader;
        }
    }

    QPluginLoader *m_loader;
};

class QServiceManagerPrivate : public QObject
{
    Q_OBJECT
public:
    QServiceManager *manager;
    DatabaseManager *dbManager;
    QServiceOperations *ops;
    QService::Scope scope;
    QServiceManager::Error error;

    QServiceManagerPrivate(QServiceManager *parent = 0)
        : QObject(parent),
          manager(parent),
          dbManager(new DatabaseManager),
          ops(0)
    {
        connect(dbManager, SIGNAL(serviceAdded(QString, DatabaseManager::DbScope)),
                SLOT(serviceAdded(QString, DatabaseManager::DbScope)));
        connect(dbManager, SIGNAL(serviceRemoved(QString, DatabaseManager::DbScope)),
                SLOT(serviceRemoved(QString, DatabaseManager::DbScope)));
    }

    ~QServiceManagerPrivate()
    {
        delete dbManager;
    }

    void setError(QServiceManager::Error err)
    {
        if (error != err)
        {
            error = err;
            manager->errorChanged();
        }
    }

    void setError()
    {
        QServiceManager::Error prev = error;
        switch (dbManager->lastError().code()) {
            case DBError::NoError:
                error = QServiceManager::NoError;
                break;
            case DBError::DatabaseNotOpen:
            case DBError::InvalidDatabaseConnection:
            case DBError::CannotCreateDbDir:
            case DBError::CannotOpenServiceDb:
            case DBError::NoWritePermissions:
            case DBError::InvalidDatabaseFile:
                error = QServiceManager::StorageAccessError;
                break;
            case DBError::LocationAlreadyRegistered:
                error = QServiceManager::ServiceAlreadyExists;
                break;
            case DBError::IfaceImplAlreadyRegistered:
                error = QServiceManager::ImplementationAlreadyExists;
                break;
            case DBError::NotFound:
                error = QServiceManager::ComponentNotFound;
                break;
            case DBError::InvalidDescriptorScope:
                error = QServiceManager::InvalidServiceInterfaceDescriptor;
                break;
            case DBError::SqlError:
            case DBError::IfaceIDNotExternal:
            case DBError::ExternalIfaceIDFound:
            case DBError::UnknownError:
                error = QServiceManager::UnknownError;
                break;
        }
        if (prev != error)
            manager->errorChanged();
    }

private Q_SLOTS:
    void serviceAdded(const QString &service, DatabaseManager::DbScope dbScope)
    {
        QService::Scope s = (dbScope == DatabaseManager::SystemScope ?
                QService::SystemScope : QService::UserScope);
        emit manager->serviceAdded(service, s);
    }

    void serviceRemoved(const QString &service, DatabaseManager::DbScope dbScope)
    {
        QService::Scope s = (dbScope == DatabaseManager::SystemScope ?
                QService::SystemScope : QService::UserScope);
        emit manager->serviceRemoved(service, s);
    }
};

/*!
    \class QServiceManager
    \ingroup servicefw
    \inmodule QtServiceFramework
    \brief The QServiceManager class enables the loading of service plugins
    and the (de)registration of services.

    A service is a stand-alone component that can be used by multiple clients.
    Each service implementation must derive from QObject. Clients request a
    reference to a service via \l loadInterface() or \l loadLocalTypedInterface().

    Services are separate deliveries in the form of plug-ins. New services can be (de)registered
    at any time via \l addService() and \l removeService() respectively. Such an event is
    published via the \l serviceAdded() and \l serviceRemoved() signal.
    Each service plug-in must implement QServicePluginInterface.

    Each plug-in may support multiple interfaces and may even provide multiple implementations
    for the same interface. Individual implementations are identified via
    QServiceInterfaceDescriptor. For a more detailed explanation of services and how they relate to
    interface and their implementations please see QServiceInterfaceDescriptor.

    \sa QServicePluginInterface
*/

/*!
    \enum QServiceManager::Error
    Defines the possible errors for the service manager.

    \value NoError No error occurred.
    \value StorageAccessError The service data storage is not accessible. This could be because the caller does not have the required permissions.
    \value InvalidServiceLocation The service was not found at its specified \l{QServiceInterfaceDescriptor::Location}{location}.
    \value InvalidServiceXml The XML defining the service metadata is invalid.
    \value InvalidServiceInterfaceDescriptor The service interface descriptor is invalid, or refers to an interface implementation that cannot be accessed in the current scope.
    \value ServiceAlreadyExists Another service has previously been registered with the same \l{QServiceInterfaceDescriptor::Location}{location}.
    \value ImplementationAlreadyExists Another service that implements the same interface version has previously been registered.
    \value PluginLoadingFailed The service plugin cannot be loaded.
    \value ComponentNotFound The service or interface implementation has not been registered.
    \value ServiceCapabilityDenied The security session does not permit service access based on its capabilities.
    \value UnknownError An unknown error occurred.
*/

/*!
    \fn void QServiceManager::serviceAdded(const QString& serviceName, QService::Scope scope)

    This signal is emited whenever a new service with the given
    \a serviceName has been registered with the service manager.
    \a scope indicates where the service was added.

    If the manager scope is QService::SystemScope, it will not receive
    notifications about services added in the user scope.

    \sa addService()
*/

/*!
    \fn void QServiceManager::serviceRemoved(const QString& serviceName, QService::Scope scope)

    This signal is emited whenever a service with the given
    \a serviceName has been deregistered with the service manager.
    \a scope indicates where the service was added.

    If the manager scope is QService::SystemScope, it will not receive
    notifications about services removed in the user scope.

    \sa removeService()
*/

/*!
    Creates a service manager with the given \a parent.

    The scope will default to QService::UserScope.

    The service manager will also ensure that a background thread is started to handle
    service manager requests.  If you need to supress this behavior so that all requests
    are handled in the foreground (in the main GUI thread) export the following environment
    variable:
    \code
    env QT_NO_SFW_BACKGROUND_OPERATION /path/to/my_sfw_app
    \endcode
*/
QServiceManager::QServiceManager(QObject *parent)
    : QObject(parent),
      d(new QServiceManagerPrivate(this))
{
    qRegisterMetaType<QService::UnrecoverableIPCError>("QService::UnrecoverableIPCError");
    d->scope = QService::UserScope;
}

/*!
    Creates a service manager with the given \a scope and \a parent.
*/
QServiceManager::QServiceManager(QService::Scope scope, QObject *parent)
    : QObject(parent),
      d(new QServiceManagerPrivate(this))
{
    d->scope = scope;
}

/*!
    Destroys the service manager.
*/
QServiceManager::~QServiceManager()
{
    if (d->ops)
        d->ops->disengage();
    delete d;
}

/*!
    Returns the scope used for registering and searching of services.
*/
QService::Scope QServiceManager::scope() const
{
    return d->scope;
}

/*!
    Returns a list of the services that provide the interface specified by
    \a interfaceName. If \a interfaceName is empty, this function returns
    a list of all available services in this manager's scope.
*/
QStringList QServiceManager::findServices(const QString& interfaceName) const
{
    d->setError(QServiceManager::NoError);
    QStringList services;
    services = d->dbManager->getServiceNames(interfaceName,
            d->scope == QService::SystemScope ? DatabaseManager::SystemScope : DatabaseManager::UserScope);
    d->setError();
    return services;
}

/*!
    Returns a list of the interfaces that match the specified \a filter.
*/
QList<QServiceInterfaceDescriptor> QServiceManager::findInterfaces(const QServiceFilter& filter) const
{
    d->setError(QServiceManager::NoError);
    QList<QServiceInterfaceDescriptor> descriptors = d->dbManager->getInterfaces(filter,
            d->scope == QService::SystemScope ? DatabaseManager::SystemScope : DatabaseManager::UserScope);
    if (descriptors.isEmpty() && d->dbManager->lastError().code() != DBError::NoError) {
        d->setError();
        return QList<QServiceInterfaceDescriptor>();
    }
    return descriptors;
}

/*!
    Returns a list of the interfaces provided by the service named
    \a serviceName. If \a serviceName is empty, this function returns
    a list of all available interfaces in this manager's scope.
*/
QList<QServiceInterfaceDescriptor> QServiceManager::findInterfaces(const QString& serviceName) const
{
    QServiceFilter filter;
    if (!serviceName.isEmpty())
        filter.setServiceName(serviceName);
    return findInterfaces(filter);
}

/*!
    Loads and returns the interface specified by \a interfaceName.

    The caller takes ownership of the returned pointer.

    This function returns a null pointer if the requested service cannot be found.

    \sa setInterfaceDefault(), interfaceDefault()
*/
QObject* QServiceManager::loadInterface(const QString& interfaceName)
{
    return loadInterface(interfaceDefault(interfaceName));
}

/*!
    \internal
    Private helper function to get the QObject for an InterProcess service.
*/
QObject *QServiceManager::loadInterProcessService(const QServiceInterfaceDescriptor &descriptor,
                                                  const QString &serviceLocation) const
{
    //ipc service
    const int majorversion = descriptor.majorVersion();
    const int minorversion = descriptor.minorVersion();
    QString version = QString::number(majorversion) + QLatin1String(".") + QString::number(minorversion);

    QRemoteServiceRegister::Entry serviceEntry;
    serviceEntry.d->iface = descriptor.interfaceName();
    serviceEntry.d->service = descriptor.serviceName();
    serviceEntry.d->ifaceVersion = version;

    QObject* service = QRemoteServiceRegisterPrivate::proxyForService(serviceEntry, serviceLocation);
    if (!service)
        d->setError(QServiceManager::InvalidServiceLocation);

    //client owns proxy object
    return service;
}

/*!
    \internal
    Private helper function to get the QObject for an InProcess (plugin-based) service.
*/
QObject *QServiceManager::loadInProcessService(const QServiceInterfaceDescriptor& descriptor,
                                               const QString &serviceFilePath) const
{
    QScopedPointer<QPluginLoader> loader(new QPluginLoader(serviceFilePath));
    QObject *obj = 0;

    // pluginIFace is same for all service instances of the same plugin
    // calling loader->unload deletes pluginIFace automatically if no other
    // service instance is around
    QServicePluginInterface *pluginIFace = qobject_cast<QServicePluginInterface *>(loader->instance());
    if (pluginIFace) {
        //check initialization first as the service may be a pre-registered one
        bool doLoading = true;
        QString serviceInitialized = descriptor.customAttribute(QLatin1String(SERVICE_INITIALIZED_ATTR));
        if (!serviceInitialized.isEmpty() && (serviceInitialized == QLatin1String("NO"))) {
            // open/create the semaphore using the service's name as identifier
            QSystemSemaphore semaphore(descriptor.serviceName(), 1);
            if (semaphore.error() != QSystemSemaphore::NoError) {
                //try to create it
                semaphore.setKey(descriptor.serviceName(), 1, QSystemSemaphore::Create);
            }
            if (semaphore.error() == QSystemSemaphore::NoError && semaphore.acquire()) {
                pluginIFace->installService();
                DatabaseManager::DbScope scope = d->scope == QService::UserScope ?
                        DatabaseManager::UserOnlyScope : DatabaseManager::SystemScope;
                d->dbManager->serviceInitialized(descriptor.serviceName(), scope);
                semaphore.release();
            } else {
                qWarning() << semaphore.errorString();
                doLoading = false;

            }
        }
        if (doLoading) {
            obj = pluginIFace->createInstance(descriptor);
            if (obj) {
                QServicePluginCleanup *cleanup = new QServicePluginCleanup(loader.take());
                QObject::connect(obj, SIGNAL(destroyed()), cleanup, SLOT(deleteLater()));
            } else {
                qWarning() << "Cannot create object instance for "
                     << descriptor.interfaceName() << ":"
                     << serviceFilePath;
            }
        }
    } else {
        qWarning() << "QServiceManager::loadInterface():" << serviceFilePath << loader->errorString();
    }
    return obj;
}

/*!
    Loads and returns the interface specified by \a descriptor.

    The caller takes ownership of the returned pointer.

    This function returns a null pointer if the requested service cannot be found.

    \sa loadInterfaceRequest()
*/
QObject* QServiceManager::loadInterface(const QServiceInterfaceDescriptor& descriptor)
{
    qServiceLog() << "class" << "QServiceManager"
                  << "event" << "loadInterface"
                  << "interface" << descriptor.interfaceName()
                  << "service" << descriptor.serviceName();

    d->setError(QServiceManager::NoError);
    if (!descriptor.isValid()) {
        d->setError(QServiceManager::InvalidServiceInterfaceDescriptor);
        return 0;
    }

    QObject *obj = 0;
    int serviceType = descriptor.attribute(QServiceInterfaceDescriptor::ServiceType).toInt();
    QString location = descriptor.attribute(QServiceInterfaceDescriptor::Location).toString();

    if (serviceType == QService::InterProcess)
    {
        return loadInterProcessService(descriptor, location);
    }
    else
    {
        QString serviceFilePath = resolveLibraryPath(location);
        if (serviceFilePath.isEmpty())
        {
            d->setError(QServiceManager::InvalidServiceLocation);
            return 0;
        }

        obj = loadInProcessService(descriptor, serviceFilePath);
        if (!obj)
            d->setError(QServiceManager::PluginLoadingFailed);
    }
    return obj;
}

/*!
    \fn QServiceReply *loadInterfaceRequest(const QString &interfaceName)
    Initiate a background request to load the interface specified by \a interfaceName, and
    return a QServiceReply object to track the results of the request.
*/

/*!
    \fn QServiceReply *loadInterfaceRequest(const QServiceInterfaceDescriptor& descriptor)
    Initiate a background request to load the interface specified by \a descriptor, and
    return a QServiceReply object to track the results of the request.
*/


/*!
    \fn QServiceReplyTyped<T> *loadLocalTypedInterfaceRequest(const QString& interfaceName)
    Initiate a background request to load the interface specified by \a interfaceName, and
    return a QServiceReplyTyped object to track the results of the request.
    \code
    // a member variable to track the reply
    QServiceReply<SomeKnownService> *m_reply;

    // make the request
    m_reply = *mgr->loadLocalTypedInterfaceRequest<SomeKnownService>(sksIfaceName);
    connect(m_reply, SIGNAL(finished()), this, SLOT(handleServiceInfo));

    // ...later, in the handler
    SomeKnownService *svc = m_reply->proxyObject();
    \endcode
*/

/*!
    \fn QServiceReplyTyped<T> *loadLocalTypedInterfaceRequest(const QServiceInterfaceDescriptor& descriptor)
    Initiate a background request to load the interface specified by \a interfaceName, and
    return a QServiceReplyTyped object to track the results of the request.
*/

/*!
    \internal
    Initiate a background request to load and return the interface specified by \a interfaceName,
    and using the given \a reply object, which (if non-null) will be a typed sub-class object.
    If it is null a default QServiceReply is constructed.
*/
QServiceReply *QServiceManager::loadInterfaceRequest(const QString &interfaceName)
{
    QServiceReply *reply = new QServiceReply;

    if (!qgetenv("QT_NO_SFW_BACKGROUND_OPERATION").isEmpty())
    {
        qWarning("Turning off sfw background operations as requested.");
        return 0;
    }

    if (!d->ops) {
        d->ops = QServiceOperations::instance();
        d->ops->engage();
    }

    reply->setRequest(interfaceName);

    QServiceRequest req(interfaceName);
    req.setReply(reply);
    req.setScope(scope());
    d->ops->initiateRequest(req);

    return reply;
}

/*!
    \internal
    Initiate a background request to load and return the interface specified by \a descriptor,
    and using the given \a reply object, which (if non-null) will be a typed sub-class object.
    If it is null a default QServiceReply is constructed.
*/
QServiceReply *QServiceManager::loadInterfaceRequest(const QServiceInterfaceDescriptor &descriptor)
{
    QServiceReply *reply = new QServiceReply();

    if (!d->ops) {
        d->ops = QServiceOperations::instance();
        d->ops->engage();
    }

    reply->setRequest(descriptor.interfaceName());

    QServiceRequest req(descriptor);
    req.setReply(reply);
    req.setScope(scope());
    d->ops->initiateRequest(req);

    return reply;
}


/*!
    If \a interfaceName is an out of process service this verifies the
    interface is running.  If the service is in process this function
    always returns false.

    Use this function to verify the interface requested is running. This
    is useful is you only want to call loadInterface when the service
    is already running.  This call does not guarantee that the service
    will remain running, as such a race condition exists if the service
    quits between this call being made and loadInterface being called.

    If the service can not be fount this returns false.

    \sa setInterfaceDefault(), interfaceDefault(), loadInterface()
*/
bool QServiceManager::isInterfaceRunning(const QString& interfaceName)
{
    return isInterfaceRunning(interfaceDefault(interfaceName));
}

/*!
    If \a descriptor is an out of process service this verifies the
    service is running.  If the service is in process this function
    always returns false.

    Use this function to verify the interface requested is running. This
    is useful is you only want to call loadInterface when the service
    is already running.  This call does not guarantee that the service
    will remain running, as such a race condition exists if the service
    quits between this call being made and loadInterface being called.

    If the service can not be found this returns false.  Error is set
    if an error occurs.

*/
bool QServiceManager::isInterfaceRunning(const QServiceInterfaceDescriptor& descriptor)
{
    d->setError(NoError);
    if (!descriptor.isValid()) {
        d->setError(InvalidServiceInterfaceDescriptor);
        return false;
    }

    const QString location = descriptor.attribute(QServiceInterfaceDescriptor::Location).toString();
    const bool isInterProcess = (descriptor.attribute(QServiceInterfaceDescriptor::ServiceType).toInt()
                                == QService::InterProcess);
    if (isInterProcess) {
        //ipc service
        const int majorversion = descriptor.majorVersion();
        const int minorversion = descriptor.minorVersion();
        QString version = QString::number(majorversion) + QLatin1String(".") + QString::number(minorversion);

        QRemoteServiceRegister::Entry serviceEntry;
        serviceEntry.d->iface = descriptor.interfaceName();
        serviceEntry.d->service = descriptor.serviceName();
        serviceEntry.d->ifaceVersion = version;

        return QRemoteServiceRegisterPrivate::isServiceRunning(serviceEntry, location);
    }

    return false;
}



/*!
    \fn T* QServiceManager::loadLocalTypedInterface(const QString& interfaceName)

    Loads the service object implementing \a interfaceName,
    as provided by the default service for this interface.
    The template class must be derived from QObject.

    If \a interfaceName is not a known interface the returned pointer will be null.

    Note that using this function implies that service and client share
    the implementation of T which means that service and client become tightly coupled.
    This may cause issue during later updates as certain changes may require code changes
    to the service and client.

    The caller takes ownership of the returned pointer.

    \sa setInterfaceDefault(), interfaceDefault()
*/


/*!
    \fn T* QServiceManager::loadLocalTypedInterface(const QServiceInterfaceDescriptor& serviceDescriptor)

    Loads the service object identified by \a serviceDescriptor.
    The template class must be derived from QObject.

    If the \a serviceDescriptor is not valid the returned pointer will be null.

    Note that using this function implies that service and client share
    the implementation of T which means that service and client become tightly coupled.
    This may cause issue during later updates as certain changes may require code changes
    to the service and client.

    The caller takes ownership of the returned pointer.

*/

/*!
    Registers the service defined by the XML file at \a xmlFilePath.
    Returns true if the registration succeeded, and false otherwise.

    If a previously unkown interface is added the newly registered service automatically
    becomes the new default service provider for the new interface.

    A service plugin cannot be added if another service is already registered
    with the same plugin file path.  A service plugin also cannot be added if
    the service is already registered and implements any of the same interface
    versions that the new plugin implements.

    \sa removeService(), setInterfaceDefault()
*/
bool QServiceManager::addService(const QString& xmlFilePath)
{
    QFile *f = new QFile(xmlFilePath);
    bool b = addService(f);
    delete f;
    return b;
}

/*!
    Registers the service defined by the XML data from the given \a device.
    Returns true if the registration succeeded, and false otherwise. If a
    previously unkown interface is added the newly registered service
    automatically becomes the new default service provider for the new
    interface.

    Registering a service also causes QServicePluginInterface::installService()
    to be called on the service. If the service plugin is not accessible
    (e.g. if the plugin file is not found) and \c installService() cannot
    be invoked on the service, the registration fails and this method returns
    false.

    A service plugin cannot be added if another service is already registered
    with the same plugin file path.  A service plugin also cannot be added if
    the service is already registered and implements any of the same interface
    versions that the new plugin implements.

    Services are always added based on the \l scope() of the current
    service manager instance.

    \sa removeService(), setInterfaceDefault()
*/
bool QServiceManager::addService(QIODevice *device)
{
    d->setError(QServiceManager::NoError);
    ServiceMetaData parser(device);
    if (!parser.extractMetadata()) {
        d->setError(QServiceManager::InvalidServiceXml);
        return false;
    }
    const ServiceMetaDataResults data = parser.parseResults();

    DatabaseManager::DbScope scope = d->scope == QService::UserScope ?
            DatabaseManager::UserOnlyScope : DatabaseManager::SystemScope;
    ServiceMetaDataResults results = parser.parseResults();

    bool result = d->dbManager->registerService(results, scope);

    if (results.type == QService::InterProcess)
        return result;

    //test the new plug-in
    if (result) {
        QPluginLoader *loader = new QPluginLoader(resolveLibraryPath(data.location));
        QServicePluginInterface *pluginIFace = qobject_cast<QServicePluginInterface *>(loader->instance());
        if (pluginIFace) {
            pluginIFace->installService();
        } else {
            d->setError(QServiceManager::PluginLoadingFailed);
            result = false;
            qWarning() << "QServiceManager::addService()"  << data.location << "->"
                << resolveLibraryPath(data.location) << ":"
                << loader->errorString() << " - Aborting registration";
            d->dbManager->unregisterService(data.name, scope);
        }
        //loader->unload();
        delete loader;
    } else {
        d->setError();
    }

    return result;
}

/*!
    Unregisters the service specified by \a serviceName.

    Returns true if the unregistration succeeded, and false otherwise.

    If a default service implementation is removed and there are other implementations
    for the same interface, the service manager chooses the implementation with the
    highest version number as the new default.  If there is more than one serivce
    with the same version number, the service manager makes a random choice with
    regards to the new default implementation. If this is
    not the desired behaviour the default selection should be updated
    via setInterfaceDefault().

    Services are always removed based on the \l scope() of the current
    service manager instance.

    \sa addService()
*/
bool QServiceManager::removeService(const QString& serviceName)
{
    d->setError(QServiceManager::NoError);
    if (serviceName.isEmpty()) {
        d->setError(QServiceManager::ComponentNotFound);
        return false;
    }

    // Call QServicePluginInterface::uninstallService() on all plugins that
    // match this service

    QSet<QString> pluginPathsSet;
    QList<QServiceInterfaceDescriptor> descriptors = findInterfaces(serviceName);
    for (int i=0; i<descriptors.count(); i++) {
        const QString loc = descriptors[i].attribute(QServiceInterfaceDescriptor::Location).toString();
        const int type = descriptors[i].attribute(QServiceInterfaceDescriptor::ServiceType).toInt();
        //exclude ipc services
        if (type <= QService::Plugin)
            pluginPathsSet << loc;
    }

    QList<QString> pluginPaths = pluginPathsSet.toList();
    for (int i=0; i<pluginPaths.count(); i++) {
        QPluginLoader *loader = new QPluginLoader(resolveLibraryPath(pluginPaths[i]));
        QServicePluginInterface *pluginIFace = qobject_cast<QServicePluginInterface *>(loader->instance());
        if (pluginIFace)
            pluginIFace->uninstallService();
        else
            qWarning() << "QServiceManager: unable to invoke uninstallService() on removed service";
        //loader->unload();
        delete loader;
    }

    if (!d->dbManager->unregisterService(serviceName, d->scope == QService::UserScope ?
            DatabaseManager::UserOnlyScope : DatabaseManager::SystemScope)) {
        d->setError();
        return false;
    }
    return true;
}

/*!
    Sets the default interface implementation for \a interfaceName to the
    matching interface implementation provided by \a service.

    If \a service provides more than one interface implementation for
    \a interfaceName, the newest version of the interface is set as the
    default.

    Returns true if the operation succeeded, and false otherwise.

    \b {Note:} When in system scope, the \a service must be a system-wide
    service rather than a user-specific service; otherwise, this will fail.
*/
bool QServiceManager::setInterfaceDefault(const QString &service, const QString &interfaceName)
{
    d->setError(QServiceManager::NoError);
    if (service.isEmpty() || interfaceName.isEmpty()) {
        d->setError(QServiceManager::ComponentNotFound);
        return false;
    }
    DatabaseManager::DbScope scope = d->scope == QService::SystemScope ?
            DatabaseManager::SystemScope : DatabaseManager::UserScope;
    if (!d->dbManager->setInterfaceDefault(service, interfaceName, scope)) {
        d->setError();
        return false;
    }
    return true;
}

/*!
    \overload

    Sets the interface implementation specified by \a descriptor to be the
    default implementation for the particular interface specified in the
    descriptor.

    Returns true if the operation succeeded, and false otherwise.

    \b {Note:} When in system scope, the \a descriptor must refer to a
    system-wide service rather than a user-specific service; otherwise, this
    will fail.
*/
bool QServiceManager::setInterfaceDefault(const QServiceInterfaceDescriptor& descriptor)
{
    d->setError(QServiceManager::NoError);
    DatabaseManager::DbScope scope = d->scope == QService::SystemScope ?
            DatabaseManager::SystemScope : DatabaseManager::UserScope;
    if (!d->dbManager->setInterfaceDefault(descriptor, scope)) {
        d->setError();
        return false;
    }
    return true;
}

/*!
    Returns the default interface implementation for the given \a interfaceName.
*/
QServiceInterfaceDescriptor QServiceManager::interfaceDefault(const QString& interfaceName) const
{
    qDebug() << "QServiceManager::interfaceDefault" << interfaceName;
    d->setError(QServiceManager::NoError);
    DatabaseManager::DbScope scope = d->scope == QService::SystemScope ?
            DatabaseManager::SystemScope : DatabaseManager::UserScope;
    QServiceInterfaceDescriptor info = d->dbManager->interfaceDefault(interfaceName, scope);
    if (d->dbManager->lastError().code() != DBError::NoError) {
        d->setError();
        qDebug() << "error" << d->dbManager->lastError().text();
        return QServiceInterfaceDescriptor();
    }
    return info;
}

/*!
    Returns the type of error that last occurred.
*/
QServiceManager::Error QServiceManager::error() const
{
    return d->error;
}

/*!
    \internal
*/
void QServiceManager::connectNotify(const QMetaMethod &signal)
{
    static const QMetaMethod serviceAddedSignal = QMetaMethod::fromSignal(&QServiceManager::serviceAdded);
    static const QMetaMethod serviceRemovedSignal = QMetaMethod::fromSignal(&QServiceManager::serviceRemoved);
    if (signal == serviceAddedSignal
            || signal == serviceRemovedSignal) {
        if (d->scope != QService::SystemScope)
            d->dbManager->setChangeNotificationsEnabled(DatabaseManager::UserScope, true);
        d->dbManager->setChangeNotificationsEnabled(DatabaseManager::SystemScope, true);
    }
}

/*!
    \internal
*/
void QServiceManager::disconnectNotify(const QMetaMethod &signal)
{
    static const QMetaMethod serviceAddedSignal = QMetaMethod::fromSignal(&QServiceManager::serviceAdded);
    static const QMetaMethod serviceRemovedSignal = QMetaMethod::fromSignal(&QServiceManager::serviceRemoved);
    if (signal == serviceAddedSignal
            || signal == serviceRemovedSignal) {
        if (!isSignalConnected(serviceAddedSignal)
                && !isSignalConnected(serviceRemovedSignal)) {
            if (d->scope != QService::SystemScope)
                d->dbManager->setChangeNotificationsEnabled(DatabaseManager::UserScope, false);
            d->dbManager->setChangeNotificationsEnabled(DatabaseManager::SystemScope, false);
        }
    }
}

bool QServiceManager::event(QEvent *e)
{
    if (e->type() == QEvent::ThreadChange) {
        qWarning() << "QServiceManager CANNOT BE MOVED THREADS!";
    }

    return QObject::event(e);
}

#include "moc_qservicemanager.cpp"
#include "qservicemanager.moc"
QT_END_NAMESPACE

