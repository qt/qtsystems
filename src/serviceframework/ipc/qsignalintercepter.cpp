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
#include "qsignalintercepter_p.h"
#include <qmetaobject.h>
#include <qmetatype.h>
#include <qobjectdefs.h>
#include <qdebug.h>

#include <stdlib.h>

/*!
    \class QSignalIntercepter
    \inpublicgroup QtBaseModule
    \internal

    \brief The QSignalIntercepter class provides an interface for intercepting signals as meta-calls

    IPC mechanisms need to intercept signals and convert them into protocol
    messages, but it is generally impractical to create a slot for every
    signal that needs to be dispatched.  The QSignalIntercepter class allows
    signals to be intercepted as meta-calls so that IPC dispatching can
    be implemented in a generic fashion.

    The activated() method is called whenever the signal is emitted,
    with the arguments in a typed list.

    \sa QSlotInvoker

    \ingroup objectmodel
*/

QT_BEGIN_NAMESPACE

class QSignalIntercepterPrivate
{
public:
    QObject *sender;
    QByteArray signal;
    int signalIndex;
    int destroyIndex;
    int slotIndex;
    int *types;
    int numArgs;

    ~QSignalIntercepterPrivate()
    {
        if ( types )
            free(types);
    }
};

/*!
    Create a new signal intercepter which traps \a signal on \a sender.
    The object will be attached to \a parent, if present.
*/
QSignalIntercepter::QSignalIntercepter
            ( QObject *sender, const QByteArray& signal, QObject *parent )
    : QObject( parent )
{
    // Initialize the private members.
    d = new QSignalIntercepterPrivate();
    d->sender = sender;
    d->signal = signal;
    d->signalIndex = -1;
    d->destroyIndex = -1;
    d->slotIndex = -1;
    d->types = 0;

    // Resolve the indices of the signals we are interested in.
    if ( sender && signal.size() > 0 ) {
        // '2' is QSIGNAL_CODE in Qt 4.4 and below,
        // '6' is QSIGNAL_CODE in Qt 4.5 and higher.
        if ( signal[0] != '2' && signal[0] != '6' ) {
            qWarning( "QSignalIntercepter: `%s' is not a valid signal "
                      "specification", signal.constData() );
            return;
        }
        QByteArray name = QMetaObject::normalizedSignature
            ( signal.constData() + 1 );
        d->signalIndex
            = sender->metaObject()->indexOfSignal( name.constData() );
        if ( d->signalIndex < 0 ) {
            qWarning( "QSignalIntercepter: no such signal: %s::%s",
                      sender->metaObject()->className(), signal.constData() );
            return;
        }
        d->destroyIndex
            = sender->metaObject()->indexOfSignal( "destroyed()" );
        d->types = connectionTypes( name, d->numArgs );
    }

    // Derive a fake slot index to use in our manual qt_metacall implementation.
    d->slotIndex = staticMetaObject.methodCount();

    // Connect up the signals.
    if ( d->signalIndex >= 0 ) {
        QMetaObject::connect( sender, d->signalIndex,
                              this, d->slotIndex,
                              Qt::DirectConnection, 0 );
    }
    if ( d->destroyIndex >= 0 ) {
        QMetaObject::connect( sender, d->destroyIndex,
                              this, d->slotIndex + 1,
                              Qt::DirectConnection, 0 );
    }
}

/*!
    Destroy a signal intercepter.
*/
QSignalIntercepter::~QSignalIntercepter()
{
    if ( d->signalIndex >= 0 ) {
        QMetaObject::disconnect( d->sender, d->signalIndex,
                                 this, d->slotIndex );
    }
    if ( d->destroyIndex >= 0 ) {
        QMetaObject::disconnect( d->sender, d->destroyIndex,
                                 this, d->slotIndex + 1 );
    }
    delete d;
}

/*!
    Returns the sender that this signal interceptor is attached to.
*/
QObject *QSignalIntercepter::sender() const
{
    return d->sender;
}

/*!
    Returns the name of the signal that this signal interceptor is attached to.
*/
QByteArray QSignalIntercepter::signal() const
{
    return d->signal;
}

/*!
    Returns true if this signal intercepter is valid; that is, there was
    a signal present with the specified parameters when this object
    was constructed.
*/
bool QSignalIntercepter::isValid() const
{
    return ( d->signalIndex != -1 );
}

/*!
    \internal
*/
int QSignalIntercepter::qt_metacall(QMetaObject::Call c, int id, void **a)
{
    id = QObject::qt_metacall(c, id, a);
    if (id < 0)
        return id;
    if (c == QMetaObject::InvokeMetaMethod) {
        switch (id) {
            case 0: {
                // The signal we are interested in has been activated.
                if ( d->types ) {
                    QList<QVariant> args;
                    for ( int i = 0; i < d->numArgs; ++i ) {
                        if ( d->types[i] != QVariantId ) {
                            QVariant arg( d->types[i], a[i + 1] );
                            args.append( arg );
                        } else {
                            args.append( *((const QVariant *)( a[i + 1] )) );
                        }
                    }
                    activated( args );
                }
            }
            break;

            case 1: {
                // The sender has been destroyed.  Clear the signal indices
                // so that we don't try to do a manual disconnect when our
                // own destructor is called.
                d->signalIndex = -1;
                d->destroyIndex = -1;
            }
            break;
        }
        id -= 2;
    }
    return id;
}

/*!
    \fn void QSignalIntercepter::activated( const QList<QVariant>& args )

    Called when the signal that is being intercepted is activated.
    The arguments to the signal are passed in the list \a args.
*/

// Get the QVariant type number for a type name.
int QSignalIntercepter::typeFromName( const QByteArray& type )
{
    int id;
    if (type.endsWith('*'))
        return QMetaType::VoidStar;
    else if ( type.size() == 0 || type == "void" )
        return QMetaType::Void;
    id = QMetaType::type( type.constData() );
    if (id == QMetaType::QVariant)
        return QSignalIntercepter::QVariantId;
    return id;
}

/*!
    Returns the connection types associated with a signal or slot \a member
    specification.  The array of types is returned from this function,
    and the number of arguments is returned in \a nargs.  Returns null
    if \a member is invalid.  The return value must be freed with qFree().
*/
int *QSignalIntercepter::connectionTypes( const QByteArray& member, int& nargs )
{
    // Based on Qt's internal queuedConnectionTypes function.
    nargs = 0;
    int *types = 0;
    const char *s = member.constData();
    while (*s != '\0' && *s != '(') { ++s; }
    if ( *s == '\0' )
        return 0;
    ++s;
    const char *e = s;
    while (*e != ')') {
        ++e;
        if (*e == ')' || *e == ',')
            ++nargs;
    }

    types = (int *) malloc((nargs + 1) * sizeof(int));
    types[nargs] = 0;
    for (int n = 0; n < nargs; ++n) {
        e = s;
        while (*s != ',' && *s != ')')
            ++s;
        QByteArray type(e, s-e);
        ++s;

        types[n] = typeFromName(type);
        if (!types[n]) {
            qWarning("QSignalIntercepter::connectionTypes: Cannot marshal arguments of type '%s'", type.data());
            free(types);
            return 0;
        }
    }
    return types;
}

QT_END_NAMESPACE
