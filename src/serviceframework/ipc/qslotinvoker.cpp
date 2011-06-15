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
#include "qslotinvoker_p.h"
#include "qsignalintercepter_p.h"
#include <qmetaobject.h>
#include <qmetatype.h>
#include <qvarlengtharray.h>

/*!
    \class QSlotInvoker
    \inpublicgroup QtBaseModule
    \internal

    \brief The QSlotInvoker class provides an interface for invoking slots with explicit arguments
    \since 1.1

    IPC mechanisms need to intercept protocol messages and convert them into
    slot invocations, but it is generally impractical to create explicit code
    for every slot that needs to be dispatched.  The QSlotInvoker class allows
    an IPC dispatching mechanism to invoke slots in a generic fashion using
    the invoke() method.

    Methods that are marked with Q_INVOKABLE or Q_SCRIPTABLE can also
    be invoked with this class.

    \sa QSignalIntercepter

    \ingroup objectmodel
*/

QTM_BEGIN_NAMESPACE

class QSlotInvokerPrivate
{
public:
    QObject *receiver;
    QByteArray member;
    int memberIndex;
    bool destroyed;
    int returnType;
    int *types;
    int numArgs;

    ~QSlotInvokerPrivate()
    {
        if ( types )
            qFree( types );
    }
};

/*!
    Create a slot invoker that can invoke \a member on \a receiver.
    The object will be attached to \a parent, if present.
    \since 1.1
*/
QSlotInvoker::QSlotInvoker( QObject *receiver, const QByteArray &member,
                            QObject *parent )
    : QObject( parent )
{
    d = new QSlotInvokerPrivate();
    d->receiver = receiver;
    QByteArray name;
    if ( member.size() > 0 && member[0] >= '0' && member[0] <= '9' ) {
        // Strip off the member type code.
        name = member.mid(1);
    } else {
        name = member;
    }
    name = QMetaObject::normalizedSignature( name.constData() );
    d->member = name;
    d->destroyed = false;
    d->returnType = 0;
    d->types = 0;
    d->numArgs = 0;
    if ( receiver && name.size() > 0 ) {
        d->memberIndex
            = receiver->metaObject()->indexOfMethod( name.constData() );
    } else {
        d->memberIndex = -1;
    }
    if ( d->memberIndex != -1 ) {
        QMetaMethod method = receiver->metaObject()->method
                ( d->memberIndex );
        {
            connect( receiver, SIGNAL(destroyed()),
                     this, SLOT(receiverDestroyed()) );
            d->returnType =
                QSignalIntercepter::typeFromName( method.typeName() );
            d->types = QSignalIntercepter::connectionTypes
                ( name, d->numArgs );
            if ( !( d->types ) )
                d->destroyed = true;
        }
    } else {
        d->destroyed = true;
    }
}

/*!
    Destroy a slot invoker.
*/
QSlotInvoker::~QSlotInvoker()
{
    delete d;
}

/*!
    Returns true if the member is present on the object.
    \since 1.1
*/
bool QSlotInvoker::memberPresent() const
{
    return ! d->destroyed;
}

/*!
    Returns true if the member can be invoked with \a numArgs arguments.
    That is, the receiver has not been destroyed, the member is present,
    and it requires \a numArgs or less arguments.
    \since 1.1
*/
bool QSlotInvoker::canInvoke( int numArgs ) const
{
    if ( d->destroyed )
        return false;
    return ( numArgs >= d->numArgs );
}

/*!
    Returns the object that will receive slot invocations.
    \since 1.1
*/
QObject *QSlotInvoker::receiver() const
{
    return d->receiver;
}

/*!
    Returns the member that will receiver slot invocations.
    \since 1.1
*/
QByteArray QSlotInvoker::member() const
{
    return d->member;
}

/*!
    Returns the parameter types associated with this member.
    \since 1.1
*/
int *QSlotInvoker::parameterTypes() const
{
    return d->types;
}

/*!
    Returns the number of parameter types associated with this member.
    \since 1.1
*/
int QSlotInvoker::parameterTypesCount() const
{
    return d->numArgs;
}

/*!
    Invokes the slot represented by this object with the argument
    list \a args.  The slot's return value is returned from
    this method.  If the slot's return type is "void", then a
    QVariant instance of type QVariant::Invalid will be returned.

    If it is possible that the slot may throw an exception,
    it is the responsibility of the caller to catch and
    handle the exception.
    \since 1.1
*/
QVariant QSlotInvoker::invoke( const QList<QVariant>& args )
{
    int arg;
    QVariant returnValue;

    // Create a default instance of the return type for the result buffer.
    if ( d->returnType != (int)QVariant::Invalid ) {
        returnValue = QVariant( d->returnType, (const void *)0 );
    }

    // Bail out if the receiver object has already disappeared.
    if ( d->destroyed )
        return returnValue;

    // Check that the number of arguments is compatible with the slot.
    int numArgs = args.size();
    if ( numArgs < d->numArgs ) {
        qWarning( "QSlotInvoker::invoke: insufficient arguments for slot" );
        return returnValue;
    } else if ( numArgs > d->numArgs ) {
        // Drop extraneous arguments.
        numArgs = d->numArgs;
    }

    // Construct the raw argument list.
    QVarLengthArray<void *, 32> a( numArgs + 1 );
    if ( d->returnType == (int)QVariant::Invalid )
        a[0] = 0;
    else
        a[0] = returnValue.data();
    for ( arg = 0; arg < numArgs; ++arg ) {
        if ( d->types[arg] == QSignalIntercepter::QVariantId ) {
            a[arg + 1] = (void *)&( args[arg] );
        } else if ( args[arg].userType() != d->types[arg] ) {
            qWarning( "QSlotInvoker::invoke: argument %d has incorrect type",
                      arg );
            return QVariant();
        } else {
            a[arg + 1] = (void *)( args[arg].data() );
        }
    }

    // Invoke the specified slot.
    d->receiver->qt_metacall( QMetaObject::InvokeMetaMethod,
                                     d->memberIndex, a.data() );
    return returnValue;
}

void QSlotInvoker::receiverDestroyed()
{
    d->destroyed = true;
}

#include "moc_qslotinvoker_p.cpp"

QTM_END_NAMESPACE
