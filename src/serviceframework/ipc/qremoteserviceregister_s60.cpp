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

#include "qremoteserviceregister_s60_p.h"
#include "ipcendpoint_p.h"
#include "objectendpoint_p.h"
#include <QTimer>
#include <QCoreApplication>

#include <e32base.h>

/* IPC based on Symbian Client-Server framework
 * This module implements the Symbian specific IPC mechanisms and related control.
 * IPC is based on Symbian Client-Server architecture.
 */

QT_BEGIN_NAMESPACE

#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
void printServicePackage(const QServicePackage& package)
{
  if(package.d) {
    qDebug() << "QServicePackage packageType  : " << package.d->packageType;
    qDebug() << "QServicePackage QUuid        : " << package.d->messageId;
    qDebug() << "QServicePackage responseType : " << package.d->responseType;
    qDebug() << "QServicePackage value        : " << package.d->payload;
  }
  else {
    qDebug() << "Invalid ServicePackage" << " LEAVING";
    User::Leave(KErrCorrupt);
  }
}
#endif


class SymbianClientEndPoint: public QServiceIpcEndPoint
{
    Q_OBJECT
public:
    SymbianClientEndPoint(RServiceSession* session, QObject* parent = 0)
        : QServiceIpcEndPoint(parent), session(session)
    {
        Q_ASSERT(session);
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
        qDebug() << "Symbian IPC endpoint created. 255 buffer";
#endif
        connect(session, SIGNAL(Disconnected()), this, SIGNAL(disconnected()));
    }

    ~SymbianClientEndPoint()
    {
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug() << "Symbian IPC client endpoint destroyed.";
#endif
    }

    void PackageReceived(QServicePackage package)
    {
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
        qDebug() << "SymbianClientEndPoint::PackageReceived. Enqueueing and emiting ReadyRead()";
        printServicePackage(package);
#endif
        incoming.enqueue(package);
        emit readyRead();
    }

protected:
    void flushPackage(const QServicePackage& package)
    {
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
        qDebug() << "SymbianClientEndPoint::flushPackage() for package: ";
        printServicePackage(package);
#endif
        session->SendServicePackage(package);
    }

private:
    RServiceSession *session;
};

class SymbianServerEndPoint: public QServiceIpcEndPoint
{
    Q_OBJECT
public:
    SymbianServerEndPoint(CServiceProviderServerSession* session, QObject* parent = 0)
        : QServiceIpcEndPoint(parent), session(session), obj(0)
    {
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
        qDebug() << "Symbian IPC server endpoint created.";
#endif
        Q_ASSERT(session);
        // CBase derived classes cannot inherit from QObject,
        // hence manual ownershipping instead of Qt hierarchy.
        session->SetParent(this);
    }


    ~SymbianServerEndPoint()
    {
    #ifdef QT_SFW_SYMBIAN_IPC_DEBUG
        qDebug() << "Symbian IPC server endpoint destroyed. --- emit disconnected";
    #endif

        emit disconnected();
    }

    void packageReceived(QServicePackage package)
    {
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
        qDebug() << "SymbianServerEndPoint::packageReceived, putting to queue and emiting readyread.";
        printServicePackage(package);
#endif
        incoming.enqueue(package);
        emit readyRead();
    }

    void setObjectEndPoint(ObjectEndPoint *aObj)
    {
        obj = aObj;
    }

protected:
    void flushPackage(const QServicePackage& package)
    {
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
        qDebug() << "SymbianServerEndPoint::flushPackage() for package: ";
        printServicePackage(package);
#endif
        TRAPD(err, session->SendServicePackageL(package));
        if(err != KErrNone){
            qDebug() << "flushPackage: Failed to send request: " << err;
        }
    }

private:
    CServiceProviderServerSession *session;
    ObjectEndPoint *obj;
};

QRemoteServiceRegisterSymbianPrivate::QRemoteServiceRegisterSymbianPrivate(QObject *parent)
    : QRemoteServiceRegisterPrivate(parent), m_server(0)
{
}

QRemoteServiceRegisterSymbianPrivate::~QRemoteServiceRegisterSymbianPrivate()
{
    delete m_server;
}

