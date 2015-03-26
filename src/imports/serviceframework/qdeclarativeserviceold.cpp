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

#include "qdeclarativeserviceold_p.h"

#include <QQmlEngine>
#include <QQmlInfo>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Service
    \instantiates QDeclarativeService

    \brief The Service element holds an instance of a service object.
    \inherits QObject

    \ingroup qml-serviceframework

    The Service element is part of the Qt ServiceFramework API and
    provides a client instance of the service object. This element is a simplified
    reflection of the QServiceInterfaceDescriptor class that allows the specification of
    the Service::interfaceName to locate the default service implemented at this interface.

    \sa ServiceList
*/
QDeclarativeService::QDeclarativeService()
    : m_serviceInstance(0), m_componentComplete(false)
{
    m_serviceManager = new QServiceManager();
}

QDeclarativeService::~QDeclarativeService()
{
    delete m_serviceInstance;
}

/*!
    \qmlproperty bool Service::valid read-only

    This property holds whether a default service was found at the
    interface name and corresponds to QServiceInterfaceDescriptor::isValid().
*/
bool QDeclarativeService::isValid() const
{
    return m_descriptor.isValid();
}

void QDeclarativeService::setInterfaceDesc(const QServiceInterfaceDescriptor &desc)
{
    if (desc == m_descriptor)
        return;

    m_descriptor = desc;

    if (m_serviceInstance)
        delete m_serviceInstance;
    setServiceObject(0);
}

QServiceInterfaceDescriptor QDeclarativeService::interfaceDesc() const
{
    return m_descriptor;
}

/*!
    \qmlproperty QString Service::interfaceName

    This property holds the interface name of the service that
    corresponds to QServiceInterfaceDescriptor::interfaceName().
*/
void QDeclarativeService::setInterfaceName(const QString &serviceInterface)
{
   m_interface = serviceInterface;
   updateDescriptor();
}

QString QDeclarativeService::interfaceName() const
{
    if (isValid())
        return m_descriptor.interfaceName();
    else
        return "No Interface";
}

/*!
    \qmlproperty QString Service::serviceName

    This property holds the service name of the service that
    corresponds to QServiceInterfaceDescriptor::serviceName().
*/
QString QDeclarativeService::serviceName() const
{
    if (isValid())
        return m_descriptor.serviceName();
    else
        return "No Service";
}

void QDeclarativeService::setServiceName(const QString &service)
{
    m_service = service;
}

/*!
    \qmlproperty int Service::majorVersion

    This property holds the major version number of the service that
    corresponds to QServiceInterfaceDescriptor::majorVersion().
*/
int QDeclarativeService::majorVersion() const
{
    if (isValid())
        return m_descriptor.majorVersion();
    else
        return 0;
}

void QDeclarativeService::setMajorVersion(int version)
{
    m_major = version;
    updateDescriptor();
}

/*!
    \qmlproperty int Service::minorVersion

    This property holds the minor version number of the service that
    corresponds to QServiceInterfaceDescriptor::minorVersion().
*/
int QDeclarativeService::minorVersion() const
{
    if (isValid())
        return m_descriptor.minorVersion();
    else
        return 0;
}

void QDeclarativeService::setMinorVersion(int version)
{
    m_minor = version;
    updateDescriptor();
}

/*!
    \qmlproperty QObject* Service::serviceObject

    This property holds an instance of the service object which
    can be used to make metaobject calls to the service.  This
    corresponds to QServiceManager::loadInterface().
*/
QObject* QDeclarativeService::serviceObject()
{
    if (m_serviceInstance) {
        return m_serviceInstance;
    }

    if (isValid()) {
        QObject *object = m_serviceManager->loadInterface(m_descriptor);
        setServiceObject(object);
        if (!m_serviceInstance) {
            emit error(QLatin1String("Failed to create object"));
            return m_serviceInstance;
        }
        emit serviceObjectChanged();
        connect(m_serviceInstance, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)),
                this, SLOT(IPCFault(QService::UnrecoverableIPCError)));
        m_error.clear();
        return m_serviceInstance;
    } else {
        return 0;
    }
}

