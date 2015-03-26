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

#include "qdeclarativeservice_p.h"

#include <QQmlEngine>
#include <QQmlInfo>

#include <qservicereply.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ServiceLoader
    \instantiates QDeclarativeService

    \brief The ServiceLoader element holds an instance of a service object.
    \inherits QObject
    \inqmlmodule QtServiceFramework
    \ingroup qml-serviceframework

    The ServiceLoader element is part of the Qt ServiceFramework API and
    provides a client instance of the service object. This element allows the specification of
    the Service::interfaceName to locate the default service implemented at this interface.

    To request a service more specifically, you can filter available ServiceDescriptors with
    the ServiceFilter element, and then request service objects based off them.

    Either way, the ServiceLoader element will provide you with the QtObject provided by that service
    interface. You can then use its properties, signals, and slots as defined by its interface.

    Example:
    \code
    import QtQuick 2.0
    import QtServiceFramework 5.0

    QtObject {
        property alias serviceObject: service.serviceObject //In case you want to expose it upwards
        ServiceLoader {
            interfaceName: "com.qt.nokia.example.interface"
            onStatusChanged: {
                if (status == Service.Ready)
                    foo(serviceObject); //In case you want to do something with it as soon as it loads
                else if (status == Service.Error)
                    errorHandling(errorString()); //In case you want to do error handling.
            }
        }
    }
    \endcode

    \sa ServiceList
*/
QDeclarativeServiceLoader::QDeclarativeServiceLoader()
    : m_serviceDescriptor(0), m_status(Null), m_asynchronous(true),
    m_serviceObject(0), m_componentComplete(false), m_serviceManager(0),
    m_serviceReply(0)
{
}

QDeclarativeServiceLoader::~QDeclarativeServiceLoader()
{
    //Manager has qobject ownership
    delete m_serviceObject;//QOBJECT parented?
    delete m_serviceReply;
}

/*!
    \qmlmethod string ServiceLoader::errorString()

    This method returns a human readable description of the last error.

    If the status is not ServiceLoader.Error, errorString() will return an empty string.
*/
/*!
    \qmlproperty Status ServiceLoader::status

    This property contains the status of the service object. It will be one of the following:

    \list
    \li ServiceLoader.Null - the service is inactive or no service has been set
    \li ServiceLoader.Ready - the service has been loaded
    \li ServiceLoader.Loading - the service is currently being loaded
    \li ServiceLoader.Error - an error occurred while loading the service
    \endlist

    If you want to do something immediately after the service loads, the recommended route is
    to monitor this property. For example:
    \code
    ServiceLoader {
        onStatusChanged: {
            if (status == ServiceLoader.Ready)
                doStuffWith(serviceObject)
            else if (status == ServiceLoader.Error)
                console.debug(errorString())
        }
    }
    \endcode

*/
/*!
    \qmlproperty bool ServiceLoader::asynchronous

    If asynchronous is false, then the element will block the main thread until a service object
    is found or an error occurs. This will skip the Loading status. This is generally not
    recommended, as blocking the main thread can lead to significant problems with user interface
    responsiveness.

    Default is true.
*/
/*!
    \qmlproperty string ServiceLoader::interfaceName

    Set this to select a service based off of the interface name. The service name,
    and service version, will be selected for you if a match is found.
*/
/*!
    \qmlproperty ServiceDescriptor* ServiceLoader::serviceDescriptor

    Set this to select a specific service. ServiceDescriptors can be obtained
    from the ServiceFilter element.

*/


/*!
    \qmlproperty QObject* ServiceLoader::serviceObject

    This property holds an instance of the service object which
    can be used to make metaobject calls to the service.

    serviceObject is only valid when the status property is set to
    ServiceLoader.Ready. Otherwise, it should be a null reference.
*/
void QDeclarativeServiceLoader::componentComplete()
{
    if (!m_interfaceName.isEmpty() || m_serviceDescriptor)
        startLoading();
    m_componentComplete = true;
}

