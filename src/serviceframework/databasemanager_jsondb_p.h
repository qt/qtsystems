/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef DATABASEMANAGER_JSON_H_
#define DATABASEMANAGER_JSON_H_

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QEventLoop>

#include "qserviceinterfacedescriptor_p.h"
#include "servicemetadata_p.h"
#include "qservicefilter.h"
#include "dberror_p.h"

#include <QtJsonDb/qjsondbglobal.h>

#ifdef QT_SFW_SERVICEDATABASE_GENERATE
#undef Q_AUTOTEST_EXPORT
#define Q_AUTOTEST_EXPORT
#endif

QT_BEGIN_NAMESPACE_JSONDB
class QJsonDbConnection;
class QJsonDbRequest;
class QJsonDbWatcher;
QT_END_NAMESPACE_JSONDB

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE


class Q_AUTOTEST_EXPORT DatabaseManager : public QObject
{
    Q_OBJECT

    public:
        enum DbScope{UserScope, SystemScope, UserOnlyScope};
        DatabaseManager(void);
        virtual ~DatabaseManager();

        bool registerService(ServiceMetaDataResults &service, DbScope scope);
        bool unregisterService(const QString &serviceName, DbScope scope);
        bool serviceInitialized(const QString &serviceName, DbScope scope);

        QList<QServiceInterfaceDescriptor> getInterfaces(const QServiceFilter &filter, DbScope scope);
        QStringList getServiceNames(const QString &interfaceName, DbScope scope);

        QServiceInterfaceDescriptor interfaceDefault(const QString &interfaceName, DbScope scope);
        bool setInterfaceDefault(const QString &serviceName, const QString &interfaceName, DbScope scope);
        bool setInterfaceDefault(const QServiceInterfaceDescriptor &interface, DbScope scope);

        DBError lastError(){ return m_lastError;}

        void setChangeNotificationsEnabled(DbScope scope, bool enabled);

    signals:
        void serviceAdded(const QString &serviceName, DatabaseManager::DbScope scope);
        void serviceRemoved(const QString &serviceName, DatabaseManager::DbScope scope);

    private slots:
        void onNotificationsAvailable();

    private:
        QT_PREPEND_NAMESPACE_JSONDB(QJsonDbConnection) *db;
        QT_PREPEND_NAMESPACE_JSONDB(QJsonDbWatcher) *dbwatcher;
        DBError m_lastError;
        QEventLoop m_eventLoop;
        bool m_notenabled;
        QHash<QString, int> m_services;

    private:
        bool sendRequest(QT_PREPEND_NAMESPACE_JSONDB(QJsonDbRequest) *r);
};

QT_END_NAMESPACE
QT_END_HEADER

#endif
