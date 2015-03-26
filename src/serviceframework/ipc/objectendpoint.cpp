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

#include "objectendpoint_p.h"
#include "instancemanager_p.h"
#include <private/qmetaobjectbuilder_p.h>
#include "proxyobject_p.h"
#include "qsignalintercepter_p.h"
#include "qserviceclientcredentials.h"
#include "qserviceclientcredentials_p.h"
#include <QTimer>
#include <QEvent>
#include <QVarLengthArray>
#include <QTime>
#include <QCoreApplication>
#include <QFile>
#include <QStringList>

#include "qservicedebuglog_p.h"

#ifdef Q_OS_LINUX
#include <execinfo.h>
#endif

QT_BEGIN_NAMESPACE

class Response
{
public:
    Response() : isFinished(false), result(0)
    { }

    ~Response()
    { }

    bool isFinished;
    void* result;
    QString error;
};

class ServiceSignalIntercepter : public QSignalIntercepter
{
    //Do not put Q_OBJECT here
public:
    ServiceSignalIntercepter(QObject* sender, const QByteArray& signal,
            ObjectEndPoint* parent)
        : QSignalIntercepter(sender, signal, parent), endPoint(parent)
    {
    this->signal = signal;
    }

    void setMetaIndex(int index)
    {
        metaIndex = index;
    }

protected:
    void activated( const QList<QVariant>& args )
    {
        qServiceLog() << "class" << "servicesignalinter"
                      << "event" << "signal"
                      << "endpoint" << endPoint->objectName()
                      << "metaidx" << metaIndex
                      << "signal" << QString::fromLatin1(signal)
                      << "args" << args.count();

        endPoint->invokeRemote(metaIndex, args, QMetaType::Void);
    }
private:
    ObjectEndPoint* endPoint;
    int metaIndex;
    QByteArray signal;

};

class ObjectEndPointPrivate
{
public:
    //used on client and service side
    ObjectEndPoint::Type endPointType;
    ObjectEndPoint* parent;

    //used on service side
    QRemoteServiceRegister::Entry entry;
    QUuid serviceInstanceId;

    // user on the client side
    bool functionReturned;
    QUuid waitingOnReturnUuid;

    ObjectEndPointPrivate() :
        endPointType(ObjectEndPoint::Service),
        parent(0),
        functionReturned(false)
    {
        waitingOnReturnUuid = QUuid();
    }

    ~ObjectEndPointPrivate()
    {
    }

    //service side
    void setupSignalIntercepters(QObject * service)
    {
        Q_ASSERT(endPointType == ObjectEndPoint::Service);

        //create a signal intercepter for each signal
        //offered by service
        //exclude QObject signals
        const QMetaObject* mo = service->metaObject();
        while (mo && strcmp(mo->className(), "QObject"))
        {
            for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
                const QMetaMethod method = mo->method(i);
                if (method.methodType() == QMetaMethod::Signal) {
                    QByteArray signal = method.methodSignature();
                    //add '2' for signal - see QSIGNAL_CODE
                    ServiceSignalIntercepter* intercept =
                        new ServiceSignalIntercepter(service, "2"+signal, parent );
                    intercept->setMetaIndex(i);
                }
            }
            mo = mo->superClass();
        }
    }

    /*!
        Activate slots connected to given signal. Unfortunately we can only do this
        using the signal index relative to the meta object defining the signal.
    */
    int triggerConnectedSlots(QObject* service, const QMetaObject* meta, int id, void **args)
    {
        Q_ASSERT(endPointType == ObjectEndPoint::Client);

        const QMetaObject* parentMeta = meta->superClass();
        if (parentMeta)
            id = triggerConnectedSlots(service, parentMeta, id, args);

        if (id < 0)
            return id;

        const int methodsThisType = meta->methodCount() - meta->methodOffset();
        if (id >= 0 && id < methodsThisType)
            QMetaObject::activate(service, meta, id, args);

        id -= methodsThisType;
        return id;
    }
};

