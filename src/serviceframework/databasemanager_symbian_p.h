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

#ifndef DATABASEMANAGER_SYMBIAN_P_H
#define DATABASEMANAGER_SYMBIAN_P_H

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

#if defined(__WINS__) && !defined(SYMBIAN_EMULATOR_SUPPORTS_PERPROCESS_WSD)
//Use DatabaseManager "directly" in emulators where per process WSD is not
//supported.
#include "databasemanager_p.h"
#else //defined(__WINS__) && !defined(SYMBIAN_EMULATOR_SUPPORTS_PERPROCESS_WSD)

#include "qserviceframeworkglobal.h"
#include <QObject>
#include <QList>

#include <servicemetadata_p.h>
#include "dberror_p.h"
#include <e32base.h>


QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class CDatabaseManagerServerThread;
class QServiceFilter;
typedef TPckgBuf<TInt> TError;

class QServiceInterfaceDescriptor;

class RDatabaseManagerSession : public RSessionBase
    {
    public:
        enum DbScope{UserScope, SystemScope, UserOnlyScope};
        RDatabaseManagerSession();

    public:
        TInt Connect();
        void Close();
        TVersion Version() const;

        bool RegisterService(ServiceMetaDataResults& aService);
        bool UnregisterService(const QString& aServiceName);
        bool ServiceInitialized(const QString& aServiceName);

        QList<QServiceInterfaceDescriptor> Interfaces(const QServiceFilter& aFilter);
        QStringList ServiceNames(const QString& aInterfaceName);

        QServiceInterfaceDescriptor InterfaceDefault(const QString& aInterfaceName);
        bool SetInterfaceDefault(const QString& aServiceName, const QString& aInterfaceName);
        bool SetInterfaceDefault(const QServiceInterfaceDescriptor& aInterface);

        DBError LastError();

        void SetChangeNotificationsEnabled(bool aEnabled);

        void NotifyServiceSignal(TRequestStatus& aStatus);
        void CancelNotifyServiceSignal() const;

    public:
        TBuf<255> iServiceName;
        TPckgBuf<TInt> iState;

    private:
        TInt StartServer();

    private:
        TIpcArgs iArgs;
        TError iError;
    };

class DatabaseManagerSignalMonitor;

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

        DBError lastError();

        void setChangeNotificationsEnabled(DbScope scope, bool enabled);

    signals:
        void serviceAdded(const QString &serviceName, DatabaseManager::DbScope scope);
        void serviceRemoved(const QString &serviceName, DatabaseManager::DbScope scope);

    private:
        enum State{EAdded, ERemoved};
        void notifyServiceSignal();

    private:
        friend class DatabaseManagerSignalMonitor;
        DatabaseManagerSignalMonitor* iDatabaseManagerSignalMonitor;
	    RDatabaseManagerSession iSession;
        QServiceInterfaceDescriptor latestDescriptor(const QList<QServiceInterfaceDescriptor> &descriptors);
};

class DatabaseManagerSignalMonitor : public CActive
{
    public:
        DatabaseManagerSignalMonitor(DatabaseManager& databaseManager, RDatabaseManagerSession& databaseManagerSession);
        ~DatabaseManagerSignalMonitor();

    protected:
        void issueNotifyServiceSignal();

    protected: // From CActive
        void DoCancel();
        void RunL();
    private:
        DatabaseManager& iDatabaseManager;
        RDatabaseManagerSession& iDatabaseManagerSession;
};

QT_END_NAMESPACE
QT_END_HEADER

#endif //defined(__WINS__) && !defined(SYMBIAN_EMULATOR_SUPPORTS_PERPROCESS_WSD)
#endif //DATABASEMANAGER_SYMBIAN_P_H