void QDeclarativeServiceLoader::startLoading()
{
    if (m_serviceReply) // Cancel pending requests
        delete m_serviceReply; //Auto-disconnects signals

    if (m_serviceObject) {
        m_serviceObject->deleteLater();
        m_serviceObject = 0;
        emit serviceObjectChanged(0);// In sync only, you might get an extra signal
    }

    if (!m_serviceDescriptor && m_interfaceName.isEmpty()) { //Actually an 'unset' request
        setStatus(Null);
        return;
    }

    if (!m_serviceManager)
        m_serviceManager = new QServiceManager(this);

    if (m_asynchronous) {
        if (m_serviceDescriptor)
            m_serviceReply = m_serviceManager->loadInterfaceRequest(*m_serviceDescriptor);
        else
            m_serviceReply = m_serviceManager->loadInterfaceRequest(m_interfaceName);
        connect(m_serviceReply, SIGNAL(finished()),
                this, SLOT(finishLoading()));
        setStatus(Loading);
    } else {
        finishLoading();
    }
}

QString stringForError(QServiceManager::Error error)// ### Should we just expose the enum? Or merely pick better strings?
{
    switch (error) {
    case QServiceManager::NoError: return QLatin1String("No error occurred.");
    case QServiceManager::StorageAccessError: return QLatin1String("Storage access error.");
    case QServiceManager::InvalidServiceLocation: return QLatin1String("Invalid service location.");
    case QServiceManager::InvalidServiceXml: return QLatin1String("Invalid service XML.");
    //case QService::InvalidInterfaceDescriptor: return QLatin1String("Invalid interface descriptor.");
    case QServiceManager::PluginLoadingFailed: return QLatin1String("Error loading service plugin.");
    case QServiceManager::ComponentNotFound: return QLatin1String("Service component not found.");
    case QServiceManager::ServiceCapabilityDenied: return QLatin1String("You do not have permission to access this service capability.");
    default: break;
    }
    return QLatin1String("Unknown error.");
}

void QDeclarativeServiceLoader::finishLoading()
{
    Q_ASSERT(m_serviceManager);
    QServiceManager::Error error;
    QObject* prevObject = m_serviceObject;
    if (m_serviceReply) {
        if (!m_serviceReply->isFinished())
            return; //TODO: Evaluate/handle this error condition
        error = m_serviceReply->error();
        m_serviceObject = m_serviceReply->proxyObject();
        m_serviceReply->deleteLater();
        m_serviceReply = 0;
    } else {
        if (m_asynchronous)
            qDebug() << "Uh oh..."; //TODO: Evaluate/handle this 'error' condition
        if (m_serviceDescriptor)
            m_serviceObject = m_serviceManager->loadInterface(*m_serviceDescriptor);
        else
            m_serviceObject = m_serviceManager->loadInterface(m_interfaceName);
        error = m_serviceManager->error();
    }

    if (error != QServiceManager::NoError) {
        m_serviceObject = 0;
        if (!m_asynchronous)
            emit serviceObjectChanged(0);// In sync we didn't emit the intermediate state
        m_errorString = stringForError(error);
        setStatus(Error);
    } else {
        setStatus(Ready);
        connect(m_serviceObject, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)),
                this, SLOT(IPCFault(QService::UnrecoverableIPCError)));
    }

    if (prevObject != m_serviceObject)
        emit serviceObjectChanged(m_serviceObject);

    delete m_serviceManager;
    m_serviceManager = 0;
}

void QDeclarativeServiceLoader::IPCFault(QService::UnrecoverableIPCError errorValue)
{
    switch (errorValue) {
    default:
    case QService::ErrorUnknown:
        m_errorString = QLatin1String("IPC Error: Unkown Error");
        break;
    case QService::ErrorServiceNoLongerAvailable:
        m_errorString = QLatin1String("IPC Error: Service no longer available");
        break;
    case QService::ErrorOutofMemory:
        m_errorString = QLatin1String("IPC Error: Out of memory");
        break;
    case QService::ErrorPermissionDenied:
        m_errorString = QLatin1String("IPC Error: Permission Denied");
        break;
    case QService::ErrorInvalidArguments:
        m_errorString = QLatin1String("IPC Error: Invalid Arguments");
        break;
    }
    setStatus(Error);
    m_serviceObject->deleteLater();
}