ObjectEndPoint::ObjectEndPoint(Type type, QServiceIpcEndPoint* comm, QObject* parent)
    : QObject(parent), dispatch(comm), service(0), localToRemote(0),
      remoteToLocal(0)
{
    Q_ASSERT(dispatch);
    d = new ObjectEndPointPrivate;
    d->parent = this;
    d->endPointType = type;

    dispatch->setParent(this);
#ifdef SFW_USE_UNIX_BACKEND
    connect(dispatch, SIGNAL(readyRead()), this, SLOT(newPackageReady()), Qt::DirectConnection);
    connect(dispatch, SIGNAL(disconnected()), this, SLOT(disconnected()), Qt::DirectConnection);
#else
    connect(dispatch, SIGNAL(readyRead()), this, SLOT(newPackageReady()), Qt::QueuedConnection);
    connect(dispatch, SIGNAL(disconnected()), this, SLOT(disconnected()), Qt::QueuedConnection);
#endif
    if (type == Client) {
        return; //we are waiting for conctructProxy() call
    } else {
        if (dispatch->packageAvailable())
            QTimer::singleShot(0, this, SLOT(newPackageReady()));
    }
}

ObjectEndPoint::~ObjectEndPoint()
{
    qServiceLog() << "class" << "objectendpoint"
                  << "event" << "delete"
                  << "name" << objectName();
    delete d;
}

void ObjectEndPoint::disconnected()
{
    qServiceLog() << "class" << "objectendpoint"
                  << "event" << "disconnected"
                  << "name" << objectName();
    if (d->endPointType == Service) {
        InstanceManager::instance()->removeObjectInstance(d->entry, d->serviceInstanceId);
        deleteLater();
    }
    foreach (Response *r, openRequests) {
        r->error = QLatin1Literal("end point disconnected");
        r->isFinished = true;
        dispatch->waitingDone();
    }
}

/*
    Client requests proxy object. The proxy is owned by calling
    code and this object must clean itself up upon destruction of
    proxy.
*/
QObject* ObjectEndPoint::constructProxy(const QRemoteServiceRegister::Entry & entry)
{
    //client side
    Q_ASSERT(d->endPointType == ObjectEndPoint::Client);

    //ask for serialized meta object
    //get proxy based on meta object
    //return meta object
    QServicePackage p;
    p.d = new QServicePackagePrivate();
    p.d->messageId = QUuid::createUuid();
    p.d->entry = entry;

    Response* response = new Response();
    openRequests.insert(p.d->messageId, response);

    dispatch->writePackage(p);
    qServiceLog() << "class" << "objectendpoint"
                  << "event" << "constructProxy"
                  << "progress" << "wait for result";
    waitForResponse(p.d->messageId);
    qServiceLog() << "class" << "objectendpoint"
                  << "event" << "constructProxy"
                  << "progress" << "got result";


    if (response->isFinished) {
        if (response->result == 0)
            qWarning() << "Request for remote service failed";
        else
            service = reinterpret_cast<QServiceProxy* >(response->result);
    } else {
        qDebug() << Q_FUNC_INFO << "response passed but not finished";
        return 0;
    }

    openRequests.take(p.d->messageId);
    delete response;

    return service;
}

void ObjectEndPoint::newPackageReady()
{
    //client and service side

    while (dispatch->packageAvailable() && !d->functionReturned)
    {
        QServicePackage p = dispatch->nextPackage();
        if (!p.isValid())
            continue;

        switch (p.d->packageType) {
            case QServicePackage::ObjectCreation:
                objectRequest(p);
                break;
            case QServicePackage::MethodCall:
                methodCall(p);
                break;
            case QServicePackage::PropertyCall:
                propertyCall(p);
                break;
            default:
                qWarning() << "Unknown package type received.";
        }
    }
}