/*!
  \qmlproperty QString Service::error

  This property holds the last error the was received, if any

  */

QString QDeclarativeService::lastError() const
{
    return m_error;
}

void QDeclarativeService::IPCFault(QService::UnrecoverableIPCError errorValue)
{
    switch (errorValue) {
    default:
    case QService::ErrorUnknown:
        m_error = QLatin1String("IPC Error: Unkown Error");
        break;
    case QService::ErrorServiceNoLongerAvailable:
        m_error = QLatin1String("IPC Error: Service no longer available");
        break;
    case QService::ErrorOutofMemory:
        m_error = QLatin1String("IPC Error: Out of memory");
        break;
    case QService::ErrorPermissionDenied:
        m_error = QLatin1String("IPC Error: Permission Denied");
        break;
    case QService::ErrorInvalidArguments:
        m_error = QLatin1String("IPC Error: Invalid Arguments");
        break;
    }
    emit error(m_error);
    m_serviceInstance->deleteLater();
    setServiceObject(0);
}

void QDeclarativeService::updateDescriptor()
{
    if (!m_componentComplete)
        return;

    if (m_interface.isEmpty())
        return;

    QServiceInterfaceDescriptor new_desc;

    if (m_minor == 0 && m_major == 0 && m_service.isEmpty()){
        new_desc = m_serviceManager->interfaceDefault(m_interface);
    }
    else {
        QServiceFilter filter;
        if (!m_service.isEmpty())
            filter.setServiceName(m_service);


        if (m_minor != 0 || m_major != 0) {
            const QString version = QString::number(m_major) + "." + QString::number(m_minor);
            filter.setInterface(m_interface, version);
        }

        QList<QServiceInterfaceDescriptor> list = m_serviceManager->findInterfaces(filter);
        if (!list.isEmpty())
            new_desc = list.takeFirst();
    }

    if (new_desc != m_descriptor) {
        m_descriptor = new_desc;
        if (m_serviceInstance)
            emit serviceObjectChanged();
    }

    if (!isValid()) {
        qWarning() << "WARNING: No service found for interface name: " << m_interface << m_service << m_major << m_minor;
    }
}

void QDeclarativeService::setServiceObject(QObject *object)
{
    if (m_serviceInstance != object) {
        m_serviceInstance = object;
        emit serviceObjectChanged();
    }
}

void QDeclarativeService::classBegin()
{

}

void QDeclarativeService::componentComplete()
{
    m_componentComplete = true;
    updateDescriptor();
}

bool QDeclarativeService::operator ==(const QServiceInterfaceDescriptor& other ) const
{
    if ( m_descriptor == other)
        return true;

    return false;
}

/*!
    \qmltype ServiceList
    \instantiates QDeclarativeServiceList

    \brief The ServiceList element holds a list of \l Service elements.
    \inherits QObject

    \ingroup qml-serviceframework

    The ServiceList element is part of the Qt ServiceFramework API and
    provides a list of \l Service elements at the interface ServiceList::interfaceName with
    minimum version match ServiceList::minVersion properties. This list can be used to
    select the desired service and instantiate a service object for access via the QMetaObject.

    This element is a simplified reflection of the QServiceFilter class that provides a list
    of simplified QServiceInterfaceDescriptors. Similarly, if the ServiceList::serviceName
    and ServiceList::versionMatch are not provided they will respectively default to an empty
    string with a minimum verison match.

    \sa Service
*/
QDeclarativeServiceList::QDeclarativeServiceList()
    : m_service(QString()),
      m_interface(),
      m_major(1),
      m_minor(1),
      m_match(QDeclarativeServiceList::Minimum),
      m_componentComplete(false)
{
    serviceManager = new QServiceManager(this);
}

QDeclarativeServiceList::~QDeclarativeServiceList()
{
    while (m_services.count())
        delete m_services.takeFirst();
}
/*!
    \qmlproperty QString ServiceList::serviceName

    This property holds the interface name of the services that
    corresponds to setting QServiceFilter::setService().
*/
void QDeclarativeServiceList::setServiceName(const QString &service)
{
    m_service = service;
    updateFilterResults();
    if (m_componentComplete)
        emit serviceNameChanged();
}

