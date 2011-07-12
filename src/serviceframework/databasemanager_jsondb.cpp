/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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

#include <QDebug>
#include <QHash>

#include <mtcore/jsondb-client.h>
#include <mtcore/jsondb-strings.h>

#include "databasemanager_jsondb_p.h"

QT_BEGIN_NAMESPACE

/*
    \class DatabaseManager
    The database manager is responsible for receiving queries about
    services and managing user and system scope databases in order to
    respond to those queries.

    It provides operations for
    - registering and unregistering services
    - querying for services and interfaces
    - setting and getting default interface implementations

    and provides notifications by emitting signals for added
    or removed services.

    Implementation note:
    When one of the above operations is first invoked a connection with the
    appropriate database(s) is opened.  This connection remains
    open until the DatabaseManager is destroyed.

    If the system scope database cannot be opened when performing
    user scope operations.  The operations are carried out as per normal
    but only acting on the user scope database.  Each operation invokation
    will try to open a connection with the system scope database.

    Terminology note:
    When referring to user scope regarding operations, it generally
    means access to both the user and system databases with the
    data from both combined into a single dataset.
    When referring to a user scope database it means the
    user database only.
*/


const QLatin1String kQuery("query");
const QLatin1String kData("data");

/*
   Constructor
*/
DatabaseManager::DatabaseManager(): db(new JsonDbClient(this))
{
    connect(db, SIGNAL(response(int,const QVariant&)),
        this, SLOT(handleResponse(int,const QVariant&)));
    connect(db, SIGNAL(error(int,int,const QString&)),
        this, SLOT(handleError(int,int,const QString&)));
}

/*
   Destructor
*/
DatabaseManager::~DatabaseManager()
{
}

/*
    Adds the details \a  service into the service database corresponding to
    \a scope.

    Returns true if the operation succeeded and false otherwise.
    The last error is set when this function is called.
*/
bool DatabaseManager::registerService(ServiceMetaDataResults &service, DbScope scope)
{
    Q_UNUSED(scope)
    qDebug() << ":!:" << __FUNCTION__;

    // Check if a service is already registered with the given location
    QServiceInterfaceDescriptor interface;
    foreach (interface, service.interfaces) {
        QVariantMap query;
        query.insert(kQuery, QString::fromLatin1("[?_type=\"interface\"][?identifier=\"%1\"][?service=\"%2\"]").arg(interface.interfaceName()).arg(service.location));
        int id = db->find(query);
        if (!waitForResponse(id)) {
            m_lastError.setError(DBError::InvalidDatabaseFile, QLatin1String("Cannot register service. Unable to contact database."));
            return false;
        }
        QList<QVariant> res = m_data.toMap()[kData].toList();
        if (!res.empty()) {
            QString alreadyRegisteredService = res.first().toMap().value(QLatin1String("0")).toString();
            qDebug() << "Location: " << service.location;
            qDebug() << "Results" << m_data;
            const QString errorText = QLatin1String("Cannot register service \"%1\". Service \"%2\" is already "
                        "registered to service \"%3\".  It must first be deregistered "
                        "before it can be reregistered");

            m_lastError.setError(DBError::LocationAlreadyRegistered,
                errorText.arg(service.name)
                        .arg(service.location)
                        .arg(alreadyRegisteredService));

            qDebug() << "Failed" << m_lastError.text();
            return false;
        }
    }

    QVariantMap query;
    query.insert(kQuery, QString(QLatin1String("[?_type=\"Application\"][?identifier=\"%1\"]")).arg(service.location));
    int id = db->find(query);
    if (!waitForResponse(id)) {
        qWarning() << "Can not find the service registered as an Application with identifier" << service.location;
        qWarning() << "Please check the info.json file is setup properly";
    }

    if (m_data.toMap()[QLatin1String("length")].toInt() == 0){
        qWarning() << "Can not find the service registered as an Application with identifier" << service.location;
        qWarning() << "Please check the info.json file is setup properly";
    }

    foreach (const QServiceInterfaceDescriptor &interface, service.interfaces) {
        QVariantMap interfaceData;
        interfaceData.insert(JsonDbString::kTypeStr, QLatin1String("interface"));
        interfaceData.insert(QLatin1String("identifier"), interface.interfaceName());
        interfaceData.insert(QLatin1String("service"), service.location);
        interfaceData.insert(QLatin1String("name"), service.name);
        interfaceData.insert(QLatin1String("vermaj"), interface.majorVersion());
        interfaceData.insert(QLatin1String("vermin"), interface.minorVersion());
        interfaceData.insert(QLatin1String("description"), interface.attribute(QServiceInterfaceDescriptor::InterfaceDescription));
        QString caps = interface.attribute(QServiceInterfaceDescriptor::Capabilities).toString();
        if (!caps.isEmpty())
            interfaceData.insert(QLatin1String("capabilities"), caps.split(QLatin1Char(',')));
        QVariantMap attData;
        foreach (const QString &attribute, interface.customAttributes()) {
            attData.insert(attribute, interface.customAttribute(attribute));
        }
        if (!attData.isEmpty())
            interfaceData.insert(QLatin1String("attributes"), attData);

        int id = db->create(interfaceData);
        if (!waitForResponse(id)) {
            qDebug() << "Failed to create interface entry";
            return false;
        }
    }

    return true;
}

