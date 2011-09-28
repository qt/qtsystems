/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#include <QDebug>
#include <QHash>
#include <QCryptographicHash>

//#include <private/jsondb-connection_p.h>
#include <private/jsondb-strings_p.h>
#include <jsondb-client.h>

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


static QString makeHash(const QString& interface, const QString& service, const QString& version) {
    return QString::fromLatin1(QCryptographicHash::hash(QString(interface + service + version).toUtf8(), QCryptographicHash::Md4).toHex());
}

/*
   Constructor
*/
DatabaseManager::DatabaseManager(): db(new JsonDbClient(this)), m_notenabled(false)
{
    connect(db, SIGNAL(response(int,const QVariant&)),
        this, SLOT(handleResponse(int,const QVariant&)));
    connect(db, SIGNAL(error(int,int,const QString&)),
        this, SLOT(handleError(int,int,const QString&)));
    connect(db, SIGNAL(disconnected()),
            this, SLOT(handleDisconnect()));
    connect(db, SIGNAL(notified(QString,QVariant,QString)),
            this, SLOT(handleNotified(QString,QVariant,QString)));

}


/*
   Destructor
*/
DatabaseManager::~DatabaseManager()
{
    if (m_notenabled) {
        setChangeNotificationsEnabled(DatabaseManager::SystemScope, false);
    }
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
    m_lastError.setError(DBError::NoError);

    // Check if a service is already registered
    QServiceInterfaceDescriptor interface;

    foreach (interface, service.interfaces) {

        QString version = QString(QLatin1String("%1.%2")).arg(interface.majorVersion()).arg(interface.minorVersion());

        QString hash = makeHash(interface.interfaceName(),
                                service.name,
                                version);

        QVariantMap query;
        query.insert(kQuery, QString::fromLatin1("[?_type=\"com.nokia.mp.serviceframework.interface\"][?identifier=\"%2\"]").arg(hash));
        int id = db->find(query);
        if (!waitForResponse(id)) {
            qDebug() << "Db error" << m_lastError.text() << query;
            m_lastError.setError(DBError::InvalidDatabaseFile, QLatin1String("Cannot register service. Unable to contact database."));
            return false;
        }
        QList<QVariant> res = m_data.toMap()[kData].toList();
        if (!res.empty()) {
            QString alreadyRegisteredService = res.first().toMap().value(QLatin1String("service")).toString();
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

//    QVariantMap query;
//    query.insert(kQuery, QString(QLatin1String("[?_type=\"com.nokia.mp.core.Package\"][?identifier=\"%1\"]")).arg(service.location));
//    int id = db->find(query);
//    if (!waitForResponse(id)) {
//        qWarning() << "Can not find query the service registered as an Application with identifier" << service.location;
//        qWarning() << "Please check the info.json file is setup properly";
//    }

//    if (m_data.toMap()[QLatin1String("length")].toInt() == 0){
//        qWarning() << "Can not find the service registered as an Application with identifier" << service.location;
//        qWarning() << "Please check the info.json file is setup properly";
//    }

    foreach (const QServiceInterfaceDescriptor &interface, service.interfaces) {
        QVariantMap interfaceData;
        interfaceData.insert(JsonDbString::kTypeStr, QLatin1String("com.nokia.mp.serviceframework.interface"));
        QString version = QString(QLatin1String("%1.%2")).arg(interface.majorVersion()).arg(interface.minorVersion());
        interfaceData.insert(QLatin1String("identifier"), makeHash(interface.interfaceName(),
                                                             service.name,
                                                             version));
        interfaceData.insert(QLatin1String("location"), service.location);
        interfaceData.insert(QLatin1String("service"), service.name);
        interfaceData.insert(QLatin1String("interface"), interface.interfaceName());
        interfaceData.insert(QLatin1String("version"), QString(QLatin1String("%1.%2")).arg(interface.majorVersion()).arg(interface.minorVersion()));
        if (interface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).isValid())
            interfaceData.insert(QLatin1String("description"), interface.attribute(QServiceInterfaceDescriptor::InterfaceDescription));
        if (interface.attribute(QServiceInterfaceDescriptor::ServiceDescription).isValid())
            interfaceData.insert(QLatin1String("servicedescription"), interface.attribute(QServiceInterfaceDescriptor::ServiceDescription));
        if (interface.attribute(QServiceInterfaceDescriptor::ServiceType).isValid())
            interfaceData.insert(QLatin1String("servicetype"), interface.attribute(QServiceInterfaceDescriptor::ServiceType));
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
    m_lastError.setError(DBError::NoError);
//    qDebug() << ":!:" << __FUNCTION__;

    QVariantMap query;
    query.insert(kQuery, QString::fromLatin1("[?%1=\"com.nokia.mp.serviceframework.interface\"][?service=\"%2\"]")
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
    qDebug() << Q_FUNC_INFO;
    Q_UNUSED(scope)
    m_lastError.setError(DBError::NoError);

    // Mark all interface defaults as not the default
    QVariantMap query;
    query.insert(kQuery, QString::fromLatin1("[?%1=\"com.nokia.mp.serviceframework.interface\"][?service=\"%2\"]")
                 .arg(JsonDbString::kTypeStr)
                 .arg(serviceName));

    qDebug() << "Doing" << query << serviceName;

    int id = db->find(query);
    if (!waitForResponse(id)) {
        qDebug() << "Did query" << query[kQuery].toString();
        m_lastError.setError(DBError::NotFound, QLatin1String("Unable to get response from jsondb."));
        return false;
    }

    QList<QVariant> res = m_data.toMap()[kData].toList();

    if (res.isEmpty()) {
        m_lastError.setError(DBError::NotFound, QLatin1String("Unable to find service"));
        return false;
    }

    while (!res.isEmpty()) {

        QVariantMap entry = res.takeFirst().toMap();

        qDebug() << entry;

        if (entry.contains(QLatin1String(SERVICE_INITIALIZED_ATTR))) {
            entry.remove(QLatin1String(SERVICE_INITIALIZED_ATTR));

            int id = db->update(entry);
            if (!waitForResponse(id)) {
                qDebug() << "Failed to update interface entry" << entry.value(QLatin1String("service")).toString() << entry.value(QLatin1String("interface")).toString();
                return false;
            }
        }

    }
    return true;
}

/*
    Retrieves a list of interface descriptors that fulfill the constraints specified
    by \a filter at a given \a scope.

    The last error is set when this function is called.
*/
QList<QServiceInterfaceDescriptor>  DatabaseManager::getInterfaces(const QServiceFilter &filter, DbScope scope)
{
    Q_UNUSED(scope)
    m_lastError.setError(DBError::NoError);
//    qDebug() << ":!:" << __FUNCTION__;
    QList<QServiceInterfaceDescriptor> descriptors;

    QVariantMap query;
    if (!filter.serviceName().isEmpty()) {
        query.insert(kQuery, QString::fromLatin1("[?%1=\"com.nokia.mp.serviceframework.interface\"][?service=\"%2\"]")
                     .arg(JsonDbString::kTypeStr)
                     .arg(filter.serviceName()));
    } else if (!filter.interfaceName().isEmpty()) {
        query.insert(kQuery, QString::fromLatin1("[?%1=\"com.nokia.mp.serviceframework.interface\"][?interface=\"%2\"]")
                     .arg(JsonDbString::kTypeStr)
                     .arg(filter.interfaceName()));
    } else {
        query.insert(kQuery, QString::fromLatin1("[?%1=\"com.nokia.mp.serviceframework.interface\"]")
                     .arg(JsonDbString::kTypeStr));
    }
    int id = db->find(query);
    if (!waitForResponse(id))
        return descriptors;

    QList<QVariant> list = m_data.toMap()[kData].toList();
    foreach (const QVariant &v, list) {
        QVariantMap vMap = v.toMap();
        bool match = false;
        if (filter.interfaceName().isEmpty()) {
            match = true;
        }
        else if (filter.interfaceName() == vMap[QLatin1String("interface")].toString()) {
            match = true;
        }

        if (filter.majorVersion() > 0 || filter.minorVersion() > 0) {
            bool ok;
            float versiondb = vMap[QLatin1String("version")].toString().toFloat(&ok);
            if (!ok) {
                match = false;
            }
            else {
                float versionfl = QString(QLatin1String("%1.%2")).arg(filter.majorVersion()).arg(filter.minorVersion()).toFloat(&ok);
                if ((filter.versionMatchRule() == QServiceFilter::MinimumVersionMatch && versionfl <= versiondb) ||
                    (filter.versionMatchRule() == QServiceFilter::ExactVersionMatch && versionfl == versiondb))
                    match = true;
                else
                    match = false;
            }
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
                interface.d->interfaceName = vMap[QLatin1String("interface")].toString();
                interface.d->serviceName = vMap[QLatin1String("service")].toString();
                interface.d->major = vMap[QLatin1String("version")].toString().split(QLatin1String(".")).at(0).toInt();
                interface.d->minor = vMap[QLatin1String("version")].toString().split(QLatin1String(".")).at(1).toInt();
                if (vMap.contains(QLatin1String("servicetype")))
                    interface.d->attributes[QServiceInterfaceDescriptor::ServiceType] = vMap[QLatin1String("servicetype")].toInt();
                if (vMap.contains(QLatin1String("location")))
                    interface.d->attributes[QServiceInterfaceDescriptor::Location] = vMap[QLatin1String("location")].toString();
                if (vMap.contains(QLatin1String("description")))
                    interface.d->attributes[QServiceInterfaceDescriptor::InterfaceDescription] = vMap[QLatin1String("description")].toString();
                if (vMap.contains(QLatin1String("servicedescription")))
                    interface.d->attributes[QServiceInterfaceDescriptor::ServiceDescription] = vMap[QLatin1String("servicedescription")].toString();

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
    Q_UNUSED(scope)
    m_lastError.setError(DBError::NoError);
    QStringList serviceNames;

    // Check if a service is already registered with the given location
    QVariantMap query;
    if (interfaceName.isEmpty())
        query.insert(kQuery, QString::fromLatin1("[?%1=\"com.nokia.mp.serviceframework.interface\"]").arg(JsonDbString::kTypeStr));
    else
        query.insert(kQuery, QString::fromLatin1("[?%1=\"com.nokia.mp.serviceframework.interface\"][?interface=\"%2\"]").arg(JsonDbString::kTypeStr).arg(interfaceName));
    int id = db->find(query);
    if (!waitForResponse(id)) {
//        qDebug() << "Found nothing";
        return serviceNames;
    }

    QList<QVariant> res = m_data.toMap()[kData].toList();
    while (!res.empty()) {
        QMap<QString, QVariant> entry = res.takeFirst().toMap();
        serviceNames.append(entry[QLatin1String("service")].toString());
    }
    serviceNames.removeDuplicates();
//    qDebug() << "Returning" << serviceNames;
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
    m_lastError.setError(DBError::NoError);

    // Mark all interface defaults as not the default
    QVariantMap query;
    query.insert(kQuery, QString::fromLatin1("[?%1=\"com.nokia.mp.serviceframework.interface\"][?interface=\"%2\"][?default=\"1\"]")
                 .arg(JsonDbString::kTypeStr)
                 .arg(interfaceName));
    int id = db->find(query);
    if (!waitForResponse(id)) {
        qDebug() << "Did query" << query[kQuery].toString();
        m_lastError.setError(DBError::NotFound, QLatin1String("Unable to get response from jsondb."));
        return QServiceInterfaceDescriptor();
    }

    QList<QVariant> res = m_data.toMap()[kData].toList();

    if (res.isEmpty()) {
        query.clear();
        query.insert(kQuery, QString::fromLatin1("[?%1=\"com.nokia.mp.serviceframework.interface\"][?interface=\"%2\"]")
                     .arg(JsonDbString::kTypeStr)
                     .arg(interfaceName));
        id = db->find(query);
        if (!waitForResponse(id)) {
            m_lastError.setError(DBError::NotFound, QLatin1String("Error fetching interface data from jsondb, failed to get result."));
            return QServiceInterfaceDescriptor();
        }
        res = m_data.toMap()[kData].toList();

        if (res.isEmpty()) {
            m_lastError.setError(DBError::NotFound, QLatin1String("Interface not found."));
            return QServiceInterfaceDescriptor();
        }

    }

    QMap<QString, QVariant> entry = res.takeFirst().toMap();

    QServiceInterfaceDescriptor interface;
    interface.d = new QServiceInterfaceDescriptorPrivate;
    interface.d->interfaceName = entry[QLatin1String("interface")].toString();
    interface.d->serviceName = entry[QLatin1String("service")].toString();
    interface.d->major = entry[QLatin1String("version")].toString().split(QLatin1String(".")).at(0).toInt();
    interface.d->minor = entry[QLatin1String("version")].toString().split(QLatin1String(".")).at(1).toInt();
    if (entry.contains(QLatin1String("servicetype")))
        interface.d->attributes[QServiceInterfaceDescriptor::ServiceType] = entry[QLatin1String("servicetype")].toInt();
    if (entry.contains(QLatin1String("location")))
        interface.d->attributes[QServiceInterfaceDescriptor::Location] = entry[QLatin1String("location")].toString();
    if (entry.contains(QLatin1String("description")))
        interface.d->attributes[QServiceInterfaceDescriptor::InterfaceDescription] = entry[QLatin1String("description")].toString();
    if (entry.contains(QLatin1String("servicedescription")))
        interface.d->attributes[QServiceInterfaceDescriptor::ServiceDescription] = entry[QLatin1String("servicedescription")].toString();
    if (entry.contains(QLatin1String("capabilities")))
        interface.d->attributes[QServiceInterfaceDescriptor::Capabilities] = entry[QLatin1String("capabilities")].toString();

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
    m_lastError.setError(DBError::NoError);
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
    m_lastError.setError(DBError::NoError);

    // Mark all interface defaults as not the default
    QVariantMap query;
    query.insert(kQuery, QString::fromLatin1("[?%1=\"com.nokia.mp.serviceframework.interface\"][?interface=\"%2\"]")
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
    QString version = QString(QLatin1String("%1.%2")).arg(descriptor.majorVersion()).arg(descriptor.minorVersion());
    QString hash = makeHash(descriptor.interfaceName(), descriptor.serviceName(), version);
    query.insert(kQuery, QString::fromLatin1("[?%1=\"com.nokia.mp.serviceframework.interface\"][?identifier=\"%2\"]")
                 .arg(JsonDbString::kTypeStr)
                 .arg(hash));
    id = db->find(query);
    if (!waitForResponse(id)) {
        qDebug() << "Found nothing";
        return false;
    }

    res = m_data.toMap()[kData].toList();

    if (res.empty()){
        qDebug() << "Can't find interface" << hash << descriptor.interfaceName() << " and service " << descriptor.serviceName() <<
                    "version" << descriptor.majorVersion() << descriptor.minorVersion();
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

    if (m_notenabled && enabled)
        return;

    m_lastError.setError(DBError::NoError);

    m_notenabled = enabled;

    if (enabled) {
        QVariantMap attrib;
        QVariantList list;
        attrib.insert(JsonDbString::kTypeStr, JsonDbString::kNotificationTypeStr);
        attrib.insert(JsonDbString::kQueryStr,
                      QString(QLatin1String("[?_type=\"com.nokia.mp.serviceframework.interface\"]")));
        list.append(JsonDbString::kCreateStr);
        list.append(JsonDbString::kRemoveStr);
        attrib.insert(JsonDbString::kActionsStr, list);

        int id = db->create(attrib);
        if (!waitForResponse(id)) {
            qDebug() << "Got error create" << m_lastError.text();
        }
        m_notuuid = m_data.toMap().value(QLatin1String("_uuid")).toString();
    }
    else {
        QVariantMap attrib;
        attrib.insert(JsonDbString::kTypeStr, JsonDbString::kNotificationTypeStr);
        attrib.insert(JsonDbString::kQueryStr,
                      QString(QLatin1String("[?_uuid=\"%1\"]")).arg(m_notuuid));

        int id = db->remove(attrib);
        if (!waitForResponse(id)) {
            qDebug() << "Got error remove" << m_lastError.text();
        }

        m_services.clear();
        m_notuuid.clear();
    }

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

void DatabaseManager::handleDisconnect()
{
    m_data.clear();
    m_lastError.setError(DBError::DatabaseNotOpen, QLatin1String("Db connection terminated"));
    m_eventLoop.exit(0);
}

bool DatabaseManager::waitForResponse(int id)
{
    if ((!db->isConnected()) ||
        (m_lastError.code() != DBError::NoError))
        return false;

    m_id = id;
    m_eventLoop.exec(QEventLoop::AllEvents);

    return m_lastError.code() == DBError::NoError;
}

void DatabaseManager::handleNotified( const QString& notify_uuid,
                                    const QVariant& object,
                                    const QString& action )
{
    Q_UNUSED(notify_uuid)

    if (!m_notenabled || notify_uuid != m_notuuid)
        return;

    QVariantMap map = object.toMap();
    QString service = map.value(QLatin1String("service")).toString();

    if (action == QLatin1String("create")) {
        if (!m_services.contains(service)) {
            emit serviceAdded(map.value(QLatin1String("service")).toString(), DatabaseManager::SystemScope);
        }
        m_services.insert(service, m_services.value(service)+1);
    }
    else if (action == QLatin1String("remove")) {
        if (m_services.value(service) == 1) {
            emit serviceRemoved(map.value(QLatin1String("service")).toString(), DatabaseManager::SystemScope);
            m_services.remove(service);
        }
        else {
            m_services.insert(service, m_services.value(service)-1);
        }
    }
}


#include "moc_databasemanager_jsondb_p.cpp"

QT_END_NAMESPACE
