/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "objectendpoint_p.h"
#include "instancemanager_p.h"
#include <private/qmetaobjectbuilder_p.h>
#include "proxyobject_p.h"
#include "qsignalintercepter_p.h"
#include <QTimer>
#include <QEventLoop>
#include <QEvent>
#include <QVarLengthArray>
#include <QTime>
#include <QCoreApplication>

QT_BEGIN_NAMESPACE

class Response
{
public:
    Response() : isFinished(false), result(0), loop(new QEventLoop)
    { }

    ~Response()
    { delete loop; }

    bool isFinished;
    void* result;
    QEventLoop *loop;
};

class ServiceSignalIntercepter : public QSignalIntercepter
{
    //Do not put Q_OBJECT here
public:
    ServiceSignalIntercepter(QObject* sender, const QByteArray& signal,
            ObjectEndPoint* parent)
        : QSignalIntercepter(sender, signal, parent), endPoint(parent)
    {

    }

    void setMetaIndex(int index)
    {
        metaIndex = index;
    }

protected:
    void activated( const QList<QVariant>& args )
    {
        //qDebug() << signal() << "emitted." << args;
        endPoint->invokeRemote(metaIndex, args, QMetaType::Void);
    }
private:
    ObjectEndPoint* endPoint;
    int metaIndex;

};

class ObjectEndPointPrivate
{
public:
    ObjectEndPointPrivate()
    {
        functionReturned = false;
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
                    QByteArray signal = method.signature();
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

    //used on client and service side
    ObjectEndPoint::Type endPointType;
    ObjectEndPoint* parent;

    //used on service side
    QRemoteServiceRegister::Entry entry;
    QUuid serviceInstanceId;

    // user on the client side
    bool functionReturned;
};

ObjectEndPoint::ObjectEndPoint(Type type, QServiceIpcEndPoint* comm, QObject* parent)
    : QObject(parent), dispatch(comm), service(0), security(0), localToRemote(0),
      remoteToLocal(0)
{
    Q_ASSERT(dispatch);
    d = new ObjectEndPointPrivate;
    d->parent = this;
    d->endPointType = type;

    dispatch->setParent(this);
    connect(dispatch, SIGNAL(readyRead()), this, SLOT(newPackageReady()), Qt::QueuedConnection);
    connect(dispatch, SIGNAL(disconnected()), this, SLOT(disconnected()), Qt::QueuedConnection);
    if (type == Client) {
        return; //we are waiting for conctructProxy() call
    } else {
        if (dispatch->packageAvailable())
            QTimer::singleShot(0, this, SLOT(newPackageReady()));
    }
}

ObjectEndPoint::~ObjectEndPoint()
{
    delete d;
}

void ObjectEndPoint::disconnected()
{
    if (d->endPointType == Service) {
        InstanceManager::instance()->removeObjectInstance(d->entry, d->serviceInstanceId);
    }
    foreach (Response *r, openRequests) {
        r->loop->exit(-1);
    }

    deleteLater();
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
    waitForResponse(p.d->messageId);

    if (response->isFinished) {
        if (response->result == 0)
            qWarning() << "Request for remote service failed";
        else
            service = reinterpret_cast<QServiceProxy* >(response->result);
    } else {
        qDebug() << "response passed but not finished";
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

        if (security && !security->isAuthorized(p.d->packageType, p.d->entry)) {
            qWarning() << "Failed auth, trying to terminate connection";
            this->disconnected();
            return;
        }

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
                response->loop->exit(-1);
                qWarning() << "Service method call failed";
                return;
            }
            QVariant* variant = new QVariant(p.d->payload);
            response->result = reinterpret_cast<void *>(variant);

            response->loop->exit();
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
            response->isFinished = true;
            response->loop->exit(-1);
            qWarning() << "Service instantiation failed";
            return;
        }
        //deserialize meta object and
        //create proxy object
        QServiceProxy* proxy = new QServiceProxy(p.d->payload.toByteArray(), this);
        response->result = reinterpret_cast<void *>(proxy);
        response->isFinished = true;

        //wake up waiting code
        response->loop->exit();

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

        //serialize meta object
        QByteArray data;
        QDataStream stream( &data, QIODevice::WriteOnly | QIODevice::Append );
        QMetaObjectBuilder builder(meta);
        builder.serialize(stream);

        //instantiate service object from type register
        service = m->createObjectInstance(p.d->entry, d->serviceInstanceId);
        if (!service) {
            qWarning() << "Cannot instanciate service object";
            dispatch->writePackage(response);
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
        const int returnType = QMetaType::type(method.typeName());

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
        Q_ASSERT(d->endPointType == ObjectEndPoint::Service);

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
        if (returnType == QMetaType::Void && strcmp(method.typeName(), "QVariant")) {
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
            if (returnType != QVariant::Invalid && strcmp(method.typeName(), "QVariant")) {
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
            qWarning( "%s::%s cannot be called.", service->metaObject()->className(), method.signature());
    } else {
        //client side
        Q_ASSERT(d->endPointType == ObjectEndPoint::Client);

        d->functionReturned = true;

        Response* response = openRequests.value(p.d->messageId);
        if (response){
            response->isFinished = true;
            if (p.d->responseType == QServicePackage::Failed) {
                response->result = 0;
                response->loop->exit(-1);
                return;
            }
            QVariant* variant = new QVariant(p.d->payload);
            response->result = reinterpret_cast<void *>(variant);

            response->loop->exit();
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
            qDebug() << "response passed but not finished";
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

        dispatch->writePackage(p);
        waitForResponse(p.d->messageId);

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
            qDebug() << "response passed but not finished";
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
        QTimer::singleShot(30000, r->loop, SLOT(quit()));
        r->loop->exec();
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

void ObjectEndPoint::setServiceSecurity(QServiceSecurity *serviceSecurity)
{
    security = serviceSecurity;
}

#include "moc_objectendpoint_p.cpp"

QT_END_NAMESPACE