void ObjectEndPoint::propertyCall(const QServicePackage& p)
{
    if (p.d->responseType == QServicePackage::NotAResponse) {
        //service side
        Q_ASSERT(d->endPointType == ObjectEndPoint::Service);

        if (!service) { // ingore everything until the service is created
            qWarning() << Q_FUNC_INFO << "dropping a property call since the object hasn't completed construction";
            return;
        }

        QByteArray data = p.d->payload.toByteArray();
        QDataStream stream(&data, QIODevice::ReadOnly);
        int metaIndex = -1;
        QVariant arg;
        int callType;
        stream >> metaIndex;
        stream >> arg;
        stream >> callType;
        const QMetaObject::Call c = (QMetaObject::Call) callType;

        QVariant result;
        QMetaProperty property = service->metaObject()->property(metaIndex);
        if (property.isValid()) {
            switch (c) {
                case QMetaObject::ReadProperty:
                    result = property.read(service);
                    break;
                case QMetaObject::WriteProperty:
                    property.write(service, arg);
                    break;
                case QMetaObject::ResetProperty:
                    property.reset(service);
                    break;
                default:
                    break;

            }
        }

        if (c == QMetaObject::ReadProperty) {
            QServicePackage response = p.createResponse();
            if (property.isValid()) {
                response.d->responseType = QServicePackage::Success;
                response.d->payload = result;
            } else {
                response.d->responseType = QServicePackage::Failed;
            }
            dispatch->writePackage(response);
        }
    } else {
        //client side
        Q_ASSERT(d->endPointType == ObjectEndPoint::Client);
        static QQueue<QUuid> lastId;
        Response* response = openRequests.value(p.d->messageId);
        if (response) {
            lastId.enqueue(p.d->messageId);
            while (lastId.count() > 10)
                lastId.dequeue();
            response->isFinished = true;
            if (p.d->responseType == QServicePackage::Failed) {
                response->result = 0;
                response->error = QLatin1Literal("QServicePackaged::Failed");
                response->isFinished = true;
                dispatch->waitingDone();
                qWarning() << "Service method call failed";
                return;
            }
            QVariant* variant = new QVariant(p.d->payload);
            response->result = reinterpret_cast<void *>(variant);

            dispatch->waitingDone();
        }
        else {
            qWarning() << "**** FAILED TO FIND MESSAGE ID!!! ****";
            qWarning() << "Current id" << p.d->messageId.toString();
            foreach (const QUuid &last, lastId)
                qWarning() << "last ids" << last.toString();
            qWarning() << p;
        }
    }
}