/*
    Removes the details of \serviceName from the database corresponding to \a
    scope.

    Returns true if the operation succeeded, false otherwise.
    The last error is set when this function is called.
*/
bool DatabaseManager::unregisterService(const QString &serviceName, DbScope scope)
{
    Q_UNUSED(scope)

    qDebug() << ":!:" << __FUNCTION__;

    QVariantMap query;
    query.insert(kQuery, QString::fromLatin1("[?%1=\"interface\"][?service=\"%2\"]")
                 .arg(JsonDbString::kTypeStr)
                 .arg(serviceName));
    int id = db->find(query);
    if (!waitForResponse(id))
        return false;

    QList<QVariant> list = m_data.toMap()[kData].toList();
    if (list.isEmpty()) {
        m_lastError.setError(DBError::NotFound, QString::fromLatin1("Service not found: \"%1\"").arg(serviceName));
        return false;
    }
    foreach (const QVariant &v, list) {
        QVariantMap x;
        x.insert(QLatin1String("_uuid"), v.toMap()[QLatin1String("_uuid")]);
        waitForResponse(db->remove(x));
    }

    return true;
}

/*
    Removes the initialization specific information of \serviceName from the database
    corresponding to a \scope.

    Returns true if the operation succeeded, false otherwise.
    The last error is set when this function is called.
  */
bool DatabaseManager::serviceInitialized(const QString &serviceName, DbScope scope)
{
    Q_UNUSED(scope)
    Q_UNUSED(serviceName)
    qDebug() << ":!:" << __FUNCTION__;
    qDebug() << "Not implemented";
    return false;
}

/*
    Retrieves a list of interface descriptors that fulfill the constraints specified
    by \a filter at a given \a scope.

    The last error is set when this function is called.
*/
QList<QServiceInterfaceDescriptor>  DatabaseManager::getInterfaces(const QServiceFilter &filter, DbScope scope)
{
    Q_UNUSED(scope)
    qDebug() << ":!:" << __FUNCTION__;
    QList<QServiceInterfaceDescriptor> descriptors;

    QVariantMap query;
    if (!filter.serviceName().isEmpty()) {
        query.insert(kQuery, QString::fromLatin1("[?%1=\"interface\"][?service=\"%2\"]")
                     .arg(JsonDbString::kTypeStr)
                     .arg(filter.serviceName()));
    } else if (!filter.interfaceName().isEmpty()) {
        query.insert(kQuery, QString::fromLatin1("[?%1=\"interface\"][?identifier=\"%2\"]")
                     .arg(JsonDbString::kTypeStr)
                     .arg(filter.interfaceName()));
    } else {
        query.insert(kQuery, QString::fromLatin1("[?%1=\"interface\"]")
                     .arg(JsonDbString::kTypeStr));
    }
    int id = db->find(query);
    if (!waitForResponse(id))
        return descriptors;

    QList<QVariant> list = m_data.toMap()[kData].toList();
    foreach (const QVariant &v, list) {
        QVariantMap vMap = v.toMap();
        QString serviceName(vMap[QLatin1String("service")].toString());
        bool match = false;
        if (filter.interfaceName().isEmpty()) {
            match = true;
        }
        else if (filter.interfaceName() == vMap[QLatin1String("identifier")].toString()) {
            match = true;
        }

        if (filter.majorVersion() > 0) {
            if ((filter.versionMatchRule() == QServiceFilter::MinimumVersionMatch && filter.majorVersion() <= vMap[QLatin1String("vermaj")].toInt()) ||
               (filter.versionMatchRule() == QServiceFilter::ExactVersionMatch && filter.majorVersion() == vMap[QLatin1String("vermaj")].toInt()))
                match = true;
            else
                match = false;
        }

        if (filter.minorVersion() > 0) {
            if ((filter.versionMatchRule() == QServiceFilter::MinimumVersionMatch && filter.minorVersion() <= vMap[QLatin1String("vermin")].toInt()) ||
               (filter.versionMatchRule() == QServiceFilter::ExactVersionMatch   && filter.minorVersion() == vMap[QLatin1String("vermin")].toInt()))
                match = true;
            else
                match = false;
        }

        if (!filter.capabilities().isEmpty()) {
            qWarning() << "JsonDB does not support capability matching currently";
        }

        if (!filter.customAttributes().isEmpty()){
            qWarning() << "JsonDB does not support attribute matching currently";
        }

        if (match) {
                QServiceInterfaceDescriptor interface;
                interface.d = new QServiceInterfaceDescriptorPrivate;
                interface.d->interfaceName = vMap[QLatin1String("identifier")].toString();
                interface.d->serviceName = serviceName;
                interface.d->major = vMap[QLatin1String("vermaj")].toInt();
                interface.d->minor = vMap[QLatin1String("vermin")].toInt();
                interface.d->attributes[QServiceInterfaceDescriptor::ServiceType] = QService::InterProcess;
                interface.d->attributes[QServiceInterfaceDescriptor::Location] = vMap[QLatin1String("service")].toString();
                interface.d->attributes[QServiceInterfaceDescriptor::ServiceDescription] = vMap[QLatin1String("description")].toString();

                descriptors.append(interface);
            }
    }
    return descriptors;
}


