/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativevaluespacesubscriber_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype ValueSpaceSubscriber
    \instantiates QDeclarativeValueSpaceSubscriber
    \inqmlmodule QtPublishSubscribe
    \ingroup qml-publishsubscribe

    \brief The QValueSpaceSubscriber class allows applications to read and subscribe to Value Space.

    Each \l ValueSpaceSubscriber element represents a single value or path in the Value Space. The
    path is set using the \e path property.

    Note that unlike the C++ class QValueSpaceSubscriber, the QML element has no default path. A path
    must be set before the subscriber will connect to the Value Space and begin receiving notifications.

    \code
    ValueSpaceSubscriber {
        id: nowPlaying
        path: "/applications/mediaplayer/now-playing"
    }
    \endcode

    The value is accessed using the \e value property:

    \code
    Text {
        text: nowPlaying.value
    }
    \endcode
*/

QDeclarativeValueSpaceSubscriber::QDeclarativeValueSpaceSubscriber()
    : d_ptr(0)
{
}

QDeclarativeValueSpaceSubscriber::~QDeclarativeValueSpaceSubscriber()
{
}

/*!
    \qmlproperty string ValueSpaceSubscriber::path

    This property holds the base path of the subscriber, and it should be set before connecting to
    any Value Space layers and receiving notifications.
*/
void QDeclarativeValueSpaceSubscriber::setPath(QString path)
{
    if (!d_ptr) {
        d_ptr = new QValueSpaceSubscriber(path, this);
        connect(d_ptr, SIGNAL(contentsChanged()), this, SIGNAL(contentsChanged()));
    } else {
        d_ptr->setPath(path);
        emit pathChanged();
    }
}

QString QDeclarativeValueSpaceSubscriber::path() const
{
    if (d_ptr)
        return d_ptr->path();
    else
        return QString::null;
}

/*!
    \qmlproperty QVariant ValueSpaceSubscriber::value

    This property holds the value of the key at the set path in the Value Space.
    Read-only.
*/
QVariant QDeclarativeValueSpaceSubscriber::value(const QString &subPath, const QVariant &def) const
{
    if (d_ptr)
        return d_ptr->value(subPath, def);
    else
        return QVariant();
}

/*!
    \qmlproperty QStringList ValueSpaceSubscriber::subPaths

    This property holds a list of known sub-paths of the currently set path.
*/
QStringList QDeclarativeValueSpaceSubscriber::subPaths() const
{
    if (d_ptr)
        return d_ptr->subPaths();
    else
        return QStringList();
}

/*!
    \qmlproperty bool ValueSpaceSubscriber::connected

    This property holds whether the subscriber is currently connected to the
    backing store of the Value Space.
*/
bool QDeclarativeValueSpaceSubscriber::isConnected() const
{
    if (d_ptr)
        return d_ptr->isConnected();
    else
        return false;
}

QT_END_NAMESPACE
