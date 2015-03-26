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

#include "databasemanager_p.h"
#include "qserviceinterfacedescriptor_p.h"
#include <QFileSystemWatcher>
#include <QHash>

QT_BEGIN_NAMESPACE

DatabaseFileWatcher::DatabaseFileWatcher(DatabaseManager *parent)
    : QObject(parent),
      m_manager(parent),
      m_watcher(0)
{
}

QString DatabaseFileWatcher::closestExistingParent(const QString &path)
{
    if (QFile::exists(path))
        return path;

    int lastSep = path.lastIndexOf(QDir::separator());
    if (lastSep < 0)
        return QString();
    return closestExistingParent(path.mid(0, lastSep));
}

void DatabaseFileWatcher::restartDirMonitoring(const QString &dbPath, const QString &previousDirPath)
{
    if (m_watcher->files().contains(dbPath))
        return;

    QString existing = closestExistingParent(dbPath);
    if (existing.isEmpty()) {
        qWarning() << "QServiceManager: can't find existing directory for path to database" << dbPath
            << "serviceAdded() and serviceRemoved() will not be emitted";
        return;
    }
    if (existing == dbPath) {
        ServiceDatabase *db = 0;
        DatabaseManager::DbScope scope;
        if (m_manager->m_userDb && dbPath == m_manager->m_userDb->databasePath()) {
            db = m_manager->m_userDb;
            scope = DatabaseManager::UserOnlyScope;
        } else if (dbPath == m_manager->m_systemDb->databasePath()) {
            db = m_manager->m_systemDb;
            scope = DatabaseManager::SystemScope;
        }

        if (db) {
            if (!previousDirPath.isEmpty())
                m_watcher->removePath(previousDirPath);
            QMutableListIterator<QString> i(m_monitoredDbPaths);
            while (i.hasNext()) {
                if (i.next() == dbPath)
                    i.remove();
            }

            QStringList newServices = m_manager->getServiceNames(QString(), scope);
            for (int i=0; i<newServices.count(); i++)
                emit m_manager->serviceAdded(newServices[i], scope);
            setEnabled(db, true);
        }
    } else {
        if (previousDirPath != existing) {
            if (!previousDirPath.isEmpty())
                m_watcher->removePath(previousDirPath);
            if (!m_watcher->directories().contains(existing))
                m_watcher->addPath(existing);
            if (!m_monitoredDbPaths.contains(dbPath))
                m_monitoredDbPaths << dbPath;
        }
    }
}

void DatabaseFileWatcher::setEnabled(ServiceDatabase *database, bool enabled)
{
    if (!m_watcher) {
        m_watcher = new QFileSystemWatcher(this);
        connect(m_watcher, SIGNAL(fileChanged(QString)),
            SLOT(databaseChanged(QString)));
        connect(m_watcher, SIGNAL(directoryChanged(QString)),
            SLOT(databaseDirectoryChanged(QString)));
    }

    QString path = database->databasePath();
    if (enabled) {
        if (QFile::exists(path)) {
            if (!database->isOpen())
                database->open();
            m_knownServices[path] = database->getServiceNames(QString());
            m_watcher->addPath(path);
        } else {
            restartDirMonitoring(path, QString());
        }
    } else {
        m_watcher->removePath(path);
        m_knownServices.remove(path);
    }
}

void DatabaseFileWatcher::databaseDirectoryChanged(const QString &path)
{
    for (int i=0; i<m_monitoredDbPaths.count(); i++) {
        if (m_monitoredDbPaths[i].contains(path))
            restartDirMonitoring(m_monitoredDbPaths[i], path);
    }
}

void DatabaseFileWatcher::databaseChanged(const QString &path)
{
    if (m_manager->m_userDb && path == m_manager->m_userDb->databasePath())
        notifyChanges(m_manager->m_userDb, DatabaseManager::UserScope);
    else if (path == m_manager->m_systemDb->databasePath())
        notifyChanges(m_manager->m_systemDb, DatabaseManager::SystemScope);

    // if database was deleted, the path may have been dropped
    if (!m_watcher->files().contains(path) && QFile::exists(path))
        m_watcher->addPath(path);
}