void QRemoteServiceRegisterSymbianPrivate::publishServices(const QString &ident)
{
    // Create service side of the Symbian Client-Server architecture.
    m_server = new CServiceProviderServer(this);
    TPtrC serviceIdent(reinterpret_cast<const TUint16*>(ident.utf16()));

    if(getSecurityFilter())
      m_server->setSecurityFilter(getSecurityFilter());

    TInt err = m_server->Start(serviceIdent);
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    if (err != KErrNone) {
        qDebug() << "publishServices: server->Start() failed: " << err;
    } else {
        qDebug("publishServices: service started successfully");
    }
#endif
    // If we're started by the client, notify them we're running
    RProcess::Rendezvous(KErrNone);
}

void QRemoteServiceRegisterSymbianPrivate::processIncoming(CServiceProviderServerSession* newSession)
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("processingIncoming session creation.");
#endif
    // Create service provider-side endpoints.
    SymbianServerEndPoint* ipcEndPoint = new SymbianServerEndPoint(newSession, this);
    ObjectEndPoint* endPoint = new ObjectEndPoint(ObjectEndPoint::Service, ipcEndPoint, this);
    ipcEndPoint->setObjectEndPoint(endPoint);
}

QRemoteServiceRegister::SecurityFilter QRemoteServiceRegisterSymbianPrivate::setSecurityFilter(QRemoteServiceRegister::SecurityFilter filter)
{
  if(m_server)
    m_server->setSecurityFilter(filter);

  return QRemoteServiceRegisterPrivate::setSecurityFilter(filter);
}


QRemoteServiceRegisterPrivate* QRemoteServiceRegisterPrivate::constructPrivateObject(QObject *parent)
{
  return new QRemoteServiceRegisterSymbianPrivate(parent);
}

QObject* QRemoteServiceRegisterPrivate::proxyForService(const QRemoteServiceRegister::Entry &entry, const QString &location)
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug() << "proxyForService for location: " << location;
#endif
    // Create client-side session for the IPC and connect it to the service
    // provide. If service provider is not up, it will be started.
    // Connecting is tried few times in a loop, because if service starting is
    // done at device startup, everything may not be ready yet.
    RServiceSession *session = new RServiceSession(location);
    int err = session->Connect();
    int i = 0;
    while (err != KErrNone) {
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
        qDebug() << "proxyForService Connecting in loop: " << i;
#endif
        if (i > 10) {
            qWarning() << "QtSFW failed to connect to service provider.";
            return NULL;
        }
        User::After(50);
        err = session->Connect();
        i++;
    }

    // Create IPC endpoint. In practice binds the communication session and abstracting
    // class presenting the IPC endoint.
    SymbianClientEndPoint* ipcEndPoint = new SymbianClientEndPoint(session);
    ipcEndPoint->setParent(session);
    // Create an active message solicitor, which listens messages from server
    ServiceMessageListener* messageListener = new ServiceMessageListener(session, ipcEndPoint);
    // Create object endpoint, which handles the metaobject protocol.
    ObjectEndPoint* endPoint = new ObjectEndPoint(ObjectEndPoint::Client, ipcEndPoint);
    endPoint->setParent(session);
    QObject *proxy = endPoint->constructProxy(entry);
    session->setParent(proxy);
    QObject::connect(session, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)),
        proxy, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)));
    return proxy;
}

RServiceSession::RServiceSession(QString address)
: iSize(0), iListener(0), iDataSizes(KIpcBufferMinimumSize),iServerStarted(EFalse)
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug() << "RServiceSession() for address: " << address;
#endif
    qt_symbian_throwIfError(iMessageFromServer.Create(KIpcBufferMinimumSize));
    iServerAddress = address;
}

RServiceSession::~RServiceSession()
{
    delete iListener;
    Close();
    iMessageFromServer.Close();
}

void RServiceSession::setListener(ServiceMessageListener *listener)
{
  iListener = listener;
}

void RServiceSession::Close()
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug() << "RServiceSession close()";
#endif
    RSessionBase::Close();
}

TInt RServiceSession::Connect()
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug() << "RServiceSession Connect()";
#endif
    TInt err=KErrUnknown;
    if(!iServerStarted){
        TInt err = StartServer();
        if (err == KErrNone) {
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
            qDebug() << "StartServer successful, Creating session.";
#endif
        iServerStarted = ETrue;
        }
    }
    TPtrC serviceAddressPtr(reinterpret_cast<const TUint16*>(iServerAddress.utf16()));
    err = CreateSession(serviceAddressPtr, Version());

    return err;
}

