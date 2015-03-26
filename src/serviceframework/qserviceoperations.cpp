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

#include "qserviceoperations_p.h"
#include "qservicerequest_p.h"
#include "qservicereply.h"
#include "qservicedebuglog_p.h"
#include "qservicereply_p.h"

Q_GLOBAL_STATIC(QServiceOperations, q_service_operations_object)

//// Implementation notes:  The background thread is implemented as a facade
//// behind which the actual worker object is hidden.  The facade - an instance
//// of QServiceOperations - lives in (has QObject affinity to) the *foreground*
//// calling thread.
////
//// The worker object, an instance of QServiceOperationProcessor, is created
//// on the stack inside the run() method of the QThread, thus ensuring it does
//// not need to be cleaned up by some deleter.  Since it is created in the
//// run() it lives in the *background* thread, and connections to it from the
//// facade (QServiceOperations) are thus queued connections.
////
//// Signal-slot connections are thus two-level indirection, but what this means
//// is that no locking is needed in QServiceOperationProcessor since it can
//// only be accessed within its own thread, by calls that are serialised into
//// its slots by the signal-slot event queue.

QServiceOperations::QServiceOperations(QObject *parent)
    : QThread(parent)
    , m_engageCount(0)
{
    // This call must be executed before the first signal-slot connection
    // which uses a QServiceRequest.  Since this constructor will need to fire
    // before we have an instance of this class to connect anything to
    // we should be OK here.
    qRegisterMetaType<QServiceRequest>("QServiceRequest");
    qRegisterMetaType<QServiceManager::Error>("QServiceManager::Error");

    // created in the constructor otherwise we can signals/slots
    QServiceOperationProcessor *processor = new QServiceOperationProcessor();
    processor->moveToThread(this);

    // This call creates a queued connection
    connect(this, SIGNAL(newRequest(QServiceRequest)),
            processor, SLOT(handleRequest(QServiceRequest)), Qt::QueuedConnection);
    connect(this, SIGNAL(destroyed()),
            processor, SLOT(deleteLater()));

    qServiceLog() << "event" << "new"
                  << "class" << "QServiceOperations";
}

QServiceOperations::~QServiceOperations()
{
    qServiceLog() << "event" << "delete"
                  << "class" << "QServiceOperations";
}

/*
    Return the per-process instance of the thread facade object
*/
QServiceOperations *QServiceOperations::instance()
{
    qServiceLog() << "event" << "instance access"
                  << "class" << "QServiceOperations"
                  << "clients" << q_service_operations_object()->clientCount();

    return q_service_operations_object();
}

/*
    Calling code engages the thread facade, marking themselves as users of it.
    The first caller will cause the thread to start.  Client code should call
    engage early during initialisation as the once-off thread startup cost could
    cause latency for time critical operations if its done on-demand.
*/
void QServiceOperations::engage()
{
    if (m_engageCount.testAndSetAcquire(0, 1)) {
        start();
    } else {
        m_engageCount.ref();
    }
    qServiceLog() << "event" << "engage"
                  << "class" << "QServiceOperations"
                  << "clients" << q_service_operations_object()->clientCount();
}

/*
    Client code disengages from the thread, releasing themselves from use of it.
    The last client to do so will cause the thread to quit, exiting the event
    loop, and running the thread shutdown process.  Normally this should be a
    100ms or so, but if for some reason there are problems it could take longer.
    This might be a bug, or exceptional condition, or some long running request
    that just happens to be in flight at the time.  The current implementation
    will try 3 times to quit the thread (with a 500ms window each time) for a
    total worst-case of 1500ms; before calling terminate on the thread.  After
    that the termination will still take a short period of time before its
    safe to clean up the thread object.
*/
void QServiceOperations::disengage()
{
    qServiceLog() << "event" << "disengage"
                  << "class" << "QServiceOperations"
                  << "clients" << q_service_operations_object()->clientCount();

    if (!m_engageCount.deref()) {
        qServiceLog() << "event" << "shutdown"
                      << "class" << "QServiceOperations";
        quit();
        int triesLeft = 3;
        bool exitedCleanly = false;
        while (triesLeft) {
            exitedCleanly = wait(500);
            if (exitedCleanly)
                break;
            qWarning() << "Waiting for QServiceOperations background thread to exit...";
            triesLeft--;
        }
        if (!exitedCleanly) {
            qWarning() << "...forcing termination of QServiceOperations thread!";
            terminate();
            wait();
        }
    }
}