void DatabaseFileWatcher::notifyChanges(ServiceDatabase *database, DatabaseManager::DbScope scope)
{
    QString dbPath = database->databasePath();
    if (!QFile::exists(dbPath)) {
        m_knownServices.remove(dbPath);
        restartDirMonitoring(dbPath, QString());
        return;
    }

    QStringList currentServices = database->getServiceNames(QString());
    if (database->lastError().code() !=DBError::NoError) {
        qWarning("QServiceManager: failed to get current service names for serviceAdded() and serviceRemoved() signals");
        return;
    }

    const QStringList &knownServicesRef = m_knownServices[dbPath];

    QSet<QString> currentServicesSet = currentServices.toSet();
    QSet<QString> knownServicesSet = knownServicesRef.toSet();
    if (currentServicesSet == knownServicesSet)
        return;

    QStringList newServices;
    for (int i=0; i<currentServices.count(); i++) {
        if (!knownServicesSet.contains(currentServices[i]))
            newServices << currentServices[i];
    }

    QStringList removedServices;
    for (int i=0; i<knownServicesRef.count(); i++) {
        if (!currentServicesSet.contains(knownServicesRef[i]))
            removedServices << knownServicesRef[i];
    }

    m_knownServices[dbPath] = currentServices;
    for (int i=0; i<newServices.count(); i++)
        emit m_manager->serviceAdded(newServices[i], scope);
    for (int i=0; i<removedServices.count(); i++)
        emit m_manager->serviceRemoved(removedServices[i], scope);
}

bool lessThan(const QServiceInterfaceDescriptor &d1,
                                        const QServiceInterfaceDescriptor &d2)
{
        return (d1.majorVersion() < d2.majorVersion())
                || ( d1.majorVersion() == d2.majorVersion()
                && d1.minorVersion() < d2.minorVersion());
}

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

/*
   Constructor
*/
DatabaseManager::DatabaseManager()
    : m_userDb(NULL),
      m_systemDb(new ServiceDatabase),
      m_fileWatcher(0),
      m_hasAccessedUserDb(false),
      m_alreadyWarnedOpenError(false)
{
    m_userDb = new ServiceDatabase;
    initDbPath(UserScope);
    initDbPath(SystemScope);
}

/*
   Destructor
*/
DatabaseManager::~DatabaseManager()
{
    delete m_fileWatcher;
    m_fileWatcher = 0;

    //Aside: databases are implicitly closed
    //during deletion
    if (m_userDb) {
        m_userDb->close();
        delete m_userDb;
    }
    m_userDb = 0;

    if (m_systemDb) {
        m_systemDb->close();
        delete m_systemDb;
    }
    m_systemDb = 0;
}


/*
    Initialises database path of m_userDb
    or m_systemDb, but does not open any
    database connections
*/
void DatabaseManager::initDbPath(DbScope scope)
{
    QSettings::Scope settingsScope;
    QString dbIdentifier;
    ServiceDatabase *db;
    if (scope == SystemScope) {
        settingsScope = QSettings::SystemScope;
        dbIdentifier = QLatin1String("_system");
        db = m_systemDb;
    } else {
        settingsScope = QSettings::UserScope;
        dbIdentifier = QLatin1String("_user");
        db = m_userDb;
    }

#ifdef QT_SIMULATOR
    dbIdentifier.append(QLatin1String("_simulator"));
#endif

    QSettings settings(QSettings::IniFormat, settingsScope,
                QLatin1String("Nokia"), QLatin1String("QtServiceFramework"));
    QFileInfo fi(settings.fileName());
    QDir dir = fi.dir();
    QString qtVersion = QLatin1String(qVersion());
    qtVersion = qtVersion.left(qtVersion.size() -2); //strip off patch version
    QString dbName = QString(QLatin1String("QtServiceFramework_")) + qtVersion + dbIdentifier + QLatin1String(".db");
    db->setDatabasePath(dir.path() + QDir::separator() + dbName);
}