void ObjectEndPoint::objectRequest(const QServicePackage& p)
{
    if (p.d->responseType != QServicePackage::NotAResponse ) {
        //client side
        Q_ASSERT(d->endPointType == ObjectEndPoint::Client);

        Response* response = openRequests.value(p.d->messageId);
        if (p.d->responseType == QServicePackage::Failed) {
            response->result = 0;
            response->error = QLatin1Literal("objectRequest QServicePackage::Failed");
            response->isFinished = true;
            dispatch->waitingDone();
            qWarning() << "Service instantiation failed";
            return;
        }
        //deserialize meta object and
        //create proxy object
        QServiceProxy* proxy = new QServiceProxy(p.d->payload.toByteArray(), this);
        response->result = reinterpret_cast<void *>(proxy);
        response->isFinished = true;

        //wake up waiting code
        dispatch->waitingDone();

    } else {
        //service side
        Q_ASSERT(d->endPointType == ObjectEndPoint::Service);

        QServicePackage response = p.createResponse();
        InstanceManager* m = InstanceManager::instance();

        //get meta object from type register
        const QMetaObject* meta = m->metaObject(p.d->entry);
        if (!meta) {
            qDebug() << "Unknown type" << p.d->entry;
            dispatch->writePackage(response);
            return;
        }

        setObjectName(p.d->entry.interfaceName() + QLatin1Char(' ') + dispatch->objectName());
        dispatch->setObjectName(objectName());

        //serialize meta object
        QByteArray data;
        QDataStream stream( &data, QIODevice::WriteOnly | QIODevice::Append );
        QMetaObjectBuilder builder(meta);
        builder.serialize(stream);

        QServiceClientCredentials creds;
        dispatch->getSecurityCredentials(creds);

        // not supported on windows at the moment
        if (!creds.isValid()) {
            qWarning() << "SFW Unable to get socket credentials client asking for" << p.d->entry.interfaceName() << p.d->entry.serviceName() << "this may fail in the future";
            disconnected();
            return;
        }

        //instantiate service object from type register
        service = m->createObjectInstance(p.d->entry, d->serviceInstanceId, creds);
        if (!service) {
            qWarning() << "Cannot instanciate service object";
            dispatch->writePackage(response);
            return;
        }

        if (!creds.isClientAccepted()) {
            qWarning() << "SFW Security failure by" <<
                          creds.getProcessIdentifier() <<
                          creds.getUserIdentifier() <<
                          creds.getGroupIdentifier() <<
                          "requesting" << p.d->entry.interfaceName() << p.d->entry.serviceName();
            m->removeObjectInstance(p.d->entry, d->serviceInstanceId);
            disconnected();
            dispatch->terminateConnection();
            return;
        }

        d->setupSignalIntercepters(service);

        //send meta object
        d->entry = p.d->entry;
        response.d->entry = p.d->entry;
        response.d->responseType = QServicePackage::Success;
        response.d->payload = QVariant(data);
        dispatch->writePackage(response);
    }
}

