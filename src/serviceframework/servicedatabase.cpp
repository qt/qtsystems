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

// #define QT_SFW_SERVICEDATABASE_DEBUG

#include "servicedatabase_p.h"
#include <QDir>
#include <QSet>
#include "qserviceinterfacedescriptor.h"
#include "qserviceinterfacedescriptor_p.h"
#include <QUuid>
#include "dberror_p.h"

//database name
#define RESOLVERDATABASE "services.db"

//database table names
#define SERVICE_TABLE "Service"
#define INTERFACE_TABLE "Interface"
#define DEFAULTS_TABLE "Defaults"
#define SERVICE_PROPERTY_TABLE "ServiceProperty"
#define INTERFACE_PROPERTY_TABLE "InterfaceProperty"

//separator
#define RESOLVERDATABASE_PATH_SEPARATOR "//"

#ifdef QT_SFW_SERVICEDATABASE_DEBUG
#include <QDebug>
#endif

#define SERVICE_DESCRIPTION_KEY "DESCRIPTION"
#ifdef QT_SFW_SERVICEDATABASE_USE_SECURITY_TOKEN
#define SECURITY_TOKEN_KEY "SECURITYTOKEN"
#endif
#define INTERFACE_DESCRIPTION_KEY "DESCRIPTION"
#define SERVICE_INITIALIZED_KEY SERVICE_INITIALIZED_ATTR
#define INTERFACE_CAPABILITY_KEY "CAPABILITIES"
#define INTERFACE_SERVICETYPE_KEY "SERVICETYPE"

//service prefixes
#define SERVICE_IPC_PREFIX "_q_ipc_addr:"

QT_BEGIN_NAMESPACE

enum TBindIndexes
    {
        EBindIndex=0,
        EBindIndex1,
        EBindIndex2,
        EBindIndex3,
        EBindIndex4,
        EBindIndex5,
        EBindIndex6,
        EBindIndex7
    };


/*
   \class ServiceDatabase
   The ServiceDatabase is responsible for the management of a single
   service database.  It provides operations for:
   - opening and closing a connection with the database,
   - registering and unregistering services
   - querying for services and interfaces
   - setting and getting default interface implementations.
*/

/*
    Constructor
*/
ServiceDatabase::ServiceDatabase(void)
:m_isDatabaseOpen(false),m_inTransaction(false)
{
}

/*
    Destructor
*/
ServiceDatabase::~ServiceDatabase()
{
    close();
}

/*
    Opens the service database
    The method creates or opens database and creates tables if they are not present
    Returns true if the operation was successful, false if not.
*/
bool ServiceDatabase::open()
{
    if (m_isDatabaseOpen)
        return true;

    QString path;

    //Create full path to database
    if (m_databasePath.isEmpty ())
        m_databasePath = databasePath();

    path = m_databasePath;
    QFileInfo dbFileInfo(path);
    if (!dbFileInfo.dir().exists()) {
       QDir::root().mkpath(dbFileInfo.path());
       QFile file(path);
       if (!file.open(QIODevice::ReadWrite)) {
           QString errorText(QLatin1String("Could not create database directory: %1"));
           m_lastError.setError(DBError::CannotCreateDbDir, errorText.arg(dbFileInfo.path()));
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
           qWarning() << "ServiceDatabase::open():-"
                        << "Problem:" << qPrintable(m_lastError.text());
#endif
           close();
           return false;
        }
        file.close();
    }

    m_connectionName = dbFileInfo.completeBaseName() + QStringLiteral("--") + QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    QSqlDatabase  database;
    if (QSqlDatabase::contains(m_connectionName)) {
        database = QSqlDatabase::database(m_connectionName);
    } else {
        database = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), m_connectionName);
        database.setDatabaseName(path);
    }

    if (!database.isValid()){
        m_lastError.setError(DBError::InvalidDatabaseConnection);
        close();
        return false;
    }

    //Create or open database
    if (!database.isOpen()) {
        if (!database.open()) {
            m_lastError.setError(DBError::SqlError, database.lastError().text());
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::open():-"
                        << "Problem:" << "Could not open database. "
                        << "Reason:" << m_lastError.text();
#endif
            close();
            return false;
        }
    }
    m_isDatabaseOpen = true;

    //Check database structure (tables) and recreate tables if neccessary
    //If one of tables is missing remove all tables and recreate them
    //This operation is required in order to avoid data coruption
    if (!checkTables()) {
        if (dropTables()) {
            if (createTables()) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
                qDebug() << "ServiceDatabase::open():-"
                    << "Database tables recreated";
#endif
            } else {
                //createTable() should've handled error message
                //and warning
                close();
                return false;
            }
        }
        else {
            //dropTables() should've handled error message
            //and warning
            close();
            return false;
        }
    }
    return true;
}

