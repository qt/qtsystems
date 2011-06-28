/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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

#if defined(__WINS__) && !defined(SYMBIAN_EMULATOR_SUPPORTS_PERPROCESS_WSD)
//Use DatabaseManager "directly" in emulators where per process WSD is not
//supported.
#include "databasemanager.cpp"
#include "servicedatabase.cpp"
#else //defined(__WINS__) && !defined(SYMBIAN_EMULATOR_SUPPORTS_PERPROCESS_WSD)

#ifdef QTM_BUILD_UNITTESTS
#include "servicedatabase.cpp"
#endif

#include "databasemanager_symbian_p.h"
#include "clientservercommon.h"
#include <qserviceinterfacedescriptor_p.h>
#include <qserviceinterfacedescriptor.h>
#include <qservicefilter.h>
#include <QProcess>
#include <QDebug>
#include <s32mem.h>

QT_BEGIN_NAMESPACE

/*
    \class DatabaseManager
    \ingroup servicesfw
    \brief The database manager is responsible for receiving queries
    about services and managing user and system scope databases in order to
    respond to those queries.

    The DatabaseManager provides operations for
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
    \fn DatabaseManager::DatabaseManager()

   Constructor
*/
DatabaseManager::DatabaseManager()
{
    TInt err = iSession.Connect();
    if (err != KErrNone)
        qt_symbian_throwIfError(err);

    iDatabaseManagerSignalMonitor = new DatabaseManagerSignalMonitor(*this, iSession);
}

/*
    \fn DatabaseManager::~DatabaseManager()

   Destructor
*/
DatabaseManager::~DatabaseManager()
{
    delete iDatabaseManagerSignalMonitor;
    iSession.Close();
}

/*
    \fn bool DatabaseManager::registerService(ServiceMetaDataResults &service, DbScope scope)

    Adds the details \a  service into the service database corresponding to
    \a scope.

    Returns true if the operation succeeded and false otherwise.
    The last error is set when this function is called.
*/
bool DatabaseManager::registerService(ServiceMetaDataResults &service, DbScope scope)
{
    Q_UNUSED(scope);
    return iSession.RegisterService(service);
}

/*
    \fn bool DatabaseManager::unregisterService(const QString &serviceName, DbScope scope)

    Removes the details of \a serviceName from the database corresponding to \a
    scope.

    Returns true if the operation succeeded, false otherwise.
    The last error is set when this function is called.
*/
bool DatabaseManager::unregisterService(const QString &serviceName, DbScope scope)
{
    Q_UNUSED(scope);
    return iSession.UnregisterService(serviceName);
}

/*
    Removes the initialization specific information of \serviceName from the database.

    Returns true if the operation succeeded, false otherwise.
    The last error is set when this function is called.
  */
bool DatabaseManager::serviceInitialized(const QString &serviceName, DbScope scope)
{
    Q_UNUSED(scope);
    return iSession.ServiceInitialized(serviceName);
}

/*
    \fn QList<QServiceInterfaceDescriptor>  DatabaseManager::getInterfaces(const QServiceFilter &filter, DbScope scope)

    Retrieves a list of interface descriptors that fulfill the constraints specified
    by \a filter at a given \a scope.

    The last error is set when this function is called.
*/
QList<QServiceInterfaceDescriptor>  DatabaseManager::getInterfaces(const QServiceFilter &filter, DbScope scope)
{
    Q_UNUSED(scope);
    return iSession.Interfaces(filter);
}


/*
    \fn QStringList DatabaseManager::getServiceNames(const QString &interfaceName, DbScope scope)

    Retrieves a list of the names of services that provide the interface
    specified by \a interfaceName.

    The last error is set when this function is called.
*/
QStringList DatabaseManager::getServiceNames(const QString &interfaceName, DbScope scope)
{
    Q_UNUSED(scope);
    return iSession.ServiceNames(interfaceName);
}

/*
    \fn  QServiceInterfaceDescriptor DatabaseManager::interfaceDefault(const QString &interfaceName, DbScope scope)

    Returns the default interface implementation descriptor for a given
    \a interfaceName and \a scope.

    The last error is set when this function is called.
*/
QServiceInterfaceDescriptor DatabaseManager::interfaceDefault(const QString &interfaceName, DbScope scope)
{
    Q_UNUSED(scope);
    return iSession.InterfaceDefault(interfaceName);
}