/*
    Make a new request.  This slot will be called synchronously from the client
    code, since the client and the facade are in the same thread.  However this
    slot then forwards the request on via a queued connection to the internal
    operations object.  The queue automatically gives thread safety to the
    processor meaning no locking is required.
*/
void QServiceOperations::initiateRequest(const QServiceRequest &req)
{
    qServiceLog() << "event" << "initiate"
                  << "class" << "QServiceOperations"
                  << "iface" << req.descriptor().interfaceName();

    emit newRequest(req);
}

/*
    This function will be called when start() is called in the engage() code above.
    All the code in run() will execute in the background thread.
*/
void QServiceOperations::run()
{
    qServiceLog() << "event" << "run"
                  << "class" << "QServiceOperations"
                  << "priority" << (qint32)priority();

    // spin the per-thread event loop
    exec();
}


////////////////////////////////////////////////////
////
////    QServiceOperationProcessor implementation

/*
    Constructor - stateless object at present.
*/
QServiceOperationProcessor::QServiceOperationProcessor(QObject *parent)
    : QObject(parent), inRequest(false)
{
    qServiceLog() << "event" << "new"
                  << "class" << "QServiceOperationProc";
}

/*
    Destructor - no resources.  Should destruct when the main event
    loop of the facade exits.
*/
QServiceOperationProcessor::~QServiceOperationProcessor()
{
    qServiceLog() << "event" << "delete"
                  << "class" << "QServiceOperationProc";
}

/*
    Do the actual work.  Note that all calls to this function are serialised by
    the queued signal-slot connection from the facade.  But since SFW could
    spin the even loop we need to lock loadInterface so it's only called once
*/
void QServiceOperationProcessor::handleRequest(const QServiceRequest &inrequest)
{
    pendingList.append(inrequest);

    if (inRequest) {
        qServiceLog() << "event" << "already handle req"
                      << "class" << "QServiceOperationProc"
                      << "iface" << inrequest.descriptor().interfaceName();
        return;
    }

    inRequest = true;

    QServiceRequest req;

    while (!pendingList.isEmpty()) {
        req = pendingList.takeFirst();

        qServiceLog() << "event" << "handle req start"
                      << "class" << "QServiceOperationProc"
                      << "iface" << inrequest.descriptor().interfaceName();

        QServiceInterfaceDescriptor descriptor = req.descriptor();
        QServiceReply *reply = req.reply();

        QServiceManager mgr(req.scope());

        QMetaObject::invokeMethod(reply, "start", Qt::QueuedConnection);

        if (req.requestType() == QServiceRequest::DefaultInterfaceRequest) {
            // OK, this was a request based on the interface name rather than a
            // fully specified descriptor
            qDebug() << "Asking for the default interface for" << mgr.interfaceDefault(req.interfaceName());
            descriptor = mgr.interfaceDefault(req.interfaceName());
        } else {
            qDebug() << "NOT asking for default";
        }

        if (!descriptor.isValid()) {
            qDebug() << "Failed to fetch default";
            QMetaObject::invokeMethod(reply, "setError", Qt::QueuedConnection, Q_ARG(QServiceManager::Error, QServiceManager::InvalidServiceInterfaceDescriptor));
            QMetaObject::invokeMethod(reply, "finish", Qt::QueuedConnection);
            continue;
        }

        QObject *obj = 0;
        int serviceType = descriptor.attribute(QServiceInterfaceDescriptor::ServiceType).toInt();
        const QString location = descriptor.attribute(QServiceInterfaceDescriptor::Location).toString();

        if (serviceType == QService::InterProcess) {
            obj = mgr.loadInterProcessService(descriptor, location);
        } else {

            const QString serviceFilePath = mgr.resolveLibraryPath(location);
            if (serviceFilePath.isEmpty()) {
                QMetaObject::invokeMethod(reply, "setError", Qt::QueuedConnection, Q_ARG(QServiceManager::Error, QServiceManager::InvalidServiceLocation));
                QMetaObject::invokeMethod(reply, "finish", Qt::QueuedConnection);
                continue;
            }

            obj = mgr.loadInProcessService(descriptor, serviceFilePath);
            if (!obj) {
                QMetaObject::invokeMethod(reply, "setError", Qt::QueuedConnection, Q_ARG(QServiceManager::Error, QServiceManager::PluginLoadingFailed));
                QMetaObject::invokeMethod(reply, "finish", Qt::QueuedConnection);
                continue;
            }
        }

        // loadInterface() can return null
        if (obj) {
            obj->moveToThread(reply->thread());
        }

        QMetaObject::invokeMethod(reply, "setProxyObject", Qt::QueuedConnection, Q_ARG(QObject*, obj));
        QMetaObject::invokeMethod(reply, "finish", Qt::QueuedConnection);

        qServiceLog() << "event" << "handle req done"
                      << "class" << "QServiceOperationProc"
                      << "iface" << inrequest.descriptor().interfaceName();
    }

    inRequest = false;
}
