/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativevaluespacesubscriber_p.h"

QT_BEGIN_NAMESPACE

class QDeclarativeValueSpaceSubscriberPrivate
{
public:
    QDeclarativeValueSpaceSubscriberPrivate();
    ~QDeclarativeValueSpaceSubscriberPrivate();

    QValueSpaceSubscriber *subscriber;
    // store our own copy, since the QVSS doesn't always seem
    // to return the current path properly
    QString path;
};

QDeclarativeValueSpaceSubscriberPrivate::QDeclarativeValueSpaceSubscriberPrivate() :
    subscriber(0)
{
}

QDeclarativeValueSpaceSubscriberPrivate::~QDeclarativeValueSpaceSubscriberPrivate()
{
    if (subscriber)
        delete subscriber;
}


/*!
    \qmlclass ValueSpaceSubscriber QDeclarativeValueSpaceSubscriber

    \brief The QValueSpaceSubscriber class allows applications to read and
           subscribe to Value Space paths.

    \ingroup qml-publishsubscribe

    The ValueSpaceSubscriber element is part of the \bold {QtMobility.publishsubscribe 1.1} module.

    Each \l ValueSpaceSubscriber element represents a single value or path in the Value Space. The
    path is set using the \i path property.

    Note that unlike the C++ class QValueSpaceSubscriber, the QML element has
    no default path. A path must be set before the subscriber will connect
    to the Value Space and begin receiving notifications.

    \code
    ValueSpaceSubscriber {
        id: nowPlaying
        path: "/applications/mediaplayer/now-playing"
    }
    \endcode

    The value is accessed using the \i value property.

    \code
    Text {
        text: nowPlaying.value
    }
    \endcode
*/

QDeclarativeValueSpaceSubscriber::QDeclarativeValueSpaceSubscriber() :
    d(new QDeclarativeValueSpaceSubscriberPrivate)
{
}

QDeclarativeValueSpaceSubscriber::~QDeclarativeValueSpaceSubscriber()
{
    delete d;
}

/*!
    \qmlproperty string ValueSpaceSubscriber::path

    This property holds the base path of the subscriber, and is read/write.
*/
void QDeclarativeValueSpaceSubscriber::setPath(QString path)
{
    if (d->subscriber) {
        if (d->path == path)
            return;
        d->subscriber->setPath(path);
    } else {
        d->subscriber = new QValueSpaceSubscriber(path);;
    }

    d->path = path;
    emit pathChanged();

    // re-connect the signal
    connect(d->subscriber, SIGNAL(contentsChanged()),
            this, SIGNAL(contentsChanged()));
}

QString QDeclarativeValueSpaceSubscriber::path() const
{
    return d->path;
}

/*!
    \qmlproperty QVariant ValueSpaceSubscriber::value

    This property holds the value of the key at the set path in the Value Space.
    Read-only.
*/
QVariant QDeclarativeValueSpaceSubscriber::value(const QString &subPath, const QVariant &def) const
{
    if (!d->subscriber)
        return QVariant();
    return d->subscriber->value(subPath, def);
}

/*!
    \qmlproperty QStringList ValueSpaceSubscriber::subPaths

    This property holds a list of known sub-paths of the currently set path.
*/
QStringList QDeclarativeValueSpaceSubscriber::subPaths() const
{
    if (!d->subscriber)
        return QStringList();
    return d->subscriber->subPaths();
}

/*!
    \qmlproperty bool ValueSpaceSubscriber::connected

    This property holds whether the subscriber is currently connected to the
    backing store of the Value Space.
*/
bool QDeclarativeValueSpaceSubscriber::isConnected() const
{
    if (!d->subscriber)
        return false;
    return d->subscriber->isConnected();
}

QT_END_NAMESPACE