/*
    \fn  bool DatabaseManager::setInterfaceDefault(const QString &serviceName, const   QString &interfaceName, DbScope scope)

    Sets the default interface implemenation for \a interfaceName to the matching
    interface implementation provided by \a service.

    If \a service provides more than one interface implementation, the newest
    version of the interface is set as the default.

    Returns true if the operation was succeeded, false otherwise
    The last error is set when this function is called.
*/
bool DatabaseManager::setInterfaceDefault(const QString &serviceName, const
        QString &interfaceName, DbScope scope) {
        Q_UNUSED(scope);
        return iSession.SetInterfaceDefault(serviceName, interfaceName);
}

/*
    \fn  bool DatabaseManager::setInterfaceDefault(const QServiceInterfaceDescriptor &descriptor, DbScope scope)

    Sets the interface implementation specified by \a descriptor to be the default
    implementation for the particular interface specified in the descriptor.

    Returns true if the operation succeeded, false otherwise.
    The last error is set when this function is called.
*/
bool DatabaseManager::setInterfaceDefault(const QServiceInterfaceDescriptor &descriptor, DbScope scope)
{
    Q_UNUSED(scope);
    return iSession.SetInterfaceDefault(descriptor);
}

/*
    \fn  void DatabaseManager::setChangeNotificationsEnabled(DbScope scope, bool enabled)

    Sets whether change notifications for added and removed services are
    \a enabled or not at a given \a scope.
*/
void DatabaseManager::setChangeNotificationsEnabled(DbScope scope, bool enabled)
{
    Q_UNUSED(scope);
    iSession.SetChangeNotificationsEnabled(enabled);
}

DatabaseManagerSignalMonitor::DatabaseManagerSignalMonitor(
    DatabaseManager& databaseManager, RDatabaseManagerSession& databaseManagerSession) :
    CActive(EPriorityNormal),
    iDatabaseManager(databaseManager), iDatabaseManagerSession(databaseManagerSession)
{
    CActiveScheduler::Add(this);
    issueNotifyServiceSignal();
}

DatabaseManagerSignalMonitor::~DatabaseManagerSignalMonitor()
{
    Cancel();
}

void DatabaseManagerSignalMonitor::issueNotifyServiceSignal()
{
    iDatabaseManagerSession.NotifyServiceSignal(iStatus);
    SetActive();
}

DBError DatabaseManager::lastError()
{
    return iSession.LastError();
}

void DatabaseManagerSignalMonitor::DoCancel()
{
    iDatabaseManagerSession.CancelNotifyServiceSignal();
}

void DatabaseManagerSignalMonitor::RunL()
{
    switch (iStatus.Int())
    {
        case ENotifySignalComplete:
        {
            QString serviceName = QString::fromUtf16(iDatabaseManagerSession.iServiceName.Ptr(), iDatabaseManagerSession.iServiceName.Length());

            if ((DatabaseManager::State)iDatabaseManagerSession.iState() == DatabaseManager::EAdded)
            {
                emit iDatabaseManager.serviceAdded(serviceName, DatabaseManager::SystemScope);
            }
            else if ((DatabaseManager::State)iDatabaseManagerSession.iState() == DatabaseManager::ERemoved)
            {
                emit iDatabaseManager.serviceRemoved(serviceName, DatabaseManager::SystemScope);
            }
            issueNotifyServiceSignal();
            break;
        }
        default:
        {

        }
        break;
    }
}


RDatabaseManagerSession::RDatabaseManagerSession()
    : RSessionBase()
    {
    }

TVersion RDatabaseManagerSession::Version() const
    {
    return TVersion(KServerMajorVersionNumber, KServerMinorVersionNumber, KServerBuildVersionNumber);
    }

TInt RDatabaseManagerSession::Connect()
    {
    TInt retryCount = 2;
    for (;;)
        {
        TInt err = CreateSession(KDatabaseManagerServerName, TVersion(), 8, EIpcSession_Sharable);
        if (err != KErrNotFound && err != KErrServerTerminated)
            return err;
        if (--retryCount == 0)
            return err;
        err = StartServer();
        if (err != KErrNone && err != KErrAlreadyExists)
            return err;
        }
    }