QString QDeclarativeServiceList::serviceName() const
{
    return m_service;
}

/*!
    \qmlproperty QString ServiceList::interfaceName

    This property holds the interface name of the services that
    corresponds to setting QServiceFilter::setInterface().
*/
void QDeclarativeServiceList::setInterfaceName(const QString &serviceInterface)
{
    m_interface = serviceInterface;
    updateFilterResults();
    if (m_componentComplete)
        emit interfaceNameChanged();
}

QString QDeclarativeServiceList::interfaceName() const
{
    return m_interface;
}

/*!
    \qmlproperty int ServiceList::majorVersion

    This property holds the major version number of the service filter that
    corresponds to QServiceFilter::majorVersion().
*/
void QDeclarativeServiceList::setMajorVersion(int major)
{
    m_major = major;
    updateFilterResults();
    if (m_componentComplete)
        emit majorVersionChanged();
}

int QDeclarativeServiceList::majorVersion() const
{
    return m_major;
}

/*!
    \qmlproperty int ServiceList::minorVersion

    This property holds the minor version number of the service filter that
    corresponds to QServiceFilter::minorVersion().
*/
void QDeclarativeServiceList::setMinorVersion(int minor)
{
    m_minor = minor;
    updateFilterResults();
    if (m_componentComplete)
        emit minorVersionChanged();
}

int QDeclarativeServiceList::minorVersion() const
{
    return m_minor;
}

/*!
    \qmlproperty int ServiceList::monitorServiceRegistrations

    This property controls the behaviour of the list when new services are
    registered or deregistered. Setting this property to true means the list
    will be automatically updated when a service is added or removed. Caution,
    your service object will be deleted if the service is unregistered, even if
    the service is still running.
*/

void QDeclarativeServiceList::setMonitorServiceRegistrations(bool updates)
{
    if (updates == false) {
        disconnect(this, SLOT(servicesAddedRemoved()));
    }
    else {
        connect(serviceManager, SIGNAL(serviceAdded(QString,QService::Scope)),
                this, SLOT(servicesAddedRemoved()));
        connect(serviceManager, SIGNAL(serviceRemoved(QString,QService::Scope)),
                this, SLOT(servicesAddedRemoved()));
    }

    if (m_dynamicUpdates != updates)
        emit monitorServiceRegistrationsChanged();

    m_dynamicUpdates = updates;
}

void QDeclarativeServiceList::servicesAddedRemoved()
{
    // invoke in another event loop run
    QMetaObject::invokeMethod(this, "updateServiceList", Qt::QueuedConnection);
}

bool QDeclarativeServiceList::monitorServiceRegistrations() const
{
    return m_dynamicUpdates;
}

/*!
    \qmlproperty enumeration ServiceList::versionMatch

    This property holds the version match rule of the service filter that
    corresponds to QServiceFilter::versionMatchRule(). Within QML the values
    ServiceList.Exact and ServiceList.Minimum correspond to
    QServiceFilter::ExactVersionMatch and QServiceFilter::MinimumVersionMatch
    respectively.
*/
void QDeclarativeServiceList::setVersionMatch(MatchRule match)
{
    m_match = match;
    updateFilterResults();
    if (m_componentComplete)
        emit versionMatchChanged();
}

QDeclarativeServiceList::MatchRule QDeclarativeServiceList::versionMatch() const
{
    return m_match;
}