/*
    Retrieves a list of the names of services that provide the interface
    specified by \a interfaceName.

    The last error is set when this function is called.
*/
QStringList DatabaseManager::getServiceNames(const QString &interfaceName, DatabaseManager::DbScope scope)
{
    qDebug() << ":!:" << __FUNCTION__ << interfaceName << scope;

    QStringList serviceNames;

    // Check if a service is already registered with the given location
    QVariantMap query;
    if (interfaceName.isEmpty())
        query.insert(kQuery, QString::fromLatin1("[?%1=\"interface\"]").arg(JsonDbString::kTypeStr));
    else
        query.insert(kQuery, QString::fromLatin1("[?%1=\"interface\"][?identifier=\"%2\"]").arg(JsonDbString::kTypeStr).arg(interfaceName));
    int id = db->find(query);
    if (!waitForResponse(id)) {
        qDebug() << "Found nothing";
        return serviceNames;
    }

    QList<QVariant> res = m_data.toMap()[kData].toList();
    while (!res.empty()) {
        QMap<QString, QVariant> entry = res.takeFirst().toMap();
        serviceNames.append(entry[QLatin1String("service")].toString());
    }
    qDebug() << "Returning" << serviceNames;
    return serviceNames;

}

/*
    Returns the default interface implementation descriptor for a given
    \a interfaceName and \a scope.

    The last error is set when this function is called.
*/
QServiceInterfaceDescriptor DatabaseManager::interfaceDefault(const QString &interfaceName, DbScope scope)
{
    Q_UNUSED(scope)
    Q_UNUSED(interfaceName)
    qDebug() << ":!:" << __FUNCTION__ << "starting";

    // Mark all interface defaults as not the default
    QVariantMap query;
    query.insert(kQuery, QString::fromLatin1("[?%1=\"interface\"][?identifier=\"%2\"][?default=\"1\"]")
                 .arg(JsonDbString::kTypeStr)
                 .arg(interfaceName));
    int id = db->find(query);
    if (!waitForResponse(id)) {
        qDebug() << "Did query" << query[kQuery].toString();
        m_lastError.setError(DBError::NotFound, QLatin1String("Unable to find interface, or no default interface set."));
        return QServiceInterfaceDescriptor();
    }

    QList<QVariant> res = m_data.toMap()[kData].toList();

    if (res.isEmpty()) {
        m_lastError.setError(DBError::NotFound, QLatin1String("Error fetching interface data."));
        return QServiceInterfaceDescriptor();
    }

    QMap<QString, QVariant> entry = res.takeFirst().toMap();

    QServiceInterfaceDescriptor interface;
    interface.d = new QServiceInterfaceDescriptorPrivate;
    interface.d->interfaceName = entry[QLatin1String("identifier")].toString();
    interface.d->serviceName = entry[QLatin1String("service")].toString();
    interface.d->major = entry[QLatin1String("vermaj")].toInt();
    interface.d->minor = entry[QLatin1String("vermin")].toInt();
    interface.d->attributes[QServiceInterfaceDescriptor::ServiceType] = QService::InterProcess;
    interface.d->attributes[QServiceInterfaceDescriptor::Location] = entry[QLatin1String("service")].toString();
    interface.d->attributes[QServiceInterfaceDescriptor::ServiceDescription] = entry[QLatin1String("description")].toString();

    qDebug() << "Found it";

    return interface;
}

bool lessThan(const QServiceInterfaceDescriptor &d1,
                                        const QServiceInterfaceDescriptor &d2)
{
        return (d1.majorVersion() < d2.majorVersion())
                || ( d1.majorVersion() == d2.majorVersion()
                && d1.minorVersion() < d2.minorVersion());
}