TVersion RServiceSession::Version() const
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug() << "RServiceSession Version()";
#endif
    return TVersion(KServerMajorVersionNumber, KServerMinorVersionNumber, KServerBuildVersionNumber);
}

void RServiceSession::SendServicePackage(const QServicePackage& aPackage)
{
    // Serialize the package into byte array, wrap it in descriptor,
    // and send to service provider.
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << aPackage;
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug() << "SendServicePackage: Size of package sent from client to server: " << block.count();
#endif
    TPtrC8 ptr8((TUint8*)(block.constData()), block.size());
    TIpcArgs args(&ptr8, &iError);
    TInt err = SendReceive(EServicePackage, args);
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug() << "SendServicePackage error code received: " << err;
#endif
    if(err != KErrNone){
      enum QService::UnrecoverableIPCError e  = QService::ErrorUnknown;
      switch(err){
      case KErrServerTerminated:
        e = QService::ErrorServiceNoLongerAvailable;
        break;
      case KErrNoMemory:
      case KErrServerBusy: // if the slots are full, something is really wrong
        e = QService::ErrorOutofMemory;
        break;
      }
      emit errorUnrecoverableIPCFault(e);
    }
}

void RServiceSession::addDataSize(TInt dataSize)
{
    iDataSizes.addSample(dataSize);
}

// StartServer() checks if the service is already published by someone (i.e. can be found
// from Kernel side). If not, it will start the process that provides the service.
TInt RServiceSession::StartServer()
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug() << "RServiceSession::StartServer()";
#endif
    TInt ret = KErrNone;
    TPtrC serviceAddressPtr(reinterpret_cast<const TUint16*>(iServerAddress.utf16()));
    TFindServer findServer(serviceAddressPtr);
    TFullName name;
    // Looks up from Kernel-side if there are active servers with the given name.
    // If not found, a process providing the service needs to be started.
    if (findServer.Next(name) != KErrNone) {
#if defined(__WINS__) && !defined(SYMBIAN_EMULATOR_SUPPORTS_PERPROCESS_WSD)
          qWarning("WINS Support for QSFW OOP not implemented.");
#else
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
        qDebug() << "RServiceSession::StartServer() Service not found. Starting " << iServerAddress;
#endif
        TRequestStatus status;
        RProcess serviceServerProcess;
        ret = serviceServerProcess.Create(serviceAddressPtr, KNullDesC);
        if (ret != KErrNone) {
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
            qDebug() << "StartServer RProcess::Create failed: " << ret;
#endif
            return ret;
        }
        // Point of synchronization. Waits until the started process calls
        // counterpart of this function (quits wait also if process dies / panics).
        serviceServerProcess.Rendezvous(status);

        if (status != KRequestPending) {
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
            qDebug() << "StartServer Service Server Process Rendezvous() failed, killing process.";
#endif
            serviceServerProcess.Kill(KErrNone);
            serviceServerProcess.Close();
            return KErrGeneral;
        } else {
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
            qDebug() << "StartServer Service Server Process Rendezvous() successful, resuming process.";
#endif
            serviceServerProcess.Resume();
        }
        User::WaitForRequest(status);
        if (status != KErrNone){
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
            qDebug("RTR Process Resume failed.");
#endif
            serviceServerProcess.Close();
            return status.Int();
        }
        // Free the handle to the process (RHandleBase function, does not 'close process')
        serviceServerProcess.Close();
#endif // __WINS__
    } else {
        qDebug() << "RServiceSession::StartServer() GTR Service found from Kernel, no need to start process.";
    }
    return ret;
}

/* Since the average size will not change in the middle of a fragmented message
 * trasnfer actual updates to buffer size will happen only when a new message
 * arrives */
void RServiceSession::updateIpcBufferSize()
{
   TInt weightedAvg = iDataSizes.averageWeighted();

#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
   qDebug() << "updateIpcBufferSize(): current weighted average of data size is: "<<weightedAvg;
   qDebug() << "Current IPC Buffer size is: "<<iMessageFromServer.MaxLength();
#endif
   TInt newSize = iMessageFromServer.MaxLength();
   if(weightedAvg > iMessageFromServer.MaxLength()){
       newSize = iMessageFromServer.MaxLength()*2;
   }
   else if(weightedAvg < iMessageFromServer.MaxLength()/2){

         newSize = iMessageFromServer.MaxLength()/2;
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
   qDebug() << "updateIpcBufferSize(): current weighted average of data size is:(2) "<<weightedAvg<<" max/2";
#endif
   }

   if(newSize < KIpcBufferMinimumSize)
       newSize = KIpcBufferMinimumSize;
   else if(newSize > KIpcBufferMaximumSize)
       newSize = KIpcBufferMaximumSize;


   if(newSize != iMessageFromServer.MaxLength()) {

        iMessageFromServer.SetLength(0);
        //If realloc fails the old descriptor is left unchanged.
        iMessageFromServer.ReAlloc(newSize);
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
       qDebug() << "updateIpcBufferSize():  New Size of IPC Buffer: "<<newSize;
#endif
   }
}