void ObjectEndPoint::methodCall(const QServicePackage& p)
{
    if (!service) // ingore everything until the service is created
    {
        qWarning() << Q_FUNC_INFO << "dropping a method call or signal since the object hasn't completed construction";
        return;
    }

    if (p.d->responseType == QServicePackage::NotAResponse ) {
        //service side if slot invocation
        //client side if signal emission (isSignal==true)

        QByteArray data = p.d->payload.toByteArray();
        QDataStream stream(&data, QIODevice::ReadOnly);
        int metaIndex = -1;
        QVariantList args;
        stream >> metaIndex;
        stream >> args;

        if (remoteToLocal)
            metaIndex = remoteToLocal[metaIndex];

        QMetaMethod method = service->metaObject()->method(metaIndex);
        const bool isSignal = (method.methodType() == QMetaMethod::Signal);
        const int returnType = method.returnType();

        if (isSignal) {
            Q_ASSERT(d->endPointType == ObjectEndPoint::Client);
            // Construct the raw argument list.
            /*  we ignore a possible return type of the signal. The value is
                not deterministic and it can actually create memory leaks
                in moc generated code.
            */
            const int numArgs = args.size();
            QVarLengthArray<void *, 32> a( numArgs+1 );
            a[0] = 0;

            const QList<QByteArray> pTypes = method.parameterTypes();
            for ( int arg = 0; arg < numArgs; ++arg ) {
                if (pTypes.at(arg) == "QVariant")
                    a[arg+1] = (void *)&( args[arg] );
                else
                    a[arg+1] = (void *)( args[arg].data() );
            }

            d->triggerConnectedSlots(service, service->metaObject(), metaIndex, a.data());
            return;
        }
        //service side
        if (d->endPointType != ObjectEndPoint::Service) {

            qWarning() << "SFW FATAL ERROR. Client got a method call that wasn't a signal" << objectName();

            qServiceLog() << "error" << "non-signal method call on client"
                          << "fatal" << 1;

#ifdef Q_OS_LINUX
            void *symbols[128];
            char **it;
            size_t symbol_count;
            size_t i;
            symbol_count = backtrace(symbols, 128);
            it = backtrace_symbols(symbols, symbol_count);
            for (i = 0; i < symbol_count; ++i)
              printf("TRACE:\t%s\n", it[i]);
            printf("\n");
#endif
            return;
        }

        const char* typenames[] = {0,0,0,0,0,0,0,0,0,0};
        const void* param[] = {0,0,0,0,0,0,0,0,0,0};

        for (int i=0; i<args.size(); i++) {
            if (args[i].isValid()) {
                typenames[i] = args[i].typeName();
            } else {
                if (method.parameterTypes().at(i) == "QVariant")
                    typenames[i] = "QVariant";
            }
            param[i] = args[i].constData();
        }


        bool result = false;
        if (returnType == QMetaType::Void) {
            result = method.invoke(service,
                   QGenericArgument(typenames[0], param[0]),
                   QGenericArgument(typenames[1], param[1]),
                   QGenericArgument(typenames[2], param[2]),
                   QGenericArgument(typenames[3], param[3]),
                   QGenericArgument(typenames[4], param[4]),
                   QGenericArgument(typenames[5], param[5]),
                   QGenericArgument(typenames[6], param[6]),
                   QGenericArgument(typenames[7], param[7]),
                   QGenericArgument(typenames[8], param[8]),
                   QGenericArgument(typenames[9], param[9]));
        } else {
            //result buffer
            QVariant returnValue;
            //ignore whether QVariant is a declared meta type or not
            if (returnType != QMetaType::QVariant) {
                returnValue = QVariant(returnType, (const void*) 0);
            }

            QGenericReturnArgument ret(method.typeName(), returnValue.data());
            result = method.invoke(service, ret,
                   QGenericArgument(typenames[0], param[0]),
                   QGenericArgument(typenames[1], param[1]),
                   QGenericArgument(typenames[2], param[2]),
                   QGenericArgument(typenames[3], param[3]),
                   QGenericArgument(typenames[4], param[4]),
                   QGenericArgument(typenames[5], param[5]),
                   QGenericArgument(typenames[6], param[6]),
                   QGenericArgument(typenames[7], param[7]),
                   QGenericArgument(typenames[8], param[8]),
                   QGenericArgument(typenames[9], param[9]));

            QServicePackage response = p.createResponse();

            if (result) {
                response.d->responseType = QServicePackage::Success;
                response.d->payload = returnValue;
            } else {
                response.d->responseType = QServicePackage::Failed;
            }
            dispatch->writePackage(response);

        }
        if (!result)
            qWarning( "%s::%s cannot be called.", service->metaObject()->className(), method.methodSignature().constData());
    } else {
        //client side
        Q_ASSERT(d->endPointType == ObjectEndPoint::Client);

        if (d->waitingOnReturnUuid == p.d->messageId) {
            d->functionReturned = true;
        }

        Response* response = openRequests.value(p.d->messageId);
        if (response){
            response->isFinished = true;
            if (p.d->responseType == QServicePackage::Failed) {
                response->result = 0;
                response->error = QLatin1Literal("methodCall QServicePakcage::Failed");
                response->isFinished = true;
                dispatch->waitingDone();
                return;
            }
            QVariant* variant = new QVariant(p.d->payload);
            response->result = reinterpret_cast<void *>(variant);

            dispatch->waitingDone();
        }
    }
}

