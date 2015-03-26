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

#include "qservicereply.h"
#include "qservicereply_p.h"

#include <QThread>

#define Q_SERVICE_REPLY_DEBUG 1

#ifdef Q_SERVICE_REPLY_DEBUG
#include <QDebug>
#endif

/*!
    \class QServiceReplyBase
    \ingroup servicefw
    \inmodule QtServiceFramework
    \brief The QServiceReplyBase class tracks non-blocking service framework calls.

    The QServiceReplyBase class is a data-carrying class.  Each instance is short-lived
    and only exists during the lifetime of a QServiceManager call.  The QServiceReplyBase
    instance never owns any of the data it points to, it just serves to carry the payload
    from the background request back to the caller.

    When an instance is first created, after being returned from QServiceManager::loadInterface(),
    that instance will return false from both the isRunning() and isFinished() functions.
    Then the request is started, and it will emit the started() signal.  After that, and
    until the request is completed, the isRunning() function will return true.
    Finally the request is completed, and it will emit the finished() signal.  After that
    the isRunning() function will return false, and the isFinished() function will return
    true.  At this point client code can access the proxyObject() function to obtain the
    payload of the service request.

    Typically there should be no reason to construct a QServiceReplyBase (or sub-class)
    instance: instead simply use the instances returned from the QServiceManager::loadInterface()
    function.

    The service QObject returned from the proxyObject() function is owned by the caller of
    the original QServiceManager::loadInterface() function which resulted in the
    QServiceReplyBase instance.  Likewise the QServiceObjectBase instance itself is owned
    by the caller and after the payload is retrieved, it should be queued for destruction
    with deleteLater() in the function which handles the finished() signal.

    As a convenience the manager() function will return the QServiceManager associated with
    the request, and the request() function will return a QString indicating the details of
    the request itself.

    For performance reasons the QServiceReplyBase object is \bold{not synchronized}, and
    thread-safety is ensured by observing the following invariant condition:
    \list
        \o all calls to non-const methods are serialised
    \endlist
    In general client code should never have to worry about this, since the private slots
    which can modify the reply are called via queued signal-slot connections behind the scenes
    ensuring that such accesses are serialised.

    In the case of a request based on an interface name, request() will return the interface name;
    otherwise in the case of a request based on a descriptor it will return the interface name
    of the descriptor.

    \sa QServiceReplyTyped, QServiceManager
*/

/*!
    \class QServiceReplyTyped
    \ingroup servicefw
    \inmodule QtServiceFramework
    \brief The QServiceReplyTyped class tracks typed non-blocking service framework calls.

    This templated sub-class of QServiceReplyBase returns QObjects that are
    already conveniently cast to the templated type.  In all other respects instances of this
    class function exactly the same as QServiceReplyBase.

    To obtain a typed payload class, rather than just a QObject instance, you can use
    one of QServiceManager's typed request functions and a typed QServiceReplyTyped
    instantiation will be returned.

    To obtain the untyped version of the QObject service, call the baseObject() function as
    for the QServiceReplyBase class.

    \sa QServiceReplyBase, QServiceManager
*/

/*!
    Constructs a new QServiceReplyBase object.  All values are set to defaults.  Generally
    creating QServiceReplayBase instances should be left to the QServiceManager.
*/
QServiceReplyBase::QServiceReplyBase(QObject *parent)
    : QObject(parent),
      d(new QServiceReplyPrivate)
{
    // nothing to do here
}

/*!
    Destroys this object recovering all resources.
*/
QServiceReplyBase::~QServiceReplyBase()
{
    delete d;
}

/*!
    Convenience function that returns an informative string of the request which was
    issued when this reply was created.  This string is not used by the request processor
    in any way and exists mainly for logging and debugging purposes.
*/
QString QServiceReplyBase::request() const
{
    return d->request;
}

/*!
    Sets the informative \a request string for this reply.  This function is called by the
    QServiceManager object when the request is created.  In general client code
    should not need to call this function.
*/
void QServiceReplyBase::setRequest(const QString &request)
{
    Q_ASSERT_X(thread() == QThread::currentThread(), Q_FUNC_INFO, "Reply object access violation!");
    d->request = request;
}

/*!
    Returns true if the QServiceReplyBase isNoError completed.  When this is true, the
    baseObject() and proxyObject() functions may be called to retrieve the payload
    of the reply.  Note that you should check the value of the error() function
    to see if the request completed successfully.

    \sa isRunning(), error()
*/
bool QServiceReplyBase::isFinished() const
{
    return d->finished;
}

/*!
    Returns true if the QServiceReplyBase is being processed.  When this is true,
    the baseObject() and proxyObject() should not be accessed as they are in an
    undefined state.  Instead wait for the finished() signal to be emitted and
    access those value then.

    \sa isFinished()
*/
bool QServiceReplyBase::isRunning() const
{
    return d->running;
}

/*!
    Returns any error state that may have been set on this reply.

    \sa isFinished()
*/
QServiceManager::Error QServiceReplyBase::error() const
{
    return d->error;
}

/*!
    \internal
    Sets the error condition of the reply to \a error, indicating that processing of
    the associated request has encountered a problem.

    Note that this is a private slot, and should be called by a queued connection, so
    that any data modification is only done in the objects own thread.
*/
void QServiceReplyBase::setError(QServiceManager::Error error)
{
    Q_ASSERT_X(thread() == QThread::currentThread(), Q_FUNC_INFO, "Reply object access violation!");
    if (d->error != error) {
        d->error = error;
        emit errorChanged();
    }
}

/*!
    \internal
    Starts the reply, indicating that processing of the associated request has begun.

    Note that this is a private slot, and should be called by a queued connection, so
    that any data modification is only done in the objects own thread.
*/
void QServiceReplyBase::start()
{
    Q_ASSERT_X(thread() == QThread::currentThread(), Q_FUNC_INFO, "Reply object access violation!");
    if (!d->running) {
        d->running = true;
        emit started();
    }
#ifdef Q_SERVICE_REPLY_DEBUG
    else
    {
        qWarning() << "Starting request that is" << ((d->finished) ? "finished:" : "started:") << d->request;
    }
    Q_ASSERT(!d->finished);
#endif
}

/*!
    \internal
    Finishes the reply, indicating that processing of the associated request has completed.

    Note that this is a private slot, and should be called by a queued connection, so
    that any data modification is only done in the objects own thread.
*/
void QServiceReplyBase::finish()
{
    Q_ASSERT_X(thread() == QThread::currentThread(), Q_FUNC_INFO, "Reply object access violation!");
    if (!d->finished) {
        d->running = false;
        d->finished = true;
        emit finished();
    }
#ifdef Q_SERVICE_REPLY_DEBUG
    else
    {
        qWarning() << "Attempt to finish request that has already finished:" << d->request;
    }
    Q_ASSERT(!d->running);
#endif
}