/*
    Adds the details \a  service into the service database corresponding to
    \a scope.

    Returns true if the operation succeeded and false otherwise.
    The last error is set when this function is called.
*/
bool DatabaseManager::registerService(ServiceMetaDataResults &service, DbScope scope)
{
    if (scope == DatabaseManager::SystemScope) {
        if (!openDb(DatabaseManager::SystemScope)) {
            return false;
        }  else {
            if (!m_systemDb->registerService(service)) {
                m_lastError = m_systemDb->lastError();
                return false;
            } else { //must be successful registration
                m_lastError.setError(DBError::NoError);
                return true;
            }
        }
    } else { //must  be registering service at user scope
        if (!openDb(DatabaseManager::UserScope)) {
            return false;
        } else {
            if (!m_userDb->registerService(service)) {
                m_lastError = m_userDb->lastError();
                return false;
            } else { //must be successful registration
                m_lastError.setError(DBError::NoError);
                return true;
            }
        }
    }
}

/*
    Removes the details of \serviceName from the database corresponding to \a
    scope.

    Returns true if the operation succeeded, false otherwise.
    The last error is set when this function is called.
*/
bool DatabaseManager::unregisterService(const QString &serviceName, DbScope scope)
{
    if (scope == DatabaseManager::SystemScope) {
        if (!openDb(DatabaseManager::SystemScope))
            return false;
   else {
            if (!m_systemDb->unregisterService(serviceName)) {
                m_lastError = m_systemDb->lastError();
                return false;
            } else { //must be successful unregistration
                m_lastError.setError(DBError::NoError);
                return true;
            }
        }
    } else {
        if (!openDb(DatabaseManager::UserScope)) {
            return false;
        } else {
            if (!m_userDb->unregisterService(serviceName)){
                m_lastError = m_userDb->lastError();
                return false;
            } else { //must be successful unregistration
                m_lastError.setError(DBError::NoError);
                return true;
            }
        }
    }
}

/*
    Removes the initialization specific information of \serviceName from the database
    corresponding to a \scope.

    Returns true if teh operation succeeded, false otherwise.
    The last error is set when this function is called.
  */
bool DatabaseManager::serviceInitialized(const QString &serviceName, DbScope scope)
{
    ServiceDatabase *db = (scope == DatabaseManager::SystemScope) ? m_systemDb : m_userDb;

    if (!openDb(scope)) {
        return false;
    } else {
        if (!db->serviceInitialized(serviceName)) {
            m_lastError = db->lastError();
            return false;
        } else {
            m_lastError.setError(DBError::NoError);
            return true;
        }
    }
}

/*
    Retrieves a list of interface descriptors that fulfill the constraints specified
    by \a filter at a given \a scope.

    The last error is set when this function is called.
*/
QList<QServiceInterfaceDescriptor>  DatabaseManager::getInterfaces(const QServiceFilter &filter, DbScope scope)
{
    QList<QServiceInterfaceDescriptor> descriptors;

    int userDescriptorCount = 0;
    if (scope == UserScope) {
        if (!openDb(UserScope))
            return descriptors;

        descriptors =  m_userDb->getInterfaces(filter);
        if (m_userDb->lastError().code() != DBError::NoError ) {
            descriptors.clear();
            m_lastError = m_userDb->lastError();
            return descriptors;
        }

        userDescriptorCount = descriptors.count();
        for (int i=0; i < userDescriptorCount; ++i) {
            descriptors[i].d->scope = QService::UserScope;
        }
    }

    if (openDb(SystemScope)) {
        descriptors.append(m_systemDb->getInterfaces(filter));
        if (m_systemDb->lastError().code() != DBError::NoError) {
            descriptors.clear();
            m_lastError = m_systemDb->lastError();
            return descriptors;
        }

        for (int i = userDescriptorCount; i < descriptors.count(); ++i)
            descriptors[i].d->scope = QService::SystemScope;
    } else {
        if ( scope == SystemScope) {
            //openDb() should already have handled lastError
            descriptors.clear();
            return descriptors;
        }
    }

    m_lastError.setError(DBError::NoError);
    return descriptors;
}


