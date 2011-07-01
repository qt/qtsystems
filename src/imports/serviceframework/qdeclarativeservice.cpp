/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativeservice_p.h"

#include <QDeclarativeEngine>
#include <QDeclarativeInfo>

QT_BEGIN_NAMESPACE

/*!
    \qmlclass Service QDeclarativeService

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
    : serviceInstance(0)
{
    serviceManager = new QServiceManager();
}

QDeclarativeService::~QDeclarativeService()
{
    delete serviceInstance;
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

    if (serviceInstance)
        delete serviceInstance;

    serviceInstance = 0;
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
void QDeclarativeService::setInterfaceName(const QString &interface)
{
    m_descriptor = serviceManager->interfaceDefault(interface);

    if (!isValid())
        qWarning() << "WARNING: No default service found for interface name: " << interface;
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

/*!
    \qmlproperty QObject* Service::serviceObject

    This property holds an instance of the service object which
    can be used to make metaobject calls to the service.  This
    corresponds to QServiceManager::loadInterface().
*/
QObject* QDeclarativeService::serviceObject()
{
    if (serviceInstance) {
        return serviceInstance;
    }

    if (isValid()) {
        serviceInstance = serviceManager->loadInterface(m_descriptor);
        return serviceInstance;
    } else {
        return 0;
    }
}

/*!
    \qmlclass ServiceList QDeclarativeServiceList

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
void QDeclarativeServiceList::setInterfaceName(const QString &interface)
{
    m_interface = interface;
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

void QDeclarativeServiceList::updateFilterResults()
{
    if (!m_componentComplete)
        return;

    const QString version = QString::number(m_major) + "." + QString::number(m_minor);

    QServiceFilter filter;
    filter.setServiceName(m_service);
    if (m_match == QDeclarativeServiceList::Exact)
        filter.setInterface(m_interface, version, QServiceFilter::ExactVersionMatch);
    else
        filter.setInterface(m_interface, version);

    while (m_services.count()) //for now we refresh the entire list
        delete m_services.takeFirst();


    QList<QServiceInterfaceDescriptor> list = serviceManager->findInterfaces(filter);
    for (int i = 0; i < list.size(); i++) {
        QDeclarativeService *service;
        service = new QDeclarativeService();
        service->setInterfaceDesc(list.at(i));
        m_services.append(service);
    }

    emit resultsChanged();
}

/*!
    \qmlproperty QDeclarativeListProperty ServiceList::services

    This property holds the list of \l Service elements that match
    the Service::interfaceName and minimum Service::versionNumber properties.
*/
QDeclarativeListProperty<QDeclarativeService> QDeclarativeServiceList::services()
{

    return QDeclarativeListProperty<QDeclarativeService>(this,
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

void QDeclarativeServiceList::s_append(QDeclarativeListProperty<QDeclarativeService> *prop, QDeclarativeService *service)
{
    QDeclarativeServiceList* list = static_cast<QDeclarativeServiceList*>(prop->object);
    list->m_services.append(service);
    list->listUpdated();
}
int QDeclarativeServiceList::s_count(QDeclarativeListProperty<QDeclarativeService> *prop)
{
    qmlInfo(0) << "s_count" << static_cast<QDeclarativeServiceList*>(prop->object)->m_services.count();

    return static_cast<QDeclarativeServiceList*>(prop->object)->m_services.count();
}

QDeclarativeService* QDeclarativeServiceList::s_at(QDeclarativeListProperty<QDeclarativeService> *prop, int index)
{
    return static_cast<QDeclarativeServiceList*>(prop->object)->m_services[index];
}

void QDeclarativeServiceList::s_clear(QDeclarativeListProperty<QDeclarativeService> *prop)
{
    QDeclarativeServiceList* list = static_cast<QDeclarativeServiceList*>(prop->object);
    qDeleteAll(list->m_services);
    list->m_services.clear();
    list->listUpdated();
}
#include "moc_qdeclarativeservice_p.cpp"

QT_END_NAMESPACE