TInt RDatabaseManagerSession::StartServer()
    {
    TInt ret = KErrNone;
    TFindServer findServer(KDatabaseManagerServerName);
    TFullName name;

    if (findServer.Next(name) != KErrNone)
    {
        TRequestStatus status;
        RProcess dbServer;	
        ret = dbServer.Create(KDatabaseManagerServerProcess, KNullDesC);
        if(ret != KErrNone)
            {
            return ret;
            }
        dbServer.Rendezvous(status);
        if(status != KRequestPending)
            {
            dbServer.Kill(KErrNone);
            dbServer.Close();
            return KErrGeneral;
            }
        else
            {
            dbServer.Resume();
            }

        User::WaitForRequest(status);
        if(status != KErrNone)
            {
            dbServer.Close();
            return status.Int();
            }
        dbServer.Close();
    }

    return ret;
    }

void RDatabaseManagerSession::Close()
    {
    RSessionBase::Close();
    }

bool RDatabaseManagerSession::RegisterService(ServiceMetaDataResults& aService)
    {
    QByteArray serviceByteArray;
    QDataStream in(&serviceByteArray, QIODevice::WriteOnly);
    in.setVersion(QDataStream::Qt_4_6);
    in << aService;
    TPtrC8 ptr8((TUint8*)(serviceByteArray.constData()), serviceByteArray.size());
    TIpcArgs args(&ptr8, &iError);
    SendReceive(ERegisterServiceRequest, args);

    return (iError() == DBError::NoError);
    }

bool RDatabaseManagerSession::UnregisterService(const QString& aServiceName)
    {
    TPtrC serviceNamePtr(reinterpret_cast<const TUint16*>(aServiceName.utf16()));
    TIpcArgs args(&serviceNamePtr, &iError);
    SendReceive(EUnregisterServiceRequest, args);

    return (iError() == DBError::NoError);
    }

bool RDatabaseManagerSession::ServiceInitialized(const QString& aServiceName)
    {
    TPtrC serviceNamePtr(reinterpret_cast<const TUint16*>(aServiceName.utf16()));
    TIpcArgs args(&serviceNamePtr, &iError);
    SendReceive(EServiceInitializedRequest, args);

    return (iError() == DBError::NoError);
    }


QList<QServiceInterfaceDescriptor> RDatabaseManagerSession::Interfaces(const QServiceFilter& aFilter)
    {
    QByteArray filterByteArray;
    QDataStream in(&filterByteArray, QIODevice::WriteOnly);
    in.setVersion(QDataStream::Qt_4_6);
    in << aFilter;
    TPtrC8 ptr8((TUint8*)(filterByteArray.constData()), filterByteArray.size());
    TPckgBuf<TInt> lengthPckg(0);
    TIpcArgs args(&ptr8, &lengthPckg, &iError);
    SendReceive(EGetInterfacesSizeRequest, args);

    HBufC8* descriptorListBuf = HBufC8::New(lengthPckg());
    TPtr8 ptrToBuf(descriptorListBuf->Des());
    TIpcArgs args2(&ptrToBuf);
    SendReceive(EGetInterfacesRequest, args2);

    QByteArray descriptorListByteArray((const char*)ptrToBuf.Ptr(), ptrToBuf.Length());
    QDataStream out(descriptorListByteArray);
    QList<QServiceInterfaceDescriptor> descriptorList;
    out >> descriptorList;

    delete descriptorListBuf;

    return descriptorList;
    }

QStringList RDatabaseManagerSession::ServiceNames(const QString& aInterfaceName)
    {
    TPtrC interfaceNamePtr(reinterpret_cast<const TUint16*>(aInterfaceName.utf16()));
    HBufC* interfaceNamebuf = HBufC::New(interfaceNamePtr.Length());
    interfaceNamebuf->Des().Copy(interfaceNamePtr);
    TPckgBuf<TInt> lengthPckg(0);
    TIpcArgs args(interfaceNamebuf, &lengthPckg, &iError);
    SendReceive(EGetServiceNamesSizeRequest, args);

    HBufC8* serviceNamesBuf = HBufC8::New(lengthPckg());
    TPtr8 ptrToBuf(serviceNamesBuf->Des());
    TIpcArgs args2(&ptrToBuf);
    SendReceive(EGetServiceNamesRequest, args2);

    QByteArray nameListByteArray((const char*)ptrToBuf.Ptr(), ptrToBuf.Length());
    QDataStream out(nameListByteArray);
    QStringList nameList;
    out >> nameList;

    delete interfaceNamebuf;
    delete serviceNamesBuf;

    return nameList;
    }