/*
    Retrieves a list of the names of services that provide the interface
    specified by \a interfaceName.

    The last error is set when this function is called.
*/
QStringList DatabaseManager::getServiceNames(const QString &interfaceName, DatabaseManager::DbScope scope)
{
    QStringList serviceNames;
    if (scope == UserScope || scope == UserOnlyScope) {
        if (!openDb(DatabaseManager::UserScope))
            return serviceNames;
        serviceNames = m_userDb->getServiceNames(interfaceName);
        if (m_userDb->lastError().code() != DBError::NoError) {
            serviceNames.clear();
            m_lastError = m_userDb->lastError();
            return serviceNames;
        }
        if (scope == UserOnlyScope) {
            m_lastError.setError(DBError::NoError);
            return serviceNames;
        }
    }

    if (openDb(DatabaseManager::SystemScope)) {
        QStringList systemServiceNames;
        systemServiceNames = m_systemDb->getServiceNames(interfaceName);
        if (m_systemDb->lastError().code() != DBError::NoError) {
            serviceNames.clear();
            m_lastError = m_systemDb->lastError();
            return serviceNames;
        }
        foreach (const QString &systemServiceName, systemServiceNames) {
            if (!serviceNames.contains(systemServiceName, Qt::CaseInsensitive))
                serviceNames.append(systemServiceName);
        }

    } else {
        if ( scope == SystemScope) {
            //openDb() should have already handled lastError
            serviceNames.clear();
            return serviceNames;
        }
    }

    m_lastError.setError(DBError::NoError);
    return serviceNames;
}