void QDeclarativeServiceList::updateServiceList()
{
    if (!m_componentComplete)
        return;

    const QString version = QString::number(m_major) + "." + QString::number(m_minor);

    QServiceFilter filter;

    if (!m_service.isEmpty())
        filter.setServiceName(m_service);

    if (m_match == QDeclarativeServiceList::Exact)
        filter.setInterface(m_interface, version, QServiceFilter::ExactVersionMatch);
    else if (!m_interface.isEmpty())
        filter.setInterface(m_interface, version);

    const QList<QServiceInterfaceDescriptor> newlist = serviceManager->findInterfaces(filter);

    QSet<QServiceInterfaceDescriptor> currentServices = QSet<QServiceInterfaceDescriptor>::fromList(m_currentList);
    QSet<QServiceInterfaceDescriptor> newServices = QSet<QServiceInterfaceDescriptor>::fromList(newlist);

    if (currentServices == newServices) {
        return;
    }

    const QSet<QServiceInterfaceDescriptor> addServices = newServices.subtract(currentServices);
    const QSet<QServiceInterfaceDescriptor> delServices = currentServices.subtract(
                QSet<QServiceInterfaceDescriptor>::fromList(newlist));

    foreach (const QServiceInterfaceDescriptor &desc, delServices) {
        foreach (QDeclarativeService *service, m_services) {
            if (*service == desc) {
                m_services.removeOne(service);
                delete service;
            }
        }
        m_currentList.removeOne(desc);
    }

    foreach (const QServiceInterfaceDescriptor &desc, addServices) {
        QDeclarativeService *service = new QDeclarativeService();
        service->setInterfaceDesc(desc);
        m_services.append(service);
        m_currentList.append(desc);
    }

    emit resultsChanged();
}

void QDeclarativeServiceList::updateFilterResults()
{
    if (!m_componentComplete)
        return;

    const QString version = QString::number(m_major) + "." + QString::number(m_minor);

    QServiceFilter filter;

    if (!m_service.isEmpty())
        filter.setServiceName(m_service);

    if (m_match == QDeclarativeServiceList::Exact)
        filter.setInterface(m_interface, version, QServiceFilter::ExactVersionMatch);
    else if (!m_interface.isEmpty())
        filter.setInterface(m_interface, version);

    QList<QServiceInterfaceDescriptor> list = serviceManager->findInterfaces(filter);

    if (list == m_currentList) {
        return;
    }

    m_currentList = list;

    while (m_services.count()) //for now we refresh the entire list
        delete m_services.takeFirst();

    for (int i = 0; i < list.size(); i++) {
        QDeclarativeService *service = new QDeclarativeService();
        service->setInterfaceDesc(list.at(i));
        m_services.append(service);
    }

    emit resultsChanged();
}

/*!
    \qmlproperty QQmlListProperty ServiceList::services

    This property holds the list of \l Service elements that match
    the Service::interfaceName and minimum Service::versionNumber properties.
*/
QQmlListProperty<QDeclarativeService> QDeclarativeServiceList::services()
{
    return QQmlListProperty<QDeclarativeService>(this,
            0,
            s_append,
            s_count,
            s_at,
            s_clear);
}

void QDeclarativeServiceList::classBegin()
{
}

void QDeclarativeServiceList::componentComplete()
{
    if (!m_componentComplete) {
        m_componentComplete = true;
        updateFilterResults();
    }
}

void QDeclarativeServiceList::listUpdated()
{
    if (m_componentComplete)
        emit resultsChanged();
}

void QDeclarativeServiceList::s_append(QQmlListProperty<QDeclarativeService> *prop, QDeclarativeService *service)
{
    QDeclarativeServiceList* list = static_cast<QDeclarativeServiceList*>(prop->object);
    list->m_services.append(service);
    list->listUpdated();
}
int QDeclarativeServiceList::s_count(QQmlListProperty<QDeclarativeService> *prop)
{
    return static_cast<QDeclarativeServiceList*>(prop->object)->m_services.count();
}

QDeclarativeService* QDeclarativeServiceList::s_at(QQmlListProperty<QDeclarativeService> *prop, int index)
{
    return static_cast<QDeclarativeServiceList*>(prop->object)->m_services[index];
}

void QDeclarativeServiceList::s_clear(QQmlListProperty<QDeclarativeService> *prop)
{
    QDeclarativeServiceList* list = static_cast<QDeclarativeServiceList*>(prop->object);
    qDeleteAll(list->m_services);
    list->m_services.clear();
    list->listUpdated();
}
#include "moc_qdeclarativeserviceold_p.cpp"

QT_END_NAMESPACE