void RServiceSession::ListenForPackages(TRequestStatus& aStatus)
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug() << "GTR RServiceSession::ListenForPackages(), iSize: " << iSize();
#endif
    updateIpcBufferSize();
    iArgs.Set(0, &iMessageFromServer);
    // Total Size of returned messaage,which might differ from the amount of data in iMessageFromServer
    iArgs.Set(1, &iSize);
    iArgs.Set(2, &iError);

    SendReceive(EPackageRequest, iArgs, aStatus);
}

void RServiceSession::CancelListenForPackages()
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("RServiceSession::CancelListenForPackages -- 2");
#endif
    TInt err = SendReceive(EPackageRequestCancel, TIpcArgs(NULL));
    if(err != KErrNone){
      enum QService::UnrecoverableIPCError e = QService::ErrorUnknown;
      switch(err){
      case KErrServerTerminated:
        e = QService::ErrorServiceNoLongerAvailable;
        break;
      case KErrNoMemory:
      case KErrServerBusy: // if the slots are full, something is really wrong
        e = QService::ErrorOutofMemory;
        break;
      }
      emit errorUnrecoverableIPCFault(e);
    }
}

void RServiceSession::ipcFailure(QService::UnrecoverableIPCError err)
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("RServiceSession::ipcFailure ipc Fault reported");
#endif
  emit errorUnrecoverableIPCFault(err);
}

static const TUint myRangeCount = 1;
static const TInt myRanges[myRangeCount] =
    {
    0 //range is 0-Max inclusive
    };
static const TUint8 myElementsIndex[myRangeCount] =
    {
    CPolicyServer::EAlwaysPass
    };
static const CPolicyServer::TPolicyElement myElements[] =
    {
        {_INIT_SECURITY_POLICY_PASS } // Dummy entry
    };
static const CPolicyServer::TPolicy myPolicy =
    {
    CPolicyServer::ECustomCheck, //specifies all connect attempts should pass
    myRangeCount,
    myRanges,
    myElementsIndex,
    myElements,
    };

CServiceProviderServer::CServiceProviderServer(QRemoteServiceRegisterSymbianPrivate* aOwner)
    : CPolicyServer(EPriorityNormal, myPolicy), iSessionCount(0), iOwner(aOwner), iFilter(0)
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("CServiceProviderServer constructor");
#endif
    Q_ASSERT(aOwner);
}

CPolicyServer::TCustomResult CServiceProviderServer::CustomSecurityCheckL(const RMessage2 &aMessage, TInt &aAction, TSecurityInfo &aMissing)
{
    if(iFilter){
        if(iFilter(reinterpret_cast<const void *>(&aMessage))){
            return CPolicyServer::EPass;
        }
        else {
            return CPolicyServer::EFail;
        }
    }
    return CPolicyServer::EPass;
}

CSession2* CServiceProviderServer::NewSessionL(const TVersion &aVersion, const RMessage2 &aMessage) const
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("CServiceProviderServer::NewSessionL()");
#endif
    if (!User::QueryVersionSupported(TVersion(KServerMajorVersionNumber,
                                              KServerMinorVersionNumber, KServerBuildVersionNumber), aVersion))
    {
        User::Leave(KErrNotSupported);
    }
    CServiceProviderServerSession* session = CServiceProviderServerSession::NewL(*const_cast<CServiceProviderServer*>(this));
    iOwner->processIncoming(session);
    return session;
}

void CServiceProviderServer::IncreaseSessions()
{
    iSessionCount++;
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug() << ">>>> CServiceProviderServer incremented session count to: " << iSessionCount;
#endif
}