/*
    Returns the default interface implementation descriptor for a given
    \a interfaceName and \a scope.

    The last error is set when this function is called.
*/
QServiceInterfaceDescriptor DatabaseManager::interfaceDefault(const QString &interfaceName, DbScope scope)
{
    QServiceInterfaceDescriptor descriptor;
    if (scope == UserScope) {
        if (!openDb(UserScope))
            return QServiceInterfaceDescriptor();
        QString interfaceID;
        descriptor = m_userDb->interfaceDefault(interfaceName, &interfaceID);

        if (m_userDb->lastError().code() == DBError::NoError) {
            descriptor.d->scope = QService::UserScope;
            return descriptor;
        } else if (m_userDb->lastError().code() == DBError::ExternalIfaceIDFound) {
            //default hasn't been found in user db, but we have found an ID
            //that may refer to an interface implementation in the system db
            if (!openDb(SystemScope)) {
                QString errorText(QLatin1String("No default service found for interface: \"%1\""));
                m_lastError.setError(DBError::NotFound, errorText.arg(interfaceName));
                return QServiceInterfaceDescriptor();
            }

            descriptor = m_systemDb->getInterface(interfaceID);
            //found the service from the system database
            if (m_systemDb->lastError().code() == DBError::NoError) {
                m_lastError.setError(DBError::NoError);
                descriptor.d->scope = QService::SystemScope;
                return descriptor;
            } else if (m_systemDb->lastError().code() == DBError::NotFound) {
                //service implementing interface doesn't exist in the system db
                //so the user db must contain a stale entry so remove it
                m_userDb->removeExternalDefaultServiceInterface(interfaceID);

                QList<QServiceInterfaceDescriptor> descriptors;
                descriptors = getInterfaces(QServiceFilter(interfaceName), UserScope);

                //make the latest interface implementation the new
                //default if there is one
                if (descriptors.count() > 0 ) {
                    descriptor = latestDescriptor(descriptors);
                    setInterfaceDefault(descriptor, UserScope);
                    m_lastError.setError(DBError::NoError);
                    return descriptor;
                } else {
                    QString errorText(QLatin1String("No default service found for interface: \"%1\""));
                    m_lastError.setError(DBError::NotFound, errorText.arg(interfaceName));
                    return QServiceInterfaceDescriptor();
                }
            } else {
                m_lastError.setError(DBError::NoError);
                return QServiceInterfaceDescriptor();
            }
        } else if (m_userDb->lastError().code() == DBError::NotFound) {
            //do nothing, the search for a default in the system db continues
            //further down
        } else { //error occurred at user db level, so return
            m_lastError = m_userDb->lastError();
            return QServiceInterfaceDescriptor();
        }
    }

    //search at system scope because we haven't found a default at user scope
    //or because we're specifically only querying at system scope
    if (!openDb(SystemScope)) {
        if (scope == SystemScope) {
            m_lastError = m_systemDb->lastError();
            return QServiceInterfaceDescriptor();
        } else if (scope == UserScope && m_userDb && m_userDb->lastError().code() == DBError::NotFound) {
            m_lastError = m_userDb->lastError();
            return QServiceInterfaceDescriptor();
        }
    } else {
        descriptor = m_systemDb->interfaceDefault(interfaceName);
        if (m_systemDb->lastError().code() == DBError::NoError) {
            descriptor.d->scope = QService::SystemScope;
            return descriptor;
        } else if (m_systemDb->lastError().code() == DBError::NotFound) {
            m_lastError = m_systemDb->lastError();
            return QServiceInterfaceDescriptor();
        } else {
            m_lastError = m_systemDb->lastError();
            return QServiceInterfaceDescriptor();
        }
    }

    //should not be possible to reach here
    m_lastError.setError(DBError::UnknownError);
    return QServiceInterfaceDescriptor();
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
    QList<QServiceInterfaceDescriptor> descriptors;
    QServiceFilter filter;
    filter.setServiceName(serviceName);
    filter.setInterface(interfaceName);

    descriptors = getInterfaces(filter, scope);
    if (m_lastError.code() != DBError::NoError)
        return false;

    if (descriptors.count() == 0) {
        QString errorText(QLatin1String("No implementation for interface \"%1\" "
                "found for service \"%2\""));
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
    if (scope == UserScope) {
        if (!openDb(UserScope))
            return false;
        if (descriptor.scope() == QService::UserScope) { //if a user scope descriptor, just set it in the user db
            if (m_userDb->setInterfaceDefault(descriptor)) {
                m_lastError.setError(DBError::NoError);
                return true;
            } else {
                m_lastError = m_userDb->lastError();
                return false;
            }
        } else { //otherwise we need to get the interfaceID from the system db and set this
            //as an external default interface ID in the user db
            if (!openDb(SystemScope))
                return false;

            QString interfaceDescriptorID = m_systemDb->getInterfaceID(descriptor);
            if (m_systemDb->lastError().code() == DBError::NoError) {
                if (m_userDb->setInterfaceDefault(descriptor, interfaceDescriptorID)) {
                    m_lastError.setError(DBError::NoError);
                    return true;
                } else {
                    m_lastError = m_userDb->lastError();
                    return false;
                }
            } else {
                m_lastError = m_systemDb->lastError();
                return false;
            }
        }
    } else {  //scope == SystemScope
        if (descriptor.scope() == QService::UserScope) {
            QString errorText(QLatin1String("Cannot set default service at system scope with a user scope "
                                            "interface descriptor"));
            m_lastError.setError(DBError::InvalidDescriptorScope, errorText);
            return false;
        } else {
            if (!openDb(SystemScope)) {
                return false;
            } else {
                if (m_systemDb->setInterfaceDefault(descriptor)) {
                    m_lastError.setError(DBError::NoError);
                    return true;
                } else {
                    m_lastError = m_systemDb->lastError();
                    return false;
                }
            }
        }
    }
}

/*
    Opens a database connection with the database at a specific \a scope.

    The last error is set when this function is called.
*/
bool DatabaseManager::openDb(DbScope scope)
{
    if (scope == SystemScope && m_systemDb->isOpen() && !QFile::exists(m_systemDb->databasePath())) {
        delete m_systemDb;
        m_systemDb = new ServiceDatabase;
        initDbPath(SystemScope);
        m_alreadyWarnedOpenError = false;
    } else if (scope != SystemScope && m_userDb->isOpen() && !QFile::exists(m_userDb->databasePath())) {
        delete m_userDb;
        m_userDb = new ServiceDatabase;
        initDbPath(UserScope);
        m_alreadyWarnedOpenError = false;
    }

    ServiceDatabase *db;
    if (scope == SystemScope) {
        db = m_systemDb;
    }
    else {
        db = m_userDb;
        m_hasAccessedUserDb = true;
    }

    if (db->isOpen())
        return true;

    bool isOpen = db->open();
    if (!isOpen) {
#ifdef QT_SFW_SERVICEDATABASE_DEBUG
        qWarning() << "DatabaseManger::openDb():-"
                    << "Problem:" << qPrintable(m_lastError.text());
#endif
        if (scope == SystemScope && m_hasAccessedUserDb == true) {
                if (QFile::exists(m_systemDb->databasePath()) && !m_alreadyWarnedOpenError)
                    qWarning() << "Service Framework:- Unable to access system database for a user scope "
                        "operation; resorting to using only the user database.  Future operations "
                        "will attempt to access the system database but no further warnings will be issued";
        }

        QString warning;
        if (db->lastError().code() == DBError::InvalidDatabaseFile) {
            warning = QString(QLatin1String("Service Framework:- Database file is corrupt or invalid: ")) + db->databasePath();
            m_lastError = db->lastError();
        } else {
            warning = QString(QLatin1String("Service Framework:- Unable to open or create database at: ")) +  db->databasePath();
            QString errorText(QLatin1String("Unable to open service framework database: %1"));
            m_lastError.setError(DBError::CannotOpenServiceDb,
                errorText.arg(db->databasePath()));
        }

        if (m_alreadyWarnedOpenError
                || (scope == SystemScope && m_hasAccessedUserDb && !QFile::exists(m_systemDb->databasePath()))) {
            //do nothing, don't output warning if already warned or we're accessing the system database
            //from user scope and the system database doesn't exist
        } else {
            qWarning() << qPrintable(warning);
            m_alreadyWarnedOpenError = true;
        }

        return false;
    }

    //if we are opening the system database while the user database is open,
    //cleanup and reset any old external defaults
    //from the user scope database
    if (scope == SystemScope && m_userDb && m_userDb->isOpen()) {
        QList<QPair<QString,QString> > externalDefaultsInfo;
        externalDefaultsInfo = m_userDb->externalDefaultsInfo();
        QServiceInterfaceDescriptor descriptor;
        QPair<QString,QString> defaultInfo;

        for (int i = 0; i < externalDefaultsInfo.count(); ++i) {
            defaultInfo = externalDefaultsInfo[i];
            descriptor = m_userDb->getInterface(defaultInfo.second);
            if (m_userDb->lastError().code() == DBError::NotFound) {
                m_userDb->removeExternalDefaultServiceInterface(defaultInfo.second);
                QList<QServiceInterfaceDescriptor> descriptors;
                descriptors = getInterfaces(QServiceFilter(defaultInfo.first), UserScope);

                if (descriptors.count() > 0 ) {
                    descriptor = latestDescriptor(descriptors);
                    setInterfaceDefault(descriptor, UserScope);
                }
            }
        }
    }

    m_lastError.setError(DBError::NoError);
    return true;
}

/*
    Returns the interface descriptor with the highest version from the
    list of interface \a descriptors
*/
QServiceInterfaceDescriptor DatabaseManager::latestDescriptor(
                                    const QList<QServiceInterfaceDescriptor> &descriptors)
{
    if (descriptors.count() == 0)
        return QServiceInterfaceDescriptor();

    int latestIndex = 0;
    for (int i = 1; i < descriptors.count(); ++i) {
        if (lessThan(descriptors[latestIndex], descriptors[i]))
            latestIndex = i;
    }

    return descriptors[latestIndex];
}

/*
    Sets whether change notifications for added and removed services are
    \a enabled or not at a given \a scope.
*/
void DatabaseManager::setChangeNotificationsEnabled(DbScope scope, bool enabled)
{
    if (!m_fileWatcher) m_fileWatcher = new
    DatabaseFileWatcher(this); m_fileWatcher->setEnabled(scope == SystemScope ?
            m_systemDb : m_userDb, enabled);
}

#include "moc_databasemanager_p.cpp"

QT_END_NAMESPACE