/*!
    \qmltype ServiceDescriptor
    \instantiates QDeclarativeServiceDescriptor

    \brief The ServiceDescriptor element holds a description of a service.
    \inherits QObject

    \ingroup qml-serviceframework

    The ServiceDescriptor element is a simplified reflection of the ServiceDescriptor class,
    and is used merely to contain data that can uniquely identify a service.

    It cannot be created manually, use a ServiceList to search for service descriptions.

    \sa ServiceList
*/
/*!
    \qmlproperty QString ServiceDescriptor::serviceName

    This property holds the interface name of the service.
*/
/*!
    \qmlproperty QString ServiceDescriptor::interfaceName

    This property holds the interface name of the service.
*/

/*!
    \qmlproperty int ServiceDescriptor::majorVersion

    This property holds the major version number of the service.
*/
/*!
    \qmlproperty int ServiceDescriptor::minorVersion

    This property holds the minor version number of the service.
*/
//TODO: Fix doc or remove property
/*!
    \qmlproperty bool ServiceDescriptor::valid

    I have no clue what this property does, but it's probably important.
*/
/*!
    \qmltype ServiceFilter
    \instantiates QDeclarativeServiceFilter

    \brief The ServiceFilter element holds a list of \l ServiceDescriptor objects.
    \inherits QObject

    \ingroup qml-serviceframework

    The ServiceFilter element is part of the Qt ServiceFramework API and
    provides a list of \l QServiceDesciptor objects at the interface ServiceFilter::interfaceName with
    minimum version match ServiceFilter::minVersion properties. This list can be used to
    select the desired service and instantiate a service object for access via the QMetaObject.

    This element is a simplified reflection of the QServiceFilter class that provides a list
    of ServiceDescriptors. Similarly, if the ServiceFilter::serviceName
    and ServiceFilter::versionMatch are not provided they will respectively default to an empty
    string with a minimum verison match.

    Example:
    \code
    Item {
        ServiceFilter {
            id: serviceFilter
            interfaceName: "com.qt.nokia.example.interface"
        }
        ServiceLoader {
            serviceDescriptor: serviceFilter.serviceDescriptions[0] //To get the first matching service
        }
        Repeater{ //To instantiate an object for all matching services.
            model: serviceFilter.serviceDescriptions
            Item {
                ServiceLoader {
                    serviceDescriptor: modelData
                }
            }
        }
    }
    \endcode

    \sa ServiceLoader ServiceDescriptor
*/
QDeclarativeServiceFilter::QDeclarativeServiceFilter(QObject* parent)
    : QObject(parent),
      m_majorVersion(1), // ### Is '1' the correct unset number?
      m_minorVersion(0),
      m_exactVersionMatching(false),
      m_monitorServiceRegistrations(false),
      m_serviceManager(0),
      m_componentComplete(false)
{
}

QDeclarativeServiceFilter::~QDeclarativeServiceFilter()
{
}
/*!
    \qmlproperty QString ServiceFilter::serviceName

    This property holds the interface name of the services that
    corresponds to setting QServiceFilter::setService().
*/
/*!
    \qmlproperty QString ServiceFilter::interfaceName

    This property holds the interface name of the services that
    corresponds to setting QServiceFilter::setInterface().
*/

/*!
    \qmlproperty int ServiceFilter::majorVersion

    This property holds the major version number of the service filter that
    corresponds to QServiceFilter::majorVersion().
*/
/*!
    \qmlproperty int ServiceFilter::minorVersion

    This property holds the minor version number of the service filter that
    corresponds to QServiceFilter::minorVersion().
*/

/*!
    \qmlproperty int ServiceFilter::monitorServiceRegistrations

    This property controls the behaviour of the list when new services are
    registered or deregistered. Setting this property to true means the list
    will be automatically updated when a service is added or removed.

    Continuing to monitor the services can be expensive, so it is recommended
    that you only enable this feature if you need it.

    Caution, your service descriptor object will be deleted if the service
    is unregistered, even if the service is still running. This is primarily
    of note if you are using the serviceDescriptions list as a model with
    Service element delegates.
*/

/*!
    \qmlproperty bool ServiceFilter::versionMatch

    This property holds the version match rule of the service filter that
    corresponds to QServiceFilter::versionMatchRule(). Within QML the values
    ServiceFilter.Exact and ServiceFilter.Minimum correspond to
    QServiceFilter::ExactVersionMatch and QServiceFilter::MinimumVersionMatch
    respectively.
*/