void CServiceProviderServer::DecreaseSessions()
{
    iSessionCount--;
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug() << "<<<< CServiceProviderServer decremented session count to: " << iSessionCount;
#endif
    if(iSessionCount == 0){
        Cancel();
        if(iOwner->quitOnLastInstanceClosed())
          QCoreApplication::exit();
    }
}

void CServiceProviderServer::setSecurityFilter(QRemoteServiceRegister::SecurityFilter filter)
{
  iFilter = filter;
}

CServiceProviderServerSession *CServiceProviderServerSession::NewL(CServiceProviderServer &aServer)
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("CServiceProviderServerSession::NewL()");
#endif
    CServiceProviderServerSession *self = CServiceProviderServerSession::NewLC(aServer);
    CleanupStack::Pop(self);
    return self;
}

CServiceProviderServerSession *CServiceProviderServerSession::NewLC(CServiceProviderServer &aServer)
{
    CServiceProviderServerSession* self = new (ELeave) CServiceProviderServerSession(aServer);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}

CServiceProviderServerSession::CServiceProviderServerSession(CServiceProviderServer &aServer)
    : iServer(aServer)
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("CServiceProviderServerSession constructor");
#endif
    iServer.IncreaseSessions();
}

void CServiceProviderServerSession::ConstructL()
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("OTR CServiceProviderServerSession::ConstructL()");
#endif
    iTotalSize = 0;    // No data
    iBlockData.clear();// clear the buffer
}

CServiceProviderServerSession::~CServiceProviderServerSession()
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("CServiceProviderServerSession destructor");
#endif
    iServer.DecreaseSessions();
    delete iByteArray;
    delete iOwner;
}

void CServiceProviderServerSession::ServiceL(const RMessage2 &aMessage)
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug() << "CServiceProviderServerSession::ServiceL for message: " << aMessage.Function();
#endif
    switch (aMessage.Function())
    {
    case EServicePackage:
        HandleServicePackageL(aMessage);
        aMessage.Complete(KErrNone);
        break;
    case EPackageRequest:
        HandlePackageRequestL(aMessage);
        break;
    case EPackageRequestCancel:
        HandlePackageRequestCancelL(aMessage);
        break;
    }
}

void CServiceProviderServerSession::HandleServicePackageL(const RMessage2& aMessage)
{
    // Reproduce the serialized data.
    HBufC8* servicePackageBuf8 = HBufC8::New(aMessage.GetDesLength(0));
    if (!servicePackageBuf8) {
        User::Leave( KErrNoMemory );
    }

    TPtr8 ptrToBuf(servicePackageBuf8->Des());
    TInt ret = KErrNone;
    TRAP(ret, aMessage.ReadL(0, ptrToBuf));
    if (ret != KErrNone) {
        // TODO: is this error handleing correct
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
        qDebug() << "HandleServicePackageL. Message read failed: " << ret;
#endif
        //iDb->lastError().setError(DBError::UnknownError);
        //aMessage.Write(1, LastErrorCode());
        delete servicePackageBuf8;
        User::Leave( ret );
    }

    QByteArray byteArray((const char*)ptrToBuf.Ptr(), ptrToBuf.Length());
    QDataStream in(byteArray);
    QServicePackage results;
    in >> results;

#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug() << "CServiceProviderServerSession Reproduced service package: ";
    printServicePackage(results);
#endif
    iOwner->packageReceived(results);
    delete servicePackageBuf8;
}

void CServiceProviderServerSession::SetParent(SymbianServerEndPoint *aOwner)
{
    iOwner = aOwner;
}

void CServiceProviderServerSession::HandlePackageRequestL(const RMessage2& aMessage)
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("HandlePackageRequestL(). Setting pending true and storing message.");
#endif
    iMsg = aMessage;
    iPendingPackageRequest = ETrue;
    if(!iPendingPackageQueue.isEmpty())
      SendServicePackageL(iPendingPackageQueue.dequeue());
}

void CServiceProviderServerSession::HandlePackageRequestCancelL(const RMessage2& aMessage)
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("HandlePackageRequestCancelL");
#endif
    if (iPendingPackageRequest) {
        iMsg.Complete(KErrCancel);
        iPendingPackageRequest = EFalse;
    }
    aMessage.Complete(EPackageRequestComplete);
}

void CServiceProviderServerSession::SendServicePackageL(const QServicePackage& aPackage)
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("CServiceProviderServerSession:: SendServicePackage for package: ");
    printServicePackage(aPackage);