/*
   Adds a \a service into the database.

   May set the following error codes
   DBError::NoError
   DBError::LocationAlreadyRegistered
   DBError::IfaceImplAlreadyRegistered
   DBError::SqlError
   DBError::DatabaseNotOpen
   DBError::InvalidDatabaseConnection
   DBError::NoWritePermissions
   DBError::InvalidDatabaseFile
*/
//bool ServiceDatabase::registerService(ServiceMetaData &service)
bool ServiceDatabase::registerService(const ServiceMetaDataResults &service, const QString &securityToken)
{
    // Derive the location name with the service type prefix to be stored
    QString locationPrefix = service.location;
    int type = service.interfaces[0].d->attributes[QServiceInterfaceDescriptor::ServiceType].toInt();
    if (type == QService::InterProcess)
        locationPrefix = QLatin1String(SERVICE_IPC_PREFIX) + service.location;

#ifndef QT_SFW_SERVICEDATABASE_USE_SECURITY_TOKEN
    Q_UNUSED(securityToken);
#else
    if (securityToken.isEmpty()) {
        QString errorText("Access denied, no security token provided (for registering service: \"%1\")");
        m_lastError.setError(DBError::NoWritePermissions, errorText.arg(service.name));
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::registerService():-"
                << "Problem: Unable to register service,"
                << "reason:" << qPrintable(m_lastError.text());
#endif
        return false;
    }
#endif

    if (!checkConnection()) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::registerService():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        return false;
    }

    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    if (!beginTransaction(&query, Write)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::registerService():-"
                    << "Unable to begin transaction,"
                    << "reason:" << qPrintable(m_lastError.text());
#endif
        return false;
    }
    //See if the service's location has already been previously registered
    QString statement(QLatin1String("SELECT Name from Service WHERE Location=? COLLATE NOCASE"));
    QList<QVariant> bindValues;
    bindValues.append(locationPrefix);
    if (!executeQuery(&query, statement, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::registerService():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    if (query.next()) {
        QString alreadyRegisteredService = query.value(EBindIndex).toString();
        const QString errorText = QLatin1String("Cannot register service \"%1\". Service location \"%2\" is already "
                    "registered to service \"%3\".  \"%3\" must first be deregistered "
                    "for new registration to take place.");

        m_lastError.setError(DBError::LocationAlreadyRegistered,
                errorText.arg(service.name)
                        .arg(service.location)
                        .arg(alreadyRegisteredService));

        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::registerService():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        return false;
    }

#ifdef QT_SFW_SERVICEDATABASE_USE_SECURITY_TOKEN
    // If service(s) have already been registered with same name, they must all come from
    // same application. Fetch a service with given name and if such exists, check that its
    // security ID equals to the security ID of the current registrar.
    // One application may register multiple services with same name (different location),
    // hence the keyword DISTINCT.
    statement = "SELECT DISTINCT ServiceProperty.Value FROM Service, ServiceProperty "
                "WHERE Service.ID = ServiceProperty.ServiceID "
                "AND ServiceProperty.Key = ? "
                "AND Service.Name = ?";
    bindValues.clear();
    bindValues.append(SECURITY_TOKEN_KEY);
    bindValues.append(service.name);
    if (!executeQuery(&query, statement, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::registerService():-"
                   << qPrintable(m_lastError.text());
#endif
        return false;
    }
    QString existingSecurityToken;
    if (query.next()) {
        existingSecurityToken = query.value(EBindIndex).toString();
    }
    if (!existingSecurityToken.isEmpty() && (existingSecurityToken != securityToken)) {
        QString errorText("Access denied: \"%1\"");
        m_lastError.setError(DBError::NoWritePermissions, errorText.arg(service.name));
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::registerService():-"
                << "Problem: Unable to register service,"
                << "reason:" << qPrintable(m_lastError.text());
#endif
        return false;
    }
#endif // QT_SFW_SERVICEDATABASE_USE_SECURITY_TOKEN

    // Checks done, create new rows into tables.
    statement = QLatin1String("INSERT INTO Service(ID,Name,Location) VALUES(?,?,?)");

    qsrand(QTime::currentTime().msec());
    QString serviceID = QUuid::createUuid().toString();

    bindValues.clear();
    bindValues.append(serviceID);
    bindValues.append(service.name);
    bindValues.append(locationPrefix);

    if (!executeQuery(&query, statement, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::registerService():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    statement = QLatin1String("INSERT INTO ServiceProperty(ServiceID,Key,Value) VALUES(?,?,?)");
    bindValues.clear();
    bindValues.append(serviceID);
    bindValues.append(QLatin1String(SERVICE_DESCRIPTION_KEY));
    if (service.description.isNull())
        bindValues.append(QLatin1String("")); // This relies on !QString::isNull().
    else
        bindValues.append(service.description);

    if (!executeQuery(&query, statement, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::registerService():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

#ifdef QT_SFW_SERVICEDATABASE_GENERATE
    statement = "INSERT INTO ServiceProperty(ServiceId,Key,Value) VALUES(?,?,?)";
    bindValues.clear();
    bindValues.append(serviceID);
    bindValues.append(SERVICE_INITIALIZED_KEY);
    bindValues.append(QString("NO"));
    if (!executeQuery(&query, statement, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::registerService():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }
#endif

#ifdef QT_SFW_SERVICEDATABASE_USE_SECURITY_TOKEN
    // Insert a security token for the particular service
    statement = "INSERT INTO ServiceProperty(ServiceID,Key,Value) VALUES(?,?,?)";
    bindValues.clear();
    bindValues.append(serviceID);
    bindValues.append(SECURITY_TOKEN_KEY);
    bindValues.append(securityToken);

    if (!executeQuery(&query, statement, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::registerService():-"
                << qPrintable(m_lastError.text());
#endif
        return false;
    }
#endif // QT_SFW_SERVICEDATABASE_USE_SECURITY_TOKEN

    QList <QServiceInterfaceDescriptor> interfaces = service.interfaces;
    QString interfaceID;;
    foreach (const QServiceInterfaceDescriptor &serviceInterface, interfaces) {
        interfaceID = getInterfaceID(&query, serviceInterface);
        if (m_lastError.code() == DBError::NoError) {
            QString errorText;
            errorText = QLatin1String("Cannot register service \"%1\". \"%1\" is already registered "
                        "and implements interface \"%2\", Version \"%3.%4.\"  \"%1\" must "
                        "first be deregistered for new registration to take place.");
            m_lastError.setError(DBError::IfaceImplAlreadyRegistered,
                                errorText.arg(serviceInterface.serviceName())
                                            .arg(serviceInterface.interfaceName())
                                            .arg(serviceInterface.majorVersion())
                                            .arg(serviceInterface.minorVersion()));

            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::registerService():-"
                        << "Problem:" << qPrintable(m_lastError.text());
#endif
            return false;
        } else if (m_lastError.code() == DBError::NotFound){
            //No interface implementation already exists for the service
            //so add it
            if (!insertInterfaceData(&query, serviceInterface, serviceID)) {
                rollbackTransaction(&query);
                return false;
            } else {
                continue;
            }
        } else {
            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::registerService():-"
                        << "Unable to confirm if implementation version"
                        << (QString::number(serviceInterface.majorVersion()) + "."
                           + QString::number(serviceInterface.minorVersion())).toLatin1()
                        << "for interface" << serviceInterface.interfaceName()
                        << "is already registered for service "
                        << serviceInterface.serviceName()
                        << "\n" << m_lastError.text();
#endif
            return false;
        }
    }

    interfaces = service.latestInterfaces;
    QServiceInterfaceDescriptor defaultInterface;
    foreach (const QServiceInterfaceDescriptor &serviceInterface, interfaces) {
        defaultInterface = interfaceDefault(serviceInterface.interfaceName(), NULL, true);
        if (m_lastError.code() == DBError::NoError
                || m_lastError.code() == DBError::ExternalIfaceIDFound) {
            continue; //default already exists so don't do anything
        } else if (m_lastError.code() == DBError::NotFound) {
            //default does not already exist so create one
            interfaceID = getInterfaceID(&query, serviceInterface);
            if (m_lastError.code() != DBError::NoError) {
                rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
                qWarning() << "ServiceDatabase::registerService():-"
                           << "Unable to retrieve interfaceID for "
                              "interface" << serviceInterface.interfaceName()
                           << "\n" << m_lastError.text();
#endif
                return false;
            }

            statement = QLatin1String("INSERT INTO Defaults(InterfaceName, InterfaceID) VALUES(?,?)");
            bindValues.clear();
            bindValues.append(serviceInterface.interfaceName());
            bindValues.append(interfaceID);
            if (!executeQuery(&query, statement, bindValues)) {
                rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
                qWarning() << "ServiceDatabase::registerService():-"
                    << qPrintable(m_lastError.text());
#endif
                return false;
            }
        } else {
            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::registerService()"
                        << "Problem: Unable to confirm if interface"
                        << serviceInterface.interfaceName()
                        << "already has a default implementation";
#endif
            return false;
        }
    }

    if (!commitTransaction(&query)) {
        rollbackTransaction(&query);
        return false;
    }
    m_lastError.setError(DBError::NoError);
    return true;
}

/*
    Obtains an interface ID corresponding to a given interface \a descriptor

    May set the following error codes:
    DBError::NoError
    DBError::NotFound
    DBError::SqlError
    DBError::DatabaseNotOpen
    DBError::InvalidDatabaseConnection
*/
QString ServiceDatabase::getInterfaceID(const QServiceInterfaceDescriptor &serviceInterface) {
    QString interfaceID;
    if (!checkConnection()) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::getInterfaceID():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        return interfaceID;
    }

    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    return getInterfaceID(&query, serviceInterface);
}

/*
    This function should only ever be called on a user scope database.
    It returns a list of Interface Name and Interface ID pairs, where
    the Interface ID refers to an external interface implementation
    in the system scope database.

    May set the last error to:
    DBError::NoError
    DBError::SqlError
    DBError::DatabaseNotOpen
    DBError::InvalidDatabaseConnection

    Aside:  There is only one query which implicitly gets
    wrapped in it's own transaction.
*/
QList<QPair<QString,QString> > ServiceDatabase::externalDefaultsInfo()
{
    QList<QPair<QString,QString> > ret;
    if (!checkConnection()) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::externalDefaultsInfo():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        return ret;
    }

    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    //Prepare search query, bind criteria values and execute search
    QString selectComponent = QLatin1String("SELECT InterfaceName, InterfaceID ");
    QString fromComponent = QLatin1String("FROM Defaults ");
    QString whereComponent = QLatin1String("WHERE InterfaceID NOT IN (SELECT Interface.ID FROM Interface) ");

    //Aside: this individual query is implicitly wrapped in a transaction
    if (!executeQuery(&query, selectComponent + fromComponent + whereComponent)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::externalDefaultsInfo():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        return ret;
    }

    while (query.next()) {
        ret.append(qMakePair(query.value(EBindIndex).toString(),
                    query.value(EBindIndex1).toString()));
    }

    m_lastError.setError(DBError::NoError);
    return ret;
}

/*
    Helper function that obtains an interfaceID for a given \a descriptor.

    May set last error to one of the following error codes:
    DBError::NoError
    DBError::NotFound
    DBError::SqlError

    Aside: This function may be safely called standalone or within an explicit
    transaction.  If called standalone, it's single query is implicitly
    wrapped in it's own transaction.
*/
QString ServiceDatabase::getInterfaceID(QSqlQuery *query, const QServiceInterfaceDescriptor &serviceInterface)
{
    QString statement = QLatin1String("SELECT Interface.ID "
                        "FROM Interface, Service "
                        "WHERE Service.ID = Interface.ServiceID "
                        "AND Service.Name = ? COLLATE NOCASE "
                        "AND Interface.Name = ? COLLATE NOCASE "
                        "AND Interface.VerMaj = ? AND Interface.VerMin = ?");
    QList<QVariant> bindValues;
    bindValues.append(serviceInterface.serviceName());
    bindValues.append(serviceInterface.interfaceName());
    bindValues.append(serviceInterface.majorVersion());
    bindValues.append(serviceInterface.minorVersion());

    if (!executeQuery(query, statement, bindValues)) {
        return QString();
    }

    if (!query->next()) {
         QString errorText(QLatin1String("No Interface Descriptor found with "
                            "Service name: %1 "
                            "Interface name: %2 "
                            "Version: %3.%4"));
        m_lastError.setError(DBError::NotFound, errorText.arg(serviceInterface.serviceName())
                                                        .arg(serviceInterface.interfaceName())
                                                        .arg(serviceInterface.majorVersion())
                                                        .arg(serviceInterface.minorVersion()));
        return QString();
    }

    m_lastError.setError(DBError::NoError);
    return query->value(EBindIndex).toString();
}

/*
    Helper functions that saves \a interface related data in the Interface table
    The \a interface data is recorded as belonging to the service assocciated
    with \a serviceID.

    May set the last error to one of the following error codes:
    DBError::NoError
    DBError::SqlError

    Aside: It is already assumed that a write transaction has been started by the
    time this function is called; and this function will not rollback/commit
    the transaction.
*/
bool ServiceDatabase::insertInterfaceData(QSqlQuery *query,const QServiceInterfaceDescriptor &serviceInterface, const QString &serviceID)
{
    QString statement = QLatin1String("INSERT INTO Interface(ID, ServiceID,Name,VerMaj, VerMin) "
                        "VALUES(?,?,?,?,?)");
    QString interfaceID = QUuid::createUuid().toString();

    QList<QVariant> bindValues;
    bindValues.append(interfaceID);
    bindValues.append(serviceID);
    bindValues.append(serviceInterface.interfaceName());
    bindValues.append(serviceInterface.majorVersion());
    bindValues.append(serviceInterface.minorVersion());

    if (!executeQuery(query, statement, bindValues)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::insertInterfaceData():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    statement = QLatin1String("INSERT INTO InterfaceProperty(InterfaceID, Key, Value) VALUES(?,?,?)");
    QHash<QServiceInterfaceDescriptor::Attribute, QVariant>::const_iterator iter = serviceInterface.d->attributes.constBegin();
    bool isValidInterfaceProperty;
    QString capabilities;
    QString interfaceDescription;
    while (iter != serviceInterface.d->attributes.constEnd()) {
        isValidInterfaceProperty = true;

        bindValues.clear();
        bindValues.append(interfaceID);
        switch (iter.key()) {
            case (QServiceInterfaceDescriptor::Capabilities):
                bindValues.append(QLatin1String(INTERFACE_CAPABILITY_KEY));
                capabilities = serviceInterface.attribute(QServiceInterfaceDescriptor::Capabilities).toStringList().join(QLatin1String(","));
                if (capabilities.isNull())
                    capabilities = QLatin1String("");
                bindValues.append(capabilities);
                break;
            case(QServiceInterfaceDescriptor::Location):
                isValidInterfaceProperty = false;
                break;
            case(QServiceInterfaceDescriptor::ServiceDescription):
                isValidInterfaceProperty = false;
                break;
            case(QServiceInterfaceDescriptor::InterfaceDescription):
                bindValues.append(QLatin1String(INTERFACE_DESCRIPTION_KEY));
                interfaceDescription = serviceInterface.attribute(QServiceInterfaceDescriptor::InterfaceDescription).toString();
                if (interfaceDescription.isNull())
                    interfaceDescription = QLatin1String("");
                bindValues.append(interfaceDescription);
                break;
            default:
                isValidInterfaceProperty = false;
                break;
        }

        if (isValidInterfaceProperty) {
              if (!executeQuery(query, statement, bindValues)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
                  qWarning() << "ServiceDatabase::insertInterfaceData():-"
                                << qPrintable(m_lastError.text());
#endif
                  return false;
              }
        }
        ++iter;
    }

    // add custom attributes
    QHash<QString, QString>::const_iterator customIter = serviceInterface.d->customAttributes.constBegin();
    while (customIter!=serviceInterface.d->customAttributes.constEnd()) {
        bindValues.clear();
        bindValues.append(interfaceID);
        // to avoid key clashes use separate c_ namespace ->is this sufficient?
        bindValues.append(QVariant(QStringLiteral("c_") + customIter.key()));
        bindValues.append(customIter.value());
        if (!executeQuery(query, statement, bindValues)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::insertInterfaceData(customProps):-"
                            << qPrintable(m_lastError.text());
#endif
            return false;
        }
        ++customIter;
    }
    m_lastError.setError(DBError::NoError);

    return true;
}

/*
    Helper function that executes the sql query specified in \a statement.
    It is assumed that the \a statement uses positional placeholders and
    corresponding parameters are placed in the list of \a bindValues.

    Aside: This function may be safely called standalone or within an explicit
    transaction.  If called standalone, it's single query is implicitly
    wrapped in it's own transaction.

    May set the last error to one of the following error codes:
    DBError::NoError
    DBError::SqlError
    DBError::NoWritePermissions
    DBError::InvalidDatabaseFile
*/
bool ServiceDatabase::executeQuery(QSqlQuery *query, const QString &statement, const QList<QVariant> &bindValues)
{
    Q_ASSERT(query != NULL);

    bool success = false;
    enum {Prepare =0 , Execute=1};
    for (int stage=Prepare; stage <= Execute; ++stage) {
        if ( stage == Prepare)
            success = query->prepare(statement);
        else // stage == Execute
            success = query->exec();

        if (!success) {
            QString errorText;
            errorText = QLatin1String("Problem: Could not %1 statement: %2"
                "Reason: %3"
                "Parameters: %4\n");
            QString parameters;
            if (bindValues.count() > 0) {
                for (int i = 0; i < bindValues.count(); ++i) {
                    parameters.append(QStringLiteral("\n\t[") + QString::number(i) + QStringLiteral("]: ") + bindValues.at(i).toString());
                }
            } else {
                parameters = QLatin1String("None");
            }

            DBError::ErrorCode errorType;
            int result = query->lastError().number();
            if (result == 26 || result == 11) {//SQLILTE_NOTADB || SQLITE_CORRUPT
                qWarning() << "Service Framework:- Database file is corrupt or invalid:" << databasePath();
                errorType = DBError::InvalidDatabaseFile;
            }
            else if ( result == 8) //SQLITE_READONLY
                errorType = DBError::NoWritePermissions;
            else
                errorType = DBError::SqlError;

            m_lastError.setError(errorType,
                    errorText
                    .arg(stage == Prepare ?QLatin1String("prepare"):QLatin1String("execute"))
                    .arg(statement)
                    .arg(query->lastError().text())
                    .arg(parameters));

            query->finish();
            query->clear();
            return false;
        }

        if (stage == Prepare) {
            foreach (const QVariant &bindValue, bindValues)
            query->addBindValue(bindValue);
        }
    }

    m_lastError.setError(DBError::NoError);
    return true;
}

/*
   Obtains a list of QServiceInterfaceDescriptors that match the constraints supplied
   by \a filter.

   May set last error to one of the following error codes:
   DBError::NoError
   DBError::SqlError
   DBError::DatabaseNotOpen
   DBError::InvalidDatabaseConnection
   DBError::NoWritePermissions
   DBError::InvalidDatabaseFile
*/
QList<QServiceInterfaceDescriptor> ServiceDatabase::getInterfaces(const QServiceFilter &filter)
{
    QList<QServiceInterfaceDescriptor> interfaces;
    if (!checkConnection()) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::getInterfaces():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        return interfaces;
    }

    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    //multiple read queries are performed so wrap them
    //in a read only transaction
    if (!beginTransaction(&query, Read)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::getInterfaces():-"
                    << "Unable to begin transaction. "
                    << "Reason:" << qPrintable(m_lastError.text());
#endif
        return interfaces;
    }

    //Prepare search query, bind criteria values
    QString selectComponent = QLatin1String("SELECT Interface.Name, "
                                "Service.Name, Interface.VerMaj, "
                                "Interface.VerMin, "
                                "Service.Location, "
                                "Service.ID, "
                                "Interface.ID ");
    QString fromComponent = QLatin1String("FROM Interface, Service ");
    QString whereComponent = QLatin1String("WHERE Service.ID = Interface.ServiceID ");
    QList<QVariant> bindValues;

    if (filter.serviceName().isEmpty() && filter.interfaceName().isEmpty()) {
        //do nothing, (don't add any extra constraints to the query
    } else {

        if (!filter.serviceName().isEmpty()) {
            whereComponent.append(QLatin1String("AND Service.Name = ?")).append(QLatin1String(" COLLATE NOCASE "));
            bindValues.append(filter.serviceName());
        }
        if (!filter.interfaceName().isEmpty()) {
            whereComponent.append(QLatin1String("AND Interface.Name = ?")).append(QLatin1String(" COLLATE NOCASE "));
            bindValues.append(filter.interfaceName());
            if (filter.majorVersion() >=0 && filter.minorVersion() >=0) {
                if (filter.versionMatchRule() == QServiceFilter::ExactVersionMatch) {
                    whereComponent.append(QLatin1String("AND Interface.VerMaj = ?")).append(QLatin1String(" AND Interface.VerMin = ? "));
                    bindValues.append(QString::number(filter.majorVersion()));
                    bindValues.append(QString::number(filter.minorVersion()));
                }
                else if (filter.versionMatchRule() == QServiceFilter::MinimumVersionMatch) {
                    whereComponent.append(QLatin1String("AND ((Interface.VerMaj > ?"))
                        .append(QLatin1String(") OR Interface.VerMaj = ?")).append(QLatin1String(" AND Interface.VerMin >= ?")).append(QLatin1String(") "));
                    bindValues.append(QString::number(filter.majorVersion()));
                    bindValues.append(QString::number(filter.majorVersion()));
                    bindValues.append(QString::number(filter.minorVersion()));
                }
            }
        }
    }

    if (!executeQuery(&query, selectComponent + fromComponent + whereComponent, bindValues)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::getInterfaces():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        rollbackTransaction(&query);
        return interfaces;
    }

    QServiceInterfaceDescriptor serviceInterface;
    serviceInterface.d = new QServiceInterfaceDescriptorPrivate;
    QStringList capabilities;
    QString serviceID;
    QString interfaceID;
    const QSet<QString> filterCaps = filter.capabilities().toSet();
    QSet<QString> difference;

    while (query.next()){
        difference.clear();
        serviceInterface.d->customAttributes.clear();
        serviceInterface.d->attributes.clear();
        serviceInterface.d->interfaceName = query.value(EBindIndex).toString();
        serviceInterface.d->serviceName = query.value(EBindIndex1).toString();
        serviceInterface.d->major = query.value(EBindIndex2).toInt();
        serviceInterface.d->minor = query.value(EBindIndex3).toInt();

        QString location = query.value(EBindIndex4).toString();
        if (location.startsWith(QLatin1String(SERVICE_IPC_PREFIX))) {
            serviceInterface.d->attributes[QServiceInterfaceDescriptor::ServiceType] = QService::InterProcess;
            serviceInterface.d->attributes[QServiceInterfaceDescriptor::Location]
                = location.remove(0,QString(QLatin1String(SERVICE_IPC_PREFIX)).size());
        } else {
            serviceInterface.d->attributes[QServiceInterfaceDescriptor::ServiceType] = QService::Plugin;
            serviceInterface.d->attributes[QServiceInterfaceDescriptor::Location] = location;
        }

        serviceID = query.value(EBindIndex5).toString();
        if (!populateServiceProperties(&serviceInterface, serviceID)) {
            //populateServiceProperties should already give a warning message
            //and set the last error
            interfaces.clear();
            rollbackTransaction(&query);
            return interfaces;
        }

        interfaceID = query.value(EBindIndex6).toString();
        if (!populateInterfaceProperties(&serviceInterface, interfaceID)) {
            //populateInterfaceProperties should already give a warning message
            //and set the last error
            interfaces.clear();
            rollbackTransaction(&query);
            return interfaces;
        }

        const QSet<QString> ifaceCaps = serviceInterface.d->attributes.value(QServiceInterfaceDescriptor::Capabilities).toStringList().toSet();
        difference = ((filter.capabilityMatchRule() == QServiceFilter::MatchMinimum) ? (filterCaps-ifaceCaps) : (ifaceCaps-filterCaps));
        if (!difference.isEmpty())
            continue;

        //only return those interfaces that comply with set custom filters
        if (filter.customAttributes().size() > 0) {
            QSet<QString> keyDiff = filter.customAttributes().toSet();
            keyDiff.subtract(serviceInterface.d->customAttributes.uniqueKeys().toSet());
            if (keyDiff.isEmpty()) { //target descriptor has same custom keys as filter
                bool isMatch = true;
                const QStringList keys = filter.customAttributes();
                for (int i = 0; i<keys.count(); i++) {
                    if (serviceInterface.d->customAttributes.value(keys[i]) !=
                            filter.customAttribute(keys[i])) {
                        isMatch = false;
                        break;
                    }
                }
                if (isMatch)
                    interfaces.append(serviceInterface);
            }
        } else { //no custom keys -> SQL statement ensures proper selection already
            interfaces.append(serviceInterface);
        }
    }

    rollbackTransaction(&query);//read-only operation so just rollback
    m_lastError.setError(DBError::NoError);
    return interfaces;
}

/*
   Obtains a QServiceInterfaceDescriptor that
   corresponds to a given \a interfaceID

   May set last error to one of the following error codes:
   DBError::NoError
   DBError::NotFound
   DBError::SqlError
   DBError::DatabaseNotOpen
   DBError::InvalidDatabaseConnection
   DBError::NoWritePermissions
   DBError::InvalidDatabaseFile
*/
QServiceInterfaceDescriptor ServiceDatabase::getInterface(const QString &interfaceID)
{
    QServiceInterfaceDescriptor serviceInterface;
    if (!checkConnection()) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::getInterface():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        return serviceInterface;
    }

    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    if (!beginTransaction(&query, Read)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::getInterface():-"
                    << "Unable to begin transaction. "
                    << "Reason:" << qPrintable(m_lastError.text());
#endif
        return serviceInterface;
    }

    QString selectComponent = QLatin1String("SELECT Interface.Name, "
                                "Service.Name, Interface.VerMaj, "
                                "Interface.VerMin, "
                                "Service.Location, "
                                "Service.ID ");
    QString fromComponent = QLatin1String("FROM Interface, Service ");
    QString whereComponent = QLatin1String("WHERE Service.ID = Interface.ServiceID "
                                    "AND Interface.ID = ? ");
    QList<QVariant> bindValues;
    bindValues.append(interfaceID);

    if (!executeQuery(&query, selectComponent + fromComponent + whereComponent, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::getInterfaces():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        return serviceInterface;
    }

    if (!query.next()) {
        rollbackTransaction(&query);
        QString errorText(QLatin1String("Interface implementation not found for Interface ID: %1"));
        m_lastError.setError(DBError::NotFound, errorText.arg(interfaceID));
        return serviceInterface;
    }

    serviceInterface.d = new QServiceInterfaceDescriptorPrivate;
    serviceInterface.d->interfaceName =query.value(EBindIndex).toString();
    serviceInterface.d->serviceName = query.value(EBindIndex1).toString();
    serviceInterface.d->major = query.value(EBindIndex2).toInt();
    serviceInterface.d->minor = query.value(EBindIndex3).toInt();

    QString location = query.value(EBindIndex4).toString();
    if (location.startsWith(QLatin1String(SERVICE_IPC_PREFIX))) {
        serviceInterface.d->attributes[QServiceInterfaceDescriptor::ServiceType] = QService::InterProcess;
        serviceInterface.d->attributes[QServiceInterfaceDescriptor::Location]
            = location.remove(0,QString(QLatin1String(SERVICE_IPC_PREFIX)).size());
    } else {
        serviceInterface.d->attributes[QServiceInterfaceDescriptor::ServiceType] = QService::Plugin;
        serviceInterface.d->attributes[QServiceInterfaceDescriptor::Location] = location;
    }

    QString serviceID = query.value(EBindIndex5).toString();
    if (!populateServiceProperties(&serviceInterface, serviceID)) {
        //populateServiceProperties should already give a warning message
        //and set the last error
        rollbackTransaction(&query);
        return QServiceInterfaceDescriptor();
    }

    if (!populateInterfaceProperties(&serviceInterface, interfaceID)) {
        //populateInterfaceProperties should already give a warning message
        //and set the last error
        rollbackTransaction(&query);
        return QServiceInterfaceDescriptor();
    }

    rollbackTransaction(&query);//read only operation so just rollback
    m_lastError.setError(DBError::NoError);
    return serviceInterface;
}

/*
    Obtains a list of services names.  If \a interfaceName is empty,
    then all service names are returned.  If \a interfaceName specifies
    an interface then the names of all services implementing that interface
    are returned

    May set last error to one of the following error codes:
    DBError::NoError
    DBError::SqlError
    DBError::DatabaseNotOpen
    DBError::InvalidDatabaseConnection
    DBError::NoWritePermissions
    DBError::InvalidDatabaseFile

    Aside:  There is only one query which implicitly gets
    wrapped in it's own transaction.
*/
QStringList ServiceDatabase::getServiceNames(const QString &interfaceName)
{
    QStringList services;
    if (!checkConnection()) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::getServiceNames():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        return services;
    }
    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);
    QString selectComponent(QLatin1String("SELECT DISTINCT Service.Name COLLATE NOCASE "));
    QString fromComponent;
    QString whereComponent;
    QList<QVariant> bindValues;
    if (interfaceName.isEmpty()) {
        fromComponent = QLatin1String("FROM Service ");
    } else {
        fromComponent = QLatin1String("FROM Interface,Service ");
        whereComponent = QLatin1String("WHERE Service.ID = Interface.ServiceID AND Interface.Name = ? COLLATE NOCASE ");
        bindValues.append(interfaceName);
    }

    if (!executeQuery(&query, selectComponent + fromComponent + whereComponent, bindValues)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::getServiceNames():-"
                    << qPrintable(m_lastError.text());
#endif
        return services;
    }

    while ( query.next()) {
        services.append(query.value(EBindIndex).toString());
    }
    query.finish();
    query.clear();
    m_lastError.setError(DBError::NoError);
    return services;
}

/*
    Returns a descriptor for the default interface implementation of
    \a interfaceName.

    For user scope databases only, \a defaultInterfaceID is set if the default
    in the user scope database refers to a interface implementation in the
    system scope database.  In this case the descriptor will be invalid and
    the \a defaultInterfaceID must be used to query the system scope database,
    The last error set to DBError::ExternalIfaceIDFound

    If this function is called within a transaction, \a inTransaction
    must be set to true.  If \a inTransaction is false, this fuction
    will begin and end its own transaction.

    The last error may be set to one of the following error codes:
    DBError::NoError
    DBError::ExternalIfaceIDFound
    DBError::SqlError
    DBError::DatabaseNotOpen
    DBError::InvalidDatabaseConnection
    DBError::NoWritePermissions
    DBError::InvalidDatabaseFile
*/
QServiceInterfaceDescriptor ServiceDatabase::interfaceDefault(const QString &interfaceName, QString *defaultInterfaceID,
                                                                    bool inTransaction)
{
    QServiceInterfaceDescriptor serviceInterface;
    if (!checkConnection())
    {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::interfaceDefault():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        return serviceInterface;
    }

    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    if (!inTransaction && !beginTransaction(&query, Read)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::interfaceDefault(QString, QString):-"
                    << "Unable to begin transaction. "
                    << "Reason:" << qPrintable(m_lastError.text());
#endif
        return serviceInterface;
    }

    QString statement(QLatin1String("SELECT InterfaceID FROM Defaults WHERE InterfaceName = ? COLLATE NOCASE"));
    QList<QVariant> bindValues;
    bindValues.append(interfaceName);
    if (!executeQuery(&query, statement, bindValues)) {
        if (!inTransaction)
            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::interfaceDefault():-"
                    << qPrintable(m_lastError.text());
#endif
        return serviceInterface;
    }

    QString interfaceID;
    if (!query.next())
    {
        if (!inTransaction)
            rollbackTransaction(&query);
        QString errorText(QLatin1String("No default service found for interface: \"%1\""));
        m_lastError.setError(DBError::NotFound, errorText.arg(interfaceName));
        return serviceInterface;
    }
    else
        interfaceID = query.value(EBindIndex).toString();
    Q_ASSERT(!interfaceID.isEmpty());

    statement = QLatin1String("SELECT Interface.Name, "
                        "Service.Name, Interface.VerMaj, "
                        "Interface.VerMin, "
                        "Service.Location, "
                        "Service.ID "
                    "FROM Service, Interface "
                    "WHERE Service.ID = Interface.ServiceID AND Interface.ID = ?");
    bindValues.clear();
    bindValues.append(interfaceID);
    if (!executeQuery(&query, statement, bindValues))
    {
        if (!inTransaction)
            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::interfaceDefault():-"
                    << qPrintable(m_lastError.text());
#endif
        return serviceInterface;
    }

    if (!query.next()) {
        if (!inTransaction)
            rollbackTransaction(&query);
        if (defaultInterfaceID != NULL )
            *defaultInterfaceID = interfaceID;
        m_lastError.setError(DBError::ExternalIfaceIDFound);
        return serviceInterface;
    }

    serviceInterface.d = new QServiceInterfaceDescriptorPrivate;
    serviceInterface.d->interfaceName =query.value(EBindIndex).toString();
    serviceInterface.d->serviceName = query.value(EBindIndex1).toString();
    serviceInterface.d->major = query.value(EBindIndex2).toInt();
    serviceInterface.d->minor = query.value(EBindIndex3).toInt();

    QString location = query.value(EBindIndex4).toString();
    if (location.startsWith(QLatin1String(SERVICE_IPC_PREFIX))) {
        serviceInterface.d->attributes[QServiceInterfaceDescriptor::ServiceType] = QService::InterProcess;
        serviceInterface.d->attributes[QServiceInterfaceDescriptor::Location]
            = location.remove(0,QString(QLatin1String(SERVICE_IPC_PREFIX)).size());
    } else {
        serviceInterface.d->attributes[QServiceInterfaceDescriptor::ServiceType] = QService::Plugin;
        serviceInterface.d->attributes[QServiceInterfaceDescriptor::Location] = location;
    }

    QString serviceID = query.value(EBindIndex5).toString();
    if (!populateServiceProperties(&serviceInterface, serviceID)) {
        //populateServiceProperties should already give a warning
        //and set the last error
        if (!inTransaction)
            rollbackTransaction(&query);
        return QServiceInterfaceDescriptor();
    }

    if (!populateInterfaceProperties(&serviceInterface, interfaceID)) {
        //populateInterfaceProperties should already give a warning
        //and set the last error
        if (!inTransaction)
            rollbackTransaction(&query);
        return QServiceInterfaceDescriptor();
    }

    if (!inTransaction)
        rollbackTransaction(&query); //Read only operation so just rollback
    m_lastError.setError(DBError::NoError);
    return serviceInterface;
}

/*
   Sets a particular service's \a interface implementation as a the default
   implementation to look up when using the interface's name in
   interfaceDefault().

   For a user scope database an \a externalInterfaceID can be provided
   so that the Defaults table will contain a "link" to an interface
   implmentation provided in the system scope database.

   May set the last error to one of the following error codes:
   DBError::NoError
   DBerror::NotFound
   DBError::SqlError
   DBError::DatabaseNotOpen
   DBError::InvalidDatabaseConnection
   DBError::NoWritePermissions
   DBError::InvalidDatabaseFile
*/
bool ServiceDatabase::setInterfaceDefault(const QServiceInterfaceDescriptor &serviceInterface, const QString &externalInterfaceID)
{
    if (!checkConnection()) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::setInterfaceDefault(QServiceInterfaceDescriptor):-"
            << "Problem:" << qPrintable(m_lastError.text());
#endif
        return false;
    }

    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    //Begin Transaction
    if (!beginTransaction(&query, Write)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::setInterfaceDefault(QServiceInterfaceDescriptor):-"
            << "Problem: Unable to begin transaction."
            << "Reason:" << qPrintable(m_lastError.text());
#endif
        return false;
    }

    QString statement;
    QList<QVariant> bindValues;
    QString interfaceID = externalInterfaceID;
    if (interfaceID.isEmpty()) {
        statement = QLatin1String("SELECT Interface.ID from Interface, Service "
                "WHERE Service.ID = Interface.ServiceID "
                "AND Service.Name = ? COLLATE NOCASE "
                "AND Interface.Name = ? COLLATE NOCASE "
                "AND Interface.VerMaj = ? "
                "AND Interface.VerMin = ? ");
        bindValues.append(serviceInterface.serviceName());
        bindValues.append(serviceInterface.interfaceName());
        bindValues.append(serviceInterface.majorVersion());
        bindValues.append(serviceInterface.minorVersion());

        if (!executeQuery(&query, statement, bindValues)) {
            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::setInterfaceDefault(QServiceInterfaceDescriptor):-"
                << qPrintable(m_lastError.text());
#endif
            return false;
        }

        if (!query.next()) {
            QString errorText;
            errorText = QLatin1String("No implementation for interface: %1, Version: %2.%3 found "
                "for service: %4");
            m_lastError.setNotFoundError(errorText.arg(serviceInterface.interfaceName())
                    .arg(serviceInterface.majorVersion())
                    .arg(serviceInterface.minorVersion())
                    .arg(serviceInterface.serviceName()));

            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatbase::setInterfaceDefault(QServiceInterfaceDescriptor):-"
                << "Problem: Unable to set default service. "
                << "Reason:" << qPrintable(m_lastError.text());
#endif
            return false;
        }

        interfaceID = query.value(EBindIndex).toString();
        Q_ASSERT(!interfaceID.isEmpty());
    }

    statement = QLatin1String("SELECT InterfaceName FROM Defaults WHERE InterfaceName = ? COLLATE NOCASE");
    bindValues.clear();
    bindValues.append(serviceInterface.interfaceName());
    if (!executeQuery(&query, statement, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::setInterfaceDefault(QServiceInterfaceDescriptor):-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    if (query.next()) {
        statement = QLatin1String("UPDATE Defaults "
            "SET InterfaceID = ? "
            "WHERE InterfaceName = ? COLLATE NOCASE");
        bindValues.clear();
        bindValues.append(interfaceID);
        bindValues.append(serviceInterface.interfaceName());

        if (!executeQuery(&query, statement, bindValues)) {
            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::setInterfaceDefault(QServiceInterfaceDescriptor):-"
                << qPrintable(m_lastError.text());
#endif
            return false;
        }
    } else {
        statement = QLatin1String("INSERT INTO Defaults(InterfaceName,InterfaceID) VALUES(?,?)");
        bindValues.clear();
        bindValues.append(serviceInterface.interfaceName());
        bindValues.append(interfaceID);

        if (!executeQuery(&query, statement, bindValues)) {
            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::setInterfaceDefault(QServiceInterfaceDescriptor):-"
                << qPrintable(m_lastError.text());
#endif
            return false;
        }
    }

    //End Transaction
    if (!commitTransaction(&query)) {
        rollbackTransaction(&query);
        return false;
    }
    m_lastError.setError(DBError::NoError);
    return true;
}

/*
   Removes the service with name \a serviceName.
   If the service provides a default interface implementation, then
   another service implementing the highest interface implementation
   version becomes the new default(if any).  If more than one service
   provides same the highest version number, an arbitrary choice is made
   between them.

   May set the last error to the folowing error codes:
   DBError::NoError
   DBError::NotFound
   DBError::SqlError
   DBError::DatabaseNotOpen
   DBError::InvalidDatabaseConnection
   DBError::NoWritePermissions
   DBError::InvalidDatabaseFile
*/
bool ServiceDatabase::unregisterService(const QString &serviceName, const QString &securityToken)
{
#ifndef QT_SFW_SERVICEDATABASE_USE_SECURITY_TOKEN
    Q_UNUSED(securityToken);
#else
    if (securityToken.isEmpty()) {
        QString errorText(QLatin1String("Access denied, no security token provided (for unregistering service: \"%1\")"));
        m_lastError.setError(DBError::NoWritePermissions, errorText.arg(serviceName));
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::unregisterService():-"
                << "Problem: Unable to unregister service. "
                << "Reason:" << qPrintable(m_lastError.text());
#endif
        return false;
    }
#endif


    if (!checkConnection()) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::unregisterService():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        return false;
    }

    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    if (!beginTransaction(&query, Write)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::unregisterService():-"
                    << "Problem: Unable to begin transaction. "
                    << "Reason:" << qPrintable(m_lastError.text());
#endif
        return false;
    }

    QString statement(QLatin1String("SELECT Service.ID from Service WHERE Service.Name = ? COLLATE NOCASE"));
    QList<QVariant> bindValues;
    bindValues.append(serviceName);
    if (!executeQuery(&query, statement, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::unregisterService():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    QStringList serviceIDs;
    while (query.next()) {
        serviceIDs << query.value(EBindIndex).toString();
    }

#ifdef QT_SFW_SERVICEDATABASE_USE_SECURITY_TOKEN
    // Only the application that registered the service is allowed to unregister that
    // service. Fetch a security ID of a service (with given name) and verify that it matches
    // with current apps security id. Only one application is allowed to register services with
    // same name, hence a distinct (just any of the) security token will do because they are identical.
    if (!serviceIDs.isEmpty()) {
        statement = QLatin1String("SELECT DISTINCT Value FROM ServiceProperty WHERE ServiceID = ? AND Key = ?");
        bindValues.clear();
        bindValues.append(serviceIDs.first());
        bindValues.append(SECURITY_TOKEN_KEY);

        if (!executeQuery(&query, statement, bindValues)) {
            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::unregisterService():-"
                    << qPrintable(m_lastError.text());
#endif
            return false;
        }
        QString existingSecurityToken;
        if (query.next()) {
            existingSecurityToken = query.value(EBindIndex).toString();
        }
        if (existingSecurityToken != securityToken) {
            QString errorText(QLatin1String("Access denied: \"%1\""));
            m_lastError.setError(DBError::NoWritePermissions, errorText.arg(serviceName));
            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::unregisterService():-"
                    << "Problem: Unable to unregister service"
                    << "Reason:" << qPrintable(m_lastError.text());
#endif
            return false;
        }
    }
#endif // QT_SFW_SERVICEDATABASE_USE_SECURITY_TOKEN

    statement = QLatin1String("SELECT Interface.ID from Interface, Service "
                "WHERE Interface.ServiceID = Service.ID "
                    "AND Service.Name =? COLLATE NOCASE");
    bindValues.clear();
    bindValues.append(serviceName);
    if (!executeQuery(&query, statement, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::unregisterService():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    QStringList interfaceIDs;
    while (query.next()) {
        interfaceIDs << query.value(EBindIndex).toString();
    }

    if (serviceIDs.count() == 0) {
        QString errorText(QLatin1String("Service not found: \"%1\""));
        m_lastError.setError(DBError::NotFound, errorText.arg(serviceName));
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::unregisterService():-"
                    << "Problem: Unable to unregister service"
                    << "Reason:" << qPrintable(m_lastError.text());
#endif
        return false;
    }

    statement = QLatin1String("SELECT Defaults.InterfaceName "
                "FROM Defaults, Interface, Service "
                "WHERE Defaults.InterfaceID = Interface.ID "
                    "AND Interface.ServiceID = Service.ID "
                    "AND Service.Name = ? COLLATE NOCASE");
    bindValues.clear();
    bindValues.append(serviceName);
    if (!executeQuery(&query, statement, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase:unregisterService():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    QStringList serviceDefaultInterfaces;
    while (query.next()) {
        serviceDefaultInterfaces << query.value(EBindIndex).toString();
    }


    statement = QLatin1String("DELETE FROM Service WHERE Service.Name = ? COLLATE NOCASE");
    bindValues.clear();
    bindValues.append(serviceName);
    if (!executeQuery(&query, statement, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::unregisterService():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    statement = QLatin1String("DELETE FROM Interface WHERE Interface.ServiceID = ?");
    foreach (const QString &serviceID, serviceIDs) {
        bindValues.clear();
        bindValues.append(serviceID);
        if (!executeQuery(&query, statement, bindValues)) {
            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::unregisterService():-"
                        << qPrintable(m_lastError.text());
#endif
            return false;
        }
    }

    statement = QLatin1String("DELETE FROM ServiceProperty WHERE ServiceID = ?");

    foreach (const QString &serviceID, serviceIDs) {
        bindValues.clear();
        bindValues.append(serviceID);
        if (!executeQuery(&query, statement, bindValues)) {
            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::unregisterService():-"
                        << qPrintable(m_lastError.text());
#endif
            return false;
        }
    }

    statement = QLatin1String("DELETE FROM InterfaceProperty WHERE InterfaceID = ?");
    foreach (const QString &interfaceID,  interfaceIDs) {
        bindValues.clear();
        bindValues.append(interfaceID);
        if (!executeQuery(&query, statement, bindValues)) {
            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::unregisterService():-"
                        << qPrintable(m_lastError.text());
#endif
            return false;
        }
    }

    foreach (const QString &interfaceName, serviceDefaultInterfaces) {
        statement = QLatin1String("SELECT ID FROM Interface WHERE Interface.Name = ? COLLATE NOCASE "
                    "ORDER BY Interface.VerMaj DESC, Interface.VerMin DESC");
        bindValues.clear();
        bindValues.append(interfaceName);
        if (!executeQuery(&query, statement, bindValues)) {
            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::unregisterService():-"
                        << qPrintable(m_lastError.text());
#endif
            return false;
        }

        if (query.next()) {
            QString newDefaultID = query.value(EBindIndex).toString();
            statement = QLatin1String("UPDATE Defaults SET InterfaceID = ? WHERE InterfaceName = ? COLLATE NOCASE ");
            bindValues.clear();
            bindValues.append(newDefaultID);
            bindValues.append(interfaceName);
            if (!executeQuery(&query, statement, bindValues)) {
                rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
                qWarning() << "ServiceDatabase::unregisterService():-"
                            << qPrintable(m_lastError.text());
#endif
                return false;
            }
        } else {
            statement = QLatin1String("DELETE FROM Defaults WHERE InterfaceName = ? COLLATE NOCASE ");
            bindValues.clear();
            bindValues.append(interfaceName);
            if (!executeQuery(&query, statement, bindValues)) {
                rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
                qWarning() << "ServiceDatabase::unregisterService():-"
                            << qPrintable(m_lastError.text());
#endif
                return false;
            }
        }
    }

    //databaseCommit
    if (!commitTransaction(&query)) {
        rollbackTransaction(&query);
        return false;
    }
    m_lastError.setError(DBError::NoError);
    return true;
}

/*
    Registers the service initialization into the database.
*/
bool ServiceDatabase::serviceInitialized(const QString &serviceName, const QString &securityToken)
{
#ifndef QT_SFW_SERVICEDATABASE_USE_SECURITY_TOKEN
    Q_UNUSED(securityToken);
#endif

    if (!checkConnection()) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::unregisterService():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        return false;
    }

    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    if (!beginTransaction(&query, Write)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::serviceInitialized():-"
                    << "Problem: Unable to begin transaction"
                    << "\nReason:" << qPrintable(m_lastError.text());
#endif
        return false;
    }

    QString statement(QLatin1String("SELECT Service.ID from Service WHERE Service.Name = ? COLLATE NOCASE"));
    QList<QVariant> bindValues;
    bindValues.append(serviceName);
    if (!executeQuery(&query, statement, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::serviceInitialized():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    QStringList serviceIDs;
    while (query.next()) {
        serviceIDs << query.value(EBindIndex).toString();
    }


#ifdef QT_SFW_SERVICEDATABASE_USE_SECURITY_TOKEN
    statement = QLatin1String("SELECT Value FROM ServiceProperty WHERE ServiceID = ? AND Key = ?");
    bindValues.clear();
    bindValues.append(serviceName);
    bindValues.append(SECURITY_TOKEN_KEY);
    if (!executeQuery(&query, statement, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::unregisterService():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    QStringList securityTokens;
    while (query.next()) {
        securityTokens << query.value(EBindIndex).toString();
    }

    if (!securityTokens.isEmpty() && (securityTokens.first() != securityToken)) {
        QString errorText(QLatin1String("Access denied: \"%1\""));
             m_lastError.setError(DBError::NoWritePermissions, errorText.arg(serviceName));
             rollbackTransaction(&query);
     #ifdef QT_SFW_SERVICEDATABASE_DEBUG
             qWarning() << "ServiceDatabase::serviceInitialized():-"
                         << "Problem: Unable to update service initialization"
                         << "\nReason:" << qPrintable(m_lastError.text());
     #endif
    }
#endif

    statement = QLatin1String("DELETE FROM ServiceProperty WHERE ServiceID = ? AND Key = ?");
    foreach (const QString &serviceID, serviceIDs) {
        bindValues.clear();
        bindValues.append(serviceID);
        bindValues.append(QLatin1String(SERVICE_INITIALIZED_KEY));
        if (!executeQuery(&query, statement, bindValues)) {
            rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::serviceInitialized():-"
                        << qPrintable(m_lastError.text());
#endif
            return false;
        }
    }

    //databaseCommit
    if (!commitTransaction(&query)) {
        rollbackTransaction(&query);
        return false;
    }
    m_lastError.setError(DBError::NoError);
    return true;
}

/*
    Closes the database

    May set the following error codes:
    DBError::NoError
    DBError::InvalidDatabaseConnection
*/
bool ServiceDatabase::close()
{
    if (m_isDatabaseOpen) {
        QSqlDatabase database = QSqlDatabase::database(m_connectionName, false);
        if (database.isValid()) {
            if (database.isOpen()) {
                database.close();
                m_isDatabaseOpen = false;
                return true;
            }
        } else {
            m_lastError.setError(DBError::InvalidDatabaseConnection);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::close():-"
                        << "Problem: " << qPrintable(m_lastError.text());
#endif
            return false;
        }
    }
    m_lastError.setError(DBError::NoError);
    return true;
}

/*
    Sets the path of the service database to \a databasePath
*/
void ServiceDatabase::setDatabasePath(const QString &databasePath)
{
    m_databasePath = QDir::toNativeSeparators(databasePath);
}

/*
    Returns the path of the service database
*/
QString ServiceDatabase::databasePath() const
{
    QString path;
    if (m_databasePath.isEmpty()) {
        QSettings settings(QSettings::SystemScope, QLatin1String("Nokia"), QLatin1String("Services"));
        path = settings.value(QLatin1String("ServicesDB/Path")).toString();
        if (path.isEmpty()) {
            path = QDir::currentPath();
            if (path.lastIndexOf(QLatin1String(RESOLVERDATABASE_PATH_SEPARATOR)) != path.length() -1) {
                path.append(QLatin1String(RESOLVERDATABASE_PATH_SEPARATOR));
            }
            path.append(QLatin1String(RESOLVERDATABASE));
        }
        path = QDir::toNativeSeparators(path);
    } else {
        path = m_databasePath;
    }

    return path;
}

/*
    Helper method that creates the database tables: Service, Interface,
    Defaults, ServiceProperty and InterfaceProperty

    May set the last error to one of the following error codes:
    DBError::NoError
    DBError::SqlError
    DBError::NoWritePermissions
    DBError::InvalidDatabaseFile
*/
bool ServiceDatabase::createTables()
{
    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    //Begin Transaction
    if (!beginTransaction(&query, Write)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::createTables():-"
                    << "Unable to begin transaction. "
                    << "Reason:" << qPrintable(m_lastError.text());
#endif
        return false;
    }

    QString statement(QLatin1String("CREATE TABLE Service("
                        "ID TEXT NOT NULL PRIMARY KEY UNIQUE,"
                        "Name TEXT NOT NULL, "
                        "Location TEXT NOT NULL)"));
    if (!executeQuery(&query, statement)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::createTables():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    statement = QLatin1String("CREATE TABLE Interface("
                "ID TEXT NOT NULL PRIMARY KEY UNIQUE,"
                "ServiceID TEXT NOT NULL, "
                "Name TEXT NOT NULL, "
                "VerMaj INTEGER NOT NULL, "
                "VerMin INTEGER NOT NULL)");
    if (!executeQuery(&query, statement)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::createTables():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    statement = QLatin1String("CREATE TABLE Defaults("
                "InterfaceName TEXT PRIMARY KEY UNIQUE NOT NULL,"
                "InterfaceID TEXT NOT NULL)");
    if (!executeQuery(&query, statement)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::createTables():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    statement = QLatin1String("CREATE TABLE ServiceProperty("
                "ServiceID TEXT NOT NULL,"
                "Key TEXT NOT NULL,"
                "Value TEXT NOT NULL)");
    if (!executeQuery(&query, statement)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::createTables():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    statement = QLatin1String("CREATE TABLE InterfaceProperty("
                "InterfaceID TEXT NOT NULL,"
                "Key TEXT NOT NULL,"
                "Value TEXT NOT NULL)");

    if (!executeQuery(&query, statement)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::createTables():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    if (!commitTransaction(&query)) {
        rollbackTransaction(&query);
        return false;
    }
    m_lastError.setError(DBError::NoError);
    return true;
}

/*!
    Helper method that checks if the all expected tables exist in the database
    Returns true if they all exist and false if any of them don't
*/
bool ServiceDatabase::checkTables()
{
    bool bTables(false);
    QStringList tables = QSqlDatabase::database(m_connectionName).tables();
    if (tables.contains(QLatin1String(SERVICE_TABLE))
        && tables.contains(QLatin1String(INTERFACE_TABLE))
        && tables.contains(QLatin1String(DEFAULTS_TABLE))
        && tables.contains(QLatin1String(SERVICE_PROPERTY_TABLE))
        && tables.contains(QLatin1String(INTERFACE_PROPERTY_TABLE))){
            bTables = true;
    }
    return bTables;
}

/*
   This function should only ever be used on a user scope database
   It removes an entry from the Defaults table where the default
   refers to an interface implementation in the system scope database.
   The particular default that is removed is specified by
   \a interfaceID.

   May set the last error to one of the following error codes:
   DBError::NoError
   DBError::IfaceIDNotExternal
   DBError::SqlError
   DBError::NoWritePermissions
   DBError::InvalidDatabaseFile
*/
bool ServiceDatabase::removeExternalDefaultServiceInterface(const QString &interfaceID)
{
    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    //begin transaction
    if (!beginTransaction(&query, Write)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::removeExternalDefaultServiceInterface():-"
                    << "Problem: Unable to begin transaction. "
                    << "Reason:" << qPrintable(m_lastError.text());
#endif
        return false;
    }

    QString statement(QLatin1String("SELECT Name FROM Interface WHERE Interface.ID = ?"));
    QList<QVariant> bindValues;
    bindValues.append(interfaceID);
    if (!executeQuery(&query, statement, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::removeDefaultServiceInterface():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }
    if (query.next()) {
        QString interfaceName = query.value(EBindIndex).toString();
        QString errorText(QLatin1String("Local interface implementation exists for interface \"%1\" "
                           "with interfaceID: \"%2\""));
        m_lastError.setError(DBError::IfaceIDNotExternal,
                errorText.arg(interfaceName).arg(interfaceID));
        rollbackTransaction(&query);
        return false;
    }

    statement = QLatin1String("DELETE FROM Defaults WHERE InterfaceID = ? COLLATE NOCASE");
    bindValues.clear();
    bindValues.append(interfaceID);
    if (!executeQuery(&query, statement, bindValues)) {
        rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::removeDefaultServiceInterface():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    //end transaction
    if (!commitTransaction(&query)){
        rollbackTransaction(&query);
        return false;
    }
    m_lastError.setError(DBError::NoError);
    return true;
}

/*
    Removes all tables from the database

    In future this function may be deprecated or removed.

    May set the last error to one of the following error codes:
    DBError::NoError
    DBError::SqlError
    DBError::NoWritePermissions
    DBError::InvalidDatabaseFile
*/
bool ServiceDatabase::dropTables()
{
    //Execute transaction for deleting the database tables
    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);
    QStringList expectedTables;
    expectedTables << QLatin1String(SERVICE_TABLE)
                << QLatin1String(INTERFACE_TABLE)
                << QLatin1String(DEFAULTS_TABLE)
                << QLatin1String(SERVICE_PROPERTY_TABLE)
                << QLatin1String(INTERFACE_PROPERTY_TABLE);

    if (database.tables().count() > 0) {
        if (!beginTransaction(&query, Write)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
            qWarning() << "ServiceDatabase::dropTables():-"
                        << "Unable to begin transaction. "
                        << "Reason:" << qPrintable(m_lastError.text());
#endif
            return false;
        }
        QStringList actualTables = database.tables();

        foreach (const QString expectedTable, expectedTables) {
            if ((actualTables.contains(expectedTable))
                && (!executeQuery(&query, QLatin1String("DROP TABLE ") + expectedTable))) {
                rollbackTransaction(&query);
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
                qWarning() << "ServiceDatabase::dropTables():-"
                           << qPrintable(m_lastError.text());
#endif
                return false;
            }
        }
        if (!commitTransaction(&query)) {
            rollbackTransaction(&query);
            return false;
        }
    }
    m_lastError.setError(DBError::NoError);
    return true;
}

/*
    Checks if the database is open
*/
bool ServiceDatabase::isOpen() const
{
  return m_isDatabaseOpen;
}

/*
   Checks the database connection.

   May set the last error to one of the following error codes:
   DBError::DatabaseNotOpen
   DBError::InvalidDatabaseConnection
*/
bool ServiceDatabase::checkConnection()
{
    if (!m_isDatabaseOpen)
    {
        m_lastError.setError(DBError::DatabaseNotOpen);
        return false;
    }

    if (!QSqlDatabase::database(m_connectionName).isValid())
    {
        m_lastError.setError(DBError::InvalidDatabaseConnection);
        return false;
    }

    return true;
}

/*
   Begins a transcaction based on the \a type which can be Read or Write.

   May set the last error to one of the following error codes:
   DBError::NoError
   DBError::SqlError
   DBError::NoWritePermissions
   DBError::InvalidDatabaseFile
*/
bool ServiceDatabase::beginTransaction(QSqlQuery *query, TransactionType type)
{
    bool success;
    if (type == Read)
        success = query->exec(QLatin1String("BEGIN"));
    else
        success = query->exec(QLatin1String("BEGIN IMMEDIATE"));

    if (!success) {
        int result = query->lastError().number();
        if (result == 26 || result == 11) {//SQLITE_NOTADB || SQLITE_CORRUPT
            qWarning() << "Service Framework:- Database file is corrupt or invalid:" << databasePath();
            m_lastError.setError(DBError::InvalidDatabaseFile, query->lastError().text());
        }
        else if (result == 8) { //SQLITE_READONLY
            qWarning() << "Service Framework:-  Insufficient permissions to write to database:" << databasePath();
            m_lastError.setError(DBError::NoWritePermissions, query->lastError().text());
        }
        else
            m_lastError.setError(DBError::SqlError, query->lastError().text());
        return false;
    }

    m_lastError.setError(DBError::NoError);
    return true;
}

/*
    Commits a transaction

    May set the last error to one of the following error codes:
    DBError::NoError
    DBError::SqlError
*/
bool ServiceDatabase::commitTransaction(QSqlQuery *query)
{
    Q_ASSERT(query != NULL);
    query->finish();
    query->clear();
    if (!query->exec(QLatin1String("COMMIT"))) {
        m_lastError.setError(DBError::SqlError, query->lastError().text());
        return false;
    }
    m_lastError.setError(DBError::NoError);
    return true;
}

/*
    Rolls back a transaction

    May set the last error to one of the following error codes:
    DBError::NoError
    DBError::SqlError
*/
bool ServiceDatabase::rollbackTransaction(QSqlQuery *query)
{
    Q_ASSERT(query !=NULL);
    query->finish();
    query->clear();

    if (!query->exec(QLatin1String("ROLLBACK"))) {
        m_lastError.setError(DBError::SqlError, query->lastError().text());
        return false;
    }
    return true;
}

/*
    Helper function that populates a service \a interface descriptor
    with interface related attributes corresponding to the interface
    represented by \a interfaceID

    It is already assumed that a transaction has been started by the time
    this function is called.  This function will not rollback/commit the
    transaction.

    May set the last error to one of the following error codes:
    DBError::NoError
    DBError::SqlError
*/
bool ServiceDatabase::populateInterfaceProperties(QServiceInterfaceDescriptor *serviceInterface, const QString &interfaceID)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString statement(QLatin1String("SELECT Key, Value FROM InterfaceProperty WHERE InterfaceID = ?"));
    QList<QVariant> bindValues;
    bindValues.append(interfaceID);
    if (!executeQuery(&query, statement, bindValues)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::populateInterfaceProperties():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    bool isFound = false;
    QString attribute;
    while (query.next()) {
        isFound = true;
        attribute = query.value(EBindIndex).toString();
        if (attribute == QLatin1String(INTERFACE_CAPABILITY_KEY)) {
            const QStringList capabilities = query.value(EBindIndex1).toString().split(QLatin1String(","));
            if (capabilities.count() == 1 && capabilities[0].isEmpty()) {
                serviceInterface->d->attributes[QServiceInterfaceDescriptor::Capabilities]
                    = QStringList();
            } else {
                serviceInterface->d->attributes[QServiceInterfaceDescriptor::Capabilities]
                = capabilities;
            }
        } else if (attribute == QLatin1String(INTERFACE_DESCRIPTION_KEY)) {
            serviceInterface->d->attributes[QServiceInterfaceDescriptor::InterfaceDescription]
               = query.value(EBindIndex1).toString();
        } else if (attribute.startsWith(QLatin1String("c_"))) {
            serviceInterface->d->customAttributes[attribute.mid(2)]
               = query.value(EBindIndex1).toString();
        }
    }

    if (!isFound) {
        QString errorText(QLatin1String("Database integrity corrupted, Properties for InterfaceID: %1 does not exist in the InterfaceProperty table for interface \"%2\""));
        m_lastError.setError(DBError::SqlError, errorText.arg(interfaceID).arg(serviceInterface->interfaceName()));
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::populateInterfaceProperties():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        return false;
    }
    m_lastError.setError(DBError::NoError);
    return true;
}

/*
    Helper function that populates a service \a interface descriptor
    with service related attributes corresponding to the service
    represented by \a serviceID

    It is already assumed that a transaction has been started by the time
    this function is called.  This function will not rollback/commit the
    transaction.

    May set the last error to one of the following error codes:
    DBError::NoError
    DBError::SqlError
*/
bool ServiceDatabase::populateServiceProperties(QServiceInterfaceDescriptor *serviceInterface, const QString &serviceID)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString statement(QLatin1String("SELECT Key, Value FROM ServiceProperty WHERE ServiceID = ?"));
    QList<QVariant> bindValues;
    bindValues.append(serviceID);
    if (!executeQuery(&query, statement, bindValues)) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::populateServiceProperties():-"
                    << qPrintable(m_lastError.text());
#endif
        return false;
    }

    bool isFound = false;
    QString attribute;
    while (query.next()) {
        isFound = true;
        attribute = query.value(EBindIndex).toString();
        if (attribute == QLatin1String(SERVICE_DESCRIPTION_KEY)) {
                serviceInterface->d->attributes[QServiceInterfaceDescriptor::ServiceDescription]
                    = query.value(EBindIndex1).toString();
        }
        // fetch initialized and put it as a custom attribute
        if (attribute == QLatin1String(SERVICE_INITIALIZED_KEY)) {
            serviceInterface->d->customAttributes[attribute] = query.value(EBindIndex1).toString();
        }
    }

    if (!isFound) {
        QString errorText(QLatin1String("Database integrity corrupted, Service Properties for ServiceID: \"%1\" does not exist in the ServiceProperty table for service \"%2\""));
        m_lastError.setError(DBError::SqlError, errorText.arg(serviceID).arg(serviceInterface->serviceName()));
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "ServiceDatabase::populateServiceProperties():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        return false;
    }
    m_lastError.setError(DBError::NoError);
    return true;
}

#include "moc_servicedatabase_p.cpp"

QT_END_NAMESPACE
