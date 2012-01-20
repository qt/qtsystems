/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QREMOTESERVICEREGISTER_S60_P_H
#define QREMOTESERVICEREGISTER_S60_P_H

//#define QT_SFW_SYMBIAN_IPC_DEBUG

#include "qremoteserviceregister.h"
#include "qremoteserviceregister_p.h"
//#include "qremoteserviceclassregister.h"
#include "qservicepackage_p.h"
#include "qservice.h"
#include <e32base.h>

#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
#include <QDebug>
#endif

#include <QQueue>

QT_BEGIN_NAMESPACE

class ServiceMessageListener;

const TUint KServerMajorVersionNumber = 1;
const TUint KServerMinorVersionNumber = 0;
const TUint KServerBuildVersionNumber = 0;

enum TServiceProviderRequest
{
    EServicePackage = 1,
    EPackageRequest = 2,
    EPackageRequestCancel = 3,
    EPackageRequestComplete = 4
};

const TInt KIpcBufferMinimumSize = 256; //Bytes
const TInt KIpcBufferMaximumSize = 1024*16;

// Forward declarations
class ObjectEndPoint;
class CServiceProviderServerSession;
class CServiceProviderServer;
class SymbianServerEndPoint;
class SymbianClientEndPoint;
class QRemoteServiceRegisterSymbianPrivate;

// Type definitions
typedef TPckgBuf<TInt> TError;

#ifdef QT_SFW_SYMBIAN_IPC_DEBUG
void printServicePackage(const QServicePackage& package);
#endif

//Internal class which calculates the Simple and Weighted moving average of
//data sizes from the Service. The average calculated can be used to decide a
//optimal IPC buffer size.
template <int WINDOW>
class MovingAverage
{
public:
  MovingAverage(TInt median):m_index(0),m_averageSimple((TReal)median),m_averageWeighted((TReal)median)
  {
    m_weightedSum = m_sum =0;
    for (TInt i=0;i<WINDOW;i++){
     m_samples[i]=median;
     m_weightedSum += m_samples[i]*(i+1);
     m_sum += m_samples[i];
     }
  }
  int average(){return static_cast<TInt>(m_averageSimple);}
  int averageWeighted(){return static_cast<TInt>(m_averageWeighted);}
  void addSample(int sample)
  {
     int pop = m_samples[m_index];
     m_samples[m_index] = sample;
     m_weightedSum = m_weightedSum + WINDOW*sample - m_sum;
     m_sum = m_sum + sample - pop;
     m_averageSimple = m_averageSimple + ((TReal)sample -pop)/WINDOW;
     m_averageWeighted = (TReal)m_weightedSum*2/(WINDOW*(WINDOW+1));
     m_index = (m_index+1)%WINDOW;
  }
private:
  int m_samples[WINDOW];
  TInt m_index;
  TInt m_weightedSum;
  /* The sum of all the samples */
  TInt m_sum;
  TReal m_averageSimple;
  TReal m_averageWeighted;
};


// Internal class handling the actual communication with the service provider.
// Communication is based on standard Symbian client-server architecture.
class RServiceSession : public QObject, public RSessionBase
{
    Q_OBJECT
public:
    RServiceSession(QString address);
    virtual ~RServiceSession();
    TInt Connect();
    void Close();
    TVersion Version() const;
    void SendServicePackage(const QServicePackage& aPackage);
    /* Adds sample to calculate the average */
    void addDataSize(TInt dataSize);

 public:
    RBuf8 iMessageFromServer;
    TPckgBuf<TInt> iSize; // TPckgBuf type can be used directly as IPC parameter

    void setListener(ServiceMessageListener* listener);

public slots:
     void ipcFailure(QService::UnrecoverableIPCError);

signals:
    void Disconnected();
    void errorUnrecoverableIPCFault(QService::UnrecoverableIPCError);

protected:
    void ListenForPackages(TRequestStatus& aStatus);
    void CancelListenForPackages();

private:
    TInt StartServer();
    /* Re-Sizes IPC buffer if needed */
    void updateIpcBufferSize();

private:
    TIpcArgs iArgs; // These two are used in actively listening to server
    TError iError;
    QString iServerAddress;
    ServiceMessageListener* iListener;
    MovingAverage<10> iDataSizes;
    TBool iServerStarted;
    friend class ServiceMessageListener;
};

// needed for creating server thread.
const TUint KDefaultHeapSize = 0x10000;

class CServiceProviderServer : public CPolicyServer
    {
    public:
        CServiceProviderServer(QRemoteServiceRegisterSymbianPrivate* aOwner);
        CSession2* NewSessionL(const TVersion& aVersion, const RMessage2& aMessage) const;

    public:

        void IncreaseSessions();
        void DecreaseSessions();

        void setSecurityFilter(QRemoteServiceRegister::SecurityFilter filter);

    protected:
        virtual TCustomResult CustomSecurityCheckL(const RMessage2 &,TInt &,TSecurityInfo &);

    private:

        int iSessionCount;
        QRemoteServiceRegisterSymbianPrivate* iOwner;
        QRemoteServiceRegister::SecurityFilter iFilter;
    };

class CServiceProviderServerSession : public CSession2
    {
    public:
        static CServiceProviderServerSession* NewL(CServiceProviderServer& aServer);
        static CServiceProviderServerSession* NewLC(CServiceProviderServer& aServer);
        virtual ~CServiceProviderServerSession();
        void ServiceL(const RMessage2& aMessage);
        void SetParent(SymbianServerEndPoint* aOwner);
        void SendServicePackageL(const QServicePackage& aPackage);

        void HandleServicePackageL(const RMessage2& aMessage);
        void HandlePackageRequestL(const RMessage2& aMessage);
        void HandlePackageRequestCancelL(const RMessage2& aMessage);

    private:
        CServiceProviderServerSession(CServiceProviderServer& aServer);
        void ConstructL();

    private:
        CServiceProviderServer& iServer;
        SymbianServerEndPoint* iOwner;
        QByteArray* iByteArray;
        RMessage2 iMsg; // For replying pending service package requests
        QQueue<QServicePackage> iPendingPackageQueue;
        TBool iPendingPackageRequest;
        QByteArray iBlockData;
        int iTotalSize;
    };


class QRemoteServiceRegisterSymbianPrivate: public QRemoteServiceRegisterPrivate
{
    Q_OBJECT

public:
    QRemoteServiceRegisterSymbianPrivate(QObject* parent);
    ~QRemoteServiceRegisterSymbianPrivate();
    void publishServices(const QString& ident );
    static QObject* proxyForService(const QRemoteServiceRegister::Entry& entry, const QString& location);
    void processIncoming(CServiceProviderServerSession* session);

    virtual QRemoteServiceRegister::SecurityFilter setSecurityFilter(QRemoteServiceRegister::SecurityFilter filter);

private:
    CServiceProviderServer *m_server;
};

// A helper class that actively listens for serviceprovider messages.
// Needed because Symbian server cannot send messages without active request
// from the client.
class ServiceMessageListener : public CActive
{
public:
    ServiceMessageListener(RServiceSession* aSession, SymbianClientEndPoint* aOwner);
    ~ServiceMessageListener();

protected:
    void StartListening();
    // from CActive baseclass
    void DoCancel();
    void RunL();

private:
    RServiceSession* iClientSession;
    SymbianClientEndPoint* iOwnerEndPoint;
    QByteArray iByteArray;
};

QT_END_NAMESPACE

#endif // QREMOTESERVICEREGISTER_S60_P_H