void QDeclarativeServiceFilter::setMonitorServiceRegistrations(bool updates)
{
    if (m_monitorServiceRegistrations == updates)
        return;

    if (updates == false) {
        disconnect(this, SLOT(servicesAddedRemoved()));
        if (m_serviceManager)
            delete m_serviceManager;
        m_serviceManager = 0;
    } else {
        if (!m_serviceManager)
            m_serviceManager = new QServiceManager(this);
        connect(m_serviceManager, SIGNAL(serviceAdded(QString,QService::Scope)),
                this, SLOT(servicesAddedRemoved()));
        connect(m_serviceManager, SIGNAL(serviceRemoved(QString,QService::Scope)),
                this, SLOT(servicesAddedRemoved()));
    }

    emit monitorServiceRegistrationsChanged(updates);
    m_monitorServiceRegistrations = updates;
}

void QDeclarativeServiceFilter::servicesAddedRemoved()
{
    // invoke in another event loop run ### Why?
    QMetaObject::invokeMethod(this, "updateServiceList", Qt::QueuedConnection);
}

QList<QDeclarativeServiceDescriptor> makeDeclarative(QList<QServiceInterfaceDescriptor> in)
{
    QList<QDeclarativeServiceDescriptor> out;
    foreach (const QServiceInterfaceDescriptor &d, in)
        out << QDeclarativeServiceDescriptor(d);
    return out;
}

void QDeclarativeServiceFilter::updateServiceList()
{
    if (!m_componentComplete)
        return;

    if (!m_serviceManager)
        m_serviceManager = new QServiceManager(this);
    const QString version = QString::number(m_majorVersion) + "." + QString::number(m_minorVersion);

    QServiceFilter filter;

    if (!m_serviceName.isEmpty())
        filter.setServiceName(m_serviceName);

    if (!m_interfaceName.isEmpty())
        filter.setInterface(m_interfaceName, version, m_exactVersionMatching ?
                QServiceFilter::ExactVersionMatch : QServiceFilter::MinimumVersionMatch);

    QList<QDeclarativeServiceDescriptor> list = makeDeclarative(m_serviceManager->findInterfaces(filter));

    if (list != m_services) {
        m_services = list;
        emit serviceDescriptionsChanged();
    }

    if (!m_monitorServiceRegistrations) {
        delete m_serviceManager;
        m_serviceManager = 0;
    }

}

/*!
    \qmlproperty QQmlListProperty ServiceFilter::serviceDescriptions

    This property holds the list of \l ServiceDescriptor objects that match
    the ServiceFilter::interfaceName and minimum ServiceFilter::versionNumber properties.
*/

void QDeclarativeServiceFilter::componentComplete()
{
    m_componentComplete = true;
    updateServiceList();
}

void QDeclarativeServiceFilter::s_append(QQmlListProperty<QDeclarativeServiceDescriptor> *prop, QDeclarativeServiceDescriptor *service)
{
    QDeclarativeServiceFilter* list = static_cast<QDeclarativeServiceFilter*>(prop->object);
    list->m_services.append(*service);//### This does not maintain the reference
    list->serviceDescriptionsChanged();
}
int QDeclarativeServiceFilter::s_count(QQmlListProperty<QDeclarativeServiceDescriptor> *prop)
{
    return static_cast<QDeclarativeServiceFilter*>(prop->object)->m_services.count();
}

QDeclarativeServiceDescriptor* QDeclarativeServiceFilter::s_at(QQmlListProperty<QDeclarativeServiceDescriptor> *prop, int index)
{
    return &(static_cast<QDeclarativeServiceFilter*>(prop->object)->m_services[index]);
}

void QDeclarativeServiceFilter::s_clear(QQmlListProperty<QDeclarativeServiceDescriptor> *prop)
{
    QDeclarativeServiceFilter* list = static_cast<QDeclarativeServiceFilter*>(prop->object);
    list->m_services.clear();
    list->serviceDescriptionsChanged();
}
#include "moc_qdeclarativeservice_p.cpp"

QT_END_NAMESPACE