/*
    Will block if return value expected
    Handles property calls
*/
QVariant ObjectEndPoint::invokeRemoteProperty(int metaIndex, const QVariant& arg, int /*returnType*/, QMetaObject::Call c )
{
    //client and service side
    Q_ASSERT(d->endPointType == ObjectEndPoint::Client
            || d->endPointType == ObjectEndPoint::Service);

    QServicePackage p;
    p.d = new QServicePackagePrivate();
    p.d->packageType = QServicePackage::PropertyCall;
    p.d->messageId = QUuid::createUuid();

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly|QIODevice::Append);
    stream << metaIndex << arg << c;
    p.d->payload = data;

    if (c == QMetaObject::ReadProperty) {
        //create response and block for answer
        Response* response = new Response();
        openRequests.insert(p.d->messageId, response);

        dispatch->writePackage(p);
        qServiceLog() << "class" << "objectendpoint"
                      << "event" << "invokeRemoteProperty"
                      << "progress" << "wait for result";
        waitForResponse(p.d->messageId);

        QVariant result;
        if (response->isFinished) {
            if (response->result == 0) {
                qWarning() << "Service property call failed";
            } else {
                QVariant* resultPointer = reinterpret_cast<QVariant* >(response->result);
                result = (*resultPointer);
                delete resultPointer;
            }
        } else {
            qDebug() << Q_FUNC_INFO << "response passed but not finished";
        }

        openRequests.take(p.d->messageId);
        delete response;

        return result;
    } else {
        dispatch->writePackage(p);
    }

    return QVariant();
}

/*
    Will block if return value expected
    Handles signal/slots
*/
QVariant ObjectEndPoint::invokeRemote(int metaIndex, const QVariantList& args, int returnType)
{
    //client side
    //Q_ASSERT(d->endPointType == ObjectEndPoint::Client);

    QServicePackage p;
    p.d = new QServicePackagePrivate();
    p.d->packageType = QServicePackage::MethodCall;
    p.d->messageId = QUuid::createUuid();

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly|QIODevice::Append);
    stream << metaIndex << args;
    p.d->payload = data;

    if (returnType == QMetaType::Void) {
        dispatch->writePackage(p);
    } else {
        //create response and block for answer
        Response* response = new Response();
        openRequests.insert(p.d->messageId, response);

        d->waitingOnReturnUuid = p.d->messageId;

        dispatch->writePackage(p);
        qServiceLog() << "class" << "objectendpoint"
                      << "event" << "invokeRemote"
                      << "progress" << "wait for result";
        waitForResponse(p.d->messageId);

        d->waitingOnReturnUuid = QUuid();

        QVariant result;
        if (response->isFinished) {
            if (response->result == 0) {
                qWarning() << "Remote function call failed";
            } else {
                QVariant* resultPointer = reinterpret_cast<QVariant* >(response->result);
                result = (*resultPointer);
                delete resultPointer;
            }
        } else {
            qDebug() << Q_FUNC_INFO << "response passed but not finished";
        }

        openRequests.take(p.d->messageId);
        delete response;

        return result;

    }

    return QVariant();
}

void ObjectEndPoint::waitForResponse(const QUuid& requestId)
{
    Q_ASSERT(d->endPointType == ObjectEndPoint::Client);
    if (openRequests.contains(requestId) ) {
        Response *r = openRequests.value(requestId);
        QTime elapsed;
        elapsed.start();
        while (r->isFinished == false && (elapsed.elapsed() < 15000)) {
            qServiceLog() << "class" << "objectendpoint"
                          << "event" << "waiting"
                          << "elapsed" << elapsed.elapsed()
                          << "name" << objectName()
                          << "uuid" << requestId.toString();
            int ret = dispatch->waitForData();
            if (ret != 0) {
                qWarning() << this << "SFW ipc error" << r->error;
                break;
            }
        }

        qServiceLog() << "class" << "objectendpoint"
                      << "event" << "waiting done"
                      << "elapsed" << elapsed.elapsed()
                      << "name" << objectName();

        if (r->isFinished == false) {
            qWarning() << "SFW IPC failure, remote end failed to respond to blocking IPC call in" << elapsed.elapsed()/1000 << "seconds." << objectName();
        }
        if (d->functionReturned) {
            d->functionReturned = false;
            QMetaObject::invokeMethod(this, "newPackageReady", Qt::QueuedConnection);
        }
    }
}

void ObjectEndPoint::setLookupTable(int *local, int *remote)
{
    localToRemote = local;
    remoteToLocal = remote;
}

#include "moc_objectendpoint_p.cpp"

QT_END_NAMESPACE