#endif
    if (iPendingPackageRequest) {
        if(iBlockData.isEmpty()){
          // Serialize the package
          QDataStream out(&iBlockData, QIODevice::WriteOnly);
          out.setVersion(QDataStream::Qt_4_6);
          out << aPackage;
          iTotalSize = iBlockData.size();
        }
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
        qDebug() << "Size of package sent from server to client: " << iBlockData.count();
        qDebug() << "Size of buffer from client: " << iMsg.GetDesMaxLength(0);
#endif

        int size = iBlockData.size();
        if(size > iMsg.GetDesMaxLength(0)){
          size = iMsg.GetDesMaxLength(0);
          // enequeue the package so we send the  next chunk
          // when the next request comes through
          iPendingPackageQueue.prepend(aPackage);
        }
        TPtrC8 ptr8((TUint8*)(iBlockData.constData()), size);
        iMsg.WriteL(0, ptr8);
        iBlockData.remove(0, size);
//#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
//        if(status == KErrOverflow){
//          qDebug() << "OTR Server to client, got overflow, sending 0 bytes";
//        }
//        else if(status != KErrNone){
//          qDebug() << "OTR SendServicePackage: error code from send: " << status;
//        }
//#endif
        TPckgBuf<TInt> totalSize(iTotalSize);
        iMsg.WriteL(1,totalSize);
        iMsg.Complete(EPackageRequestComplete);
        iPendingPackageRequest = EFalse;
    } else {
        iPendingPackageQueue.enqueue(aPackage);
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
        qWarning() << "RTR SendServicePackage: service package from server to client queued - no pending receive request.";
#endif
    }
}

ServiceMessageListener::ServiceMessageListener(RServiceSession* aSession, SymbianClientEndPoint* aOwner)
    : CActive(EPriorityNormal),
    iClientSession(aSession),
    iOwnerEndPoint(aOwner)
{
    Q_ASSERT(iClientSession);
    Q_ASSERT(iOwnerEndPoint);
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("ServiceMessageListener constructor");
#endif
    aSession->setListener(this);
    CActiveScheduler::Add(this);
    StartListening();
}

ServiceMessageListener::~ServiceMessageListener()
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("ServiceMessageListener destructor");
#endif
    Cancel();
}

void ServiceMessageListener::StartListening()
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("ServiceMessageListener::StartListening");
#endif
    iClientSession->ListenForPackages(iStatus);
    SetActive();
}

void ServiceMessageListener::DoCancel()
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug("ServiceMessageListener::DoCancel");
#endif
    iClientSession->CancelListenForPackages();
}

void ServiceMessageListener::RunL()
{
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
    qDebug() << "ServiceMessageListener::RunL for iStatus.Int(should be 4): " << iStatus.Int();
#endif
    if (iStatus.Int() == EPackageRequestComplete) {
        // Client side has received a service package from server. Pass it onwards and
        // issue new pending request.
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
        qDebug() << "RunL length of the package received client side is: " << iClientSession->iMessageFromServer.Length();
#endif
        if(iClientSession->iMessageFromServer.Length() == 0){
          // we received 0 bytes from the other side,
          // normally because it tried to write more bytes
          // than were in the TDes
          User::Leave(KErrTooBig);
        }

        if(!iByteArray.length()){
            /* Helps client session to calculate an optimum IPC buffer size */
            iClientSession->addDataSize(iClientSession->iSize());
        }

        iByteArray.append((const char*)iClientSession->iMessageFromServer.Ptr(),
                             iClientSession->iMessageFromServer.Length());
        if(iByteArray.length() >= iClientSession->iSize()){
          QDataStream in(iByteArray);
          iByteArray.clear();
          QServicePackage results;
          in >> results;
#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
          qDebug() << "ServiceMessageListener Reproduced service package: ";
          printServicePackage(results);
#endif
          iOwnerEndPoint->PackageReceived(results);
        }
        StartListening();
    }
    else if(iStatus.Int() < 0){
      TInt s = iStatus.Int();
      switch(s){
      case KErrServerTerminated:
        iClientSession->ipcFailure(QService::ErrorServiceNoLongerAvailable);
        break;
      case KErrServerBusy:
      case KErrNoMemory:
        iClientSession->ipcFailure(QService::ErrorOutofMemory);
        break;
      }
    }
}

#include "moc_qremoteserviceregister_s60_p.cpp"
#include "qremoteserviceregister_s60.moc"
QT_END_NAMESPACE