QServiceInterfaceDescriptor RDatabaseManagerSession::InterfaceDefault(const QString& aInterfaceName)
    {
    TPtrC interfaceNamePtr(reinterpret_cast<const TUint16*>(aInterfaceName.utf16()));
    HBufC* interfaceNameBuf = HBufC::New(interfaceNamePtr.Length());
    interfaceNameBuf->Des().Copy(interfaceNamePtr);
    TPckgBuf<TInt> lengthPckg(0);
    TIpcArgs args(interfaceNameBuf, &lengthPckg, &iError);
    SendReceive(EInterfaceDefaultSizeRequest, args);

    HBufC8* interfaceDescriptorBuf = HBufC8::New(lengthPckg());
    TPtr8 ptrToBuf(interfaceDescriptorBuf->Des());
    TIpcArgs args2(&ptrToBuf);
    SendReceive(EInterfaceDefaultRequest, args2);

    QByteArray interfaceDescriptorByteArray((const char*)interfaceDescriptorBuf->Ptr(), interfaceDescriptorBuf->Length());
    QDataStream out(interfaceDescriptorByteArray);
    QServiceInterfaceDescriptor interfaceDescriptor;
    out >> interfaceDescriptor;

    delete interfaceNameBuf;
    delete interfaceDescriptorBuf;

    return interfaceDescriptor;
    }

bool RDatabaseManagerSession::SetInterfaceDefault(const QString &aServiceName, const QString &aInterfaceName)
    {
    TPtrC serviceNamePtr(reinterpret_cast<const TUint16*>(aServiceName.utf16()));
    TPtrC interfaceNamePtr(reinterpret_cast<const TUint16*>(aInterfaceName.utf16()));
    TIpcArgs args(&serviceNamePtr, &interfaceNamePtr, &iError);
    SendReceive(ESetInterfaceDefault, args);

    return (iError() == DBError::NoError);
    }

bool RDatabaseManagerSession::SetInterfaceDefault(const QServiceInterfaceDescriptor &aInterface)
    {
    QByteArray interfaceByteArray;
    QDataStream in(&interfaceByteArray, QIODevice::WriteOnly);
    in.setVersion(QDataStream::Qt_4_6);
    in << aInterface;
    TPtrC8 ptr8((TUint8 *)(interfaceByteArray.constData()), interfaceByteArray.size());
    TIpcArgs args(&ptr8, &iError);
    SendReceive(ESetInterfaceDefault2, args);

    return (iError() == DBError::NoError);
    }

DBError RDatabaseManagerSession::LastError()
    {
    DBError error;
    error.setError((DBError::ErrorCode)iError());
    return error;
    }

void RDatabaseManagerSession::SetChangeNotificationsEnabled(bool aEnabled)
    {
    TIpcArgs args((aEnabled ? 1 : 0), &iError);
    SendReceive(ESetChangeNotificationsEnabledRequest, args);
    }

void RDatabaseManagerSession::NotifyServiceSignal(TRequestStatus& aStatus)
    {
    iArgs.Set(0, &iServiceName);
    iArgs.Set(1, &iState);
    iArgs.Set(2, &iError);
    SendReceive(ENotifyServiceSignalRequest, iArgs, aStatus);
    }

void RDatabaseManagerSession::CancelNotifyServiceSignal() const
    {
    SendReceive(ECancelNotifyServiceSignalRequest, TIpcArgs(NULL));
    }

#include "moc_databasemanager_symbian_p.cpp"

QT_END_NAMESPACE

#endif //defined(__WINS__) && !defined(SYMBIAN_EMULATOR_SUPPORTS_PERPROCESS_WSD)