/*
    Sets the default interface implemenation for \a interfaceName to the matching
    interface implementation provided by \a service.

    If \a service provides more than one interface implementation, the newest
    version of the interface is set as the default.

    Returns true if the operation was succeeded, false otherwise
    The last error is set when this function is called.
*/
bool DatabaseManager::setInterfaceDefault(const QString &serviceName, const
        QString &interfaceName, DbScope scope)
{
    qDebug() << ":!:" << __FUNCTION__;
    QList<QServiceInterfaceDescriptor> descriptors;
    QServiceFilter filter;
    filter.setServiceName(serviceName);
    filter.setInterface(interfaceName);

    descriptors = getInterfaces(filter, scope);
    if (m_lastError.code() != DBError::NoError)
        return false;

    if (descriptors.count() == 0) {
        QString errorText = QString::fromLatin1(
                    "No implementation for interface \"%1\" "
                    "found for service \"%2\"");
        m_lastError.setError(DBError::NotFound,
                errorText.arg(interfaceName)
                .arg(serviceName));
        return false;
    }

    //find the descriptor with the latest version
    int latestIndex = 0;
        for (int i = 1; i < descriptors.count(); ++i) {
            if (lessThan(descriptors[latestIndex], descriptors[i]))
                latestIndex = i;
    }

    return setInterfaceDefault(descriptors[latestIndex], scope);
}

/*
    Sets the interface implementation specified by \a descriptor to be the default
    implementation for the particular interface specified in the descriptor.

    Returns true if the operation succeeded, false otherwise.
    The last error is set when this function is called.
*/
bool DatabaseManager::setInterfaceDefault(const QServiceInterfaceDescriptor &descriptor, DbScope scope)
{
    Q_UNUSED(scope)
    qDebug() << ":!:" << __FUNCTION__;

    // Mark all interface defaults as not the default
    QVariantMap query;
    query.insert(kQuery, QString::fromLatin1("[?%1=\"interface\"][?identifier=\"%2\"]")
                 .arg(JsonDbString::kTypeStr)
                 .arg(descriptor.interfaceName()));
    int id = db->find(query);
    if (!waitForResponse(id)) {
        qDebug() << "Found nothing";
        return false;
    }

    QList<QVariant> res = m_data.toMap()[kData].toList();

    while (!res.empty()) {
        QMap<QString, QVariant> entry = res.takeFirst().toMap();
        if (entry.contains(QLatin1String("default"))){
            entry.remove(QLatin1String("default"));
            id = db->update(entry);
            if (!waitForResponse(id)) {
                qDebug() << "Failed to update" << entry;
            }
        }
    }

    // Fetch the entry
    query.clear();
    query.insert(kQuery, QString::fromLatin1("[?%1=\"interface\"][?identifier=\"%2\"][?service=\"%3\"][?vermaj=\"%4\"][?vermin=\"%5\"]")
                 .arg(JsonDbString::kTypeStr)
                 .arg(descriptor.interfaceName())
                 .arg(descriptor.serviceName())
                 .arg(descriptor.majorVersion())
                 .arg(descriptor.minorVersion()));
    id = db->find(query);
    if (!waitForResponse(id)) {
        qDebug() << "Found nothing";
        return false;
    }

    res = m_data.toMap()[kData].toList();

    if (res.empty()){
        qDebug() << "Can't find interface" << descriptor.interfaceName() << " and service " << descriptor.serviceName();
        return false;
    }
    else if (res.count() > 1){
        qWarning() << "Found more than one interface with exactly the same signature, something is wrong" << res.count();
        return false;
    }

    QMap<QString, QVariant> entry = res.takeFirst().toMap();
    entry.insert(QLatin1String("default"), true);
    id = db->update(entry);
    if (!waitForResponse(id)) {
        return false;
    }

    return true;
}

/*
    Sets whether change notifications for added and removed services are
    \a enabled or not at a given \a scope.
*/
void DatabaseManager::setChangeNotificationsEnabled(DbScope scope, bool enabled)
{
    Q_UNUSED(scope)
    Q_UNUSED(enabled)
    qDebug() << ":!:" << __FUNCTION__ << "warning: not implemented";
}

void DatabaseManager::handleResponse(int id, const QVariant& data)
{
    if (id == m_id) {
        m_data = data;
        m_eventLoop.exit(0);
    }
}

void DatabaseManager::handleError(int id, int code, const QString& message)
{
    Q_UNUSED(code)

    if (id == m_id) {
        m_data.clear();
        m_lastError.setError(DBError::SqlError, message);
        m_eventLoop.exit(0);
    }
}

bool DatabaseManager::waitForResponse(int id)
{
    if (m_lastError.code() != DBError::NoError)
        return false;

    m_id = id;
    m_eventLoop.exec(QEventLoop::AllEvents);

    return m_lastError.code() == DBError::NoError;
}

#include "moc_databasemanager_jsondb_p.cpp"

QT_END_NAMESPACE
