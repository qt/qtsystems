/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#include <qvaluespacesubscriber.h>

#include "qvaluespacemanager_p.h"
#include "qvaluespacesubscriber_p.h"

#include <QtCore/qset.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

/*!
    \class QValueSpaceSubscriber
    \brief The QValueSpaceSubscriber class allows applications to read and subscribe to Value Space paths.
    \inmodule QtSystems

    By default QValueSpaceSubscriber can read values from and report change notifications for all
    available Value Space layers.  Only data from the layer with the highest order and that
    contains the specific key is returned by this class.

    The layers that QValueSpaceSubscriber accesses can be limited by specifying either a
    \l {QValueSpace::LayerOptions}{filter} or a QUuid at construction time.

    Applications subscribe to a particular path in the Value Space.  If anything under that path
    changes the contextChanged() signal is emitted.  For example given the schema:

    \code
    /Device/Buttons = 3
    /Device/Buttons/1/Name = Menu
    /Device/Buttons/1/Usable = true
    /Device/Buttons/2/Name = Select
    /Device/Buttons/2/Usable = false
    /Device/Buttons/3/Name = Back
    /Device/Buttons/3/Usable = true
    \endcode

    The code:

    \code
        QValueSpaceSubscriber *buttons = new QValueSpaceSubscriber("/Device/Buttons");
        QObject::connect(buttons, SIGNAL(contentsChanged()), this, SLOT(buttonInfoChanged()));
    \endcode

    will invoke the \c {buttonInfoChanged()} slot whenever any value under \c {/Device/Buttons}
    changes.  This includes the value of \c {/Device/Buttons} itself, a change of a subpath such as
    \c {/Device/Buttons/2/Name} or the creation or removal of a subpath.
*/

/*!
    \fn QValueSpaceSubscriber::contentsChanged()

    Emitted whenever any value under the current path changes.

    \bold {Note:} that if a value changes multiple times in quick succession, only the most recent
    value may be accessible via the value() function.
*/

/*!
    \property QValueSpaceSubscriber::path

    This property holds the current path that the QValueSpaceSubscriber refers to.

    Settings this property causes the QValueSpaceSubscriber to disconnect and reconnect to the
    Value Space with the new path.  As a result all signal/slot connections are disconnected.
*/

/*!
    \property QValueSpaceSubscriber::value

    This property holds the value of the path that this QValueSpaceSubscriber refers to.
*/

void QValueSpaceSubscriberPrivateProxy::handleChanged(quintptr handle)
{
    QAbstractValueSpaceLayer *layer = qobject_cast<QAbstractValueSpaceLayer *>(sender());

    for (int i = 0; i < readers.count(); ++i) {
        if (readers.at(i).first == layer && readers.at(i).second == handle) {
            emit changed();
            return;
        }
    }
}

static LayerList matchLayers(const QString &path, QValueSpace::LayerOptions filter)
{
    LayerList list;

    QValueSpaceManager *manager = QValueSpaceManager::instance();
    if (!manager)
        return list;

    // Invalid filter combination.
    if ((filter & QValueSpace::PermanentLayer && filter & QValueSpace::TransientLayer)
        || (filter & QValueSpace::WritableLayer && filter & QValueSpace::ReadOnlyLayer)) {
        return list;
    }

    const QList<QAbstractValueSpaceLayer *> &readerList = manager->getLayers();

    for (int ii = 0; ii < readerList.count(); ++ii) {
        QAbstractValueSpaceLayer *read = readerList.at(ii);
        if (filter != QValueSpace::UnspecifiedLayer &&
            !(read->layerOptions() & filter)) {
            continue;
        }

        QAbstractValueSpaceLayer::Handle handle =
            read->item(QAbstractValueSpaceLayer::InvalidHandle, path);
        if (QAbstractValueSpaceLayer::InvalidHandle != handle) {
            list.append(qMakePair(read, handle));

            read->notifyInterest(handle, true);
        }
    }

    return list;
}

QValueSpaceSubscriberPrivate::QValueSpaceSubscriberPrivate(const QString &path, QValueSpace::LayerOptions filter)
    : path(qCanonicalPath(path))
    , readers(matchLayers(this->path, filter))
    , connections(0)
{
}

static LayerList matchLayers(const QString &path, const QUuid &uuid)
{
    LayerList list;

    QValueSpaceManager *manager = QValueSpaceManager::instance();
    if (!manager)
        return list;

    const QList<QAbstractValueSpaceLayer *> &readerList = manager->getLayers();

    for (int ii = 0; ii < readerList.count(); ++ii) {
        QAbstractValueSpaceLayer *read = readerList.at(ii);
        if (read->id() != uuid)
            continue;

        QAbstractValueSpaceLayer::Handle handle =
            read->item(QAbstractValueSpaceLayer::InvalidHandle, path);
        if (QAbstractValueSpaceLayer::InvalidHandle != handle) {
            list.append(qMakePair(read, handle));

            read->notifyInterest(handle, true);
        }
    }

    return list;
}

QValueSpaceSubscriberPrivate::QValueSpaceSubscriberPrivate(const QString &path, const QUuid &uuid)
    : path(qCanonicalPath(path))
    , readers(matchLayers(this->path, uuid))
    , connections(0)
{
}

QValueSpaceSubscriberPrivate::~QValueSpaceSubscriberPrivate()
{
    for (int ii = 0; ii < readers.count(); ++ii) {
        readers[ii].first->notifyInterest(readers[ii].second, false);
        readers[ii].first->removeHandle(readers[ii].second);
    }

    if (connections)
        delete connections;
}

void QValueSpaceSubscriberPrivate::connect(const QValueSpaceSubscriber *space) const
{
    QMutexLocker locker(&lock);

    if (!connections) {
        qRegisterMetaType<quintptr>("quintptr");
        connections = new QValueSpaceSubscriberPrivateProxy;
        connections->readers = readers;
        connections->connections.insert(space,1);
        QObject::connect(connections, SIGNAL(changed()),
                         space, SIGNAL(contentsChanged()));
        for (int ii = 0; ii < readers.count(); ++ii) {
            readers.at(ii).first->setProperty(readers.at(ii).second, QAbstractValueSpaceLayer::Publish);
            QObject::connect(readers.at(ii).first, SIGNAL(handleChanged(quintptr)), connections, SLOT(handleChanged(quintptr)));
        }
    } else if (!connections->connections.contains(space)) {
        connections->connections[space] = 1;

        QObject::connect(connections, SIGNAL(changed()), space, SIGNAL(contentsChanged()));
    } else {
        ++connections->connections[space];
    }
}

bool QValueSpaceSubscriberPrivate::disconnect(QValueSpaceSubscriber * space)
{
    QMutexLocker locker(&lock);

    if (connections) {
        QHash<const QValueSpaceSubscriber *, int>::Iterator iter = connections->connections.find(space);
        if (iter != connections->connections.end()) {
            --(*iter);
            if (!*iter) {
                QObject::disconnect(connections, SIGNAL(changed()), space, SIGNAL(contentsChanged()));
                connections->connections.erase(iter);
            }
            return true;
        }
    }
    return false;
}

/*!
    Constructs a QValueSpaceSubscriber with the specified \a parent that refers to the root path.

    The constructed Value Space subscriber will access all available layers.
*/
QValueSpaceSubscriber::QValueSpaceSubscriber(QObject *parent)
    : QObject(parent)
{
    d = new QValueSpaceSubscriberPrivate(QLatin1String("/"));
}

/*!
    Constructs a QValueSpaceSubscriber with the specified \a parent that refers to \a path.

    The constructed Value Space subscriber will access all available layers.
*/
QValueSpaceSubscriber::QValueSpaceSubscriber(const QString &path, QObject *parent)
    : QObject(parent)
{
    d = new QValueSpaceSubscriberPrivate(path);
}

/*!
    Constructs a QValueSpaceSubscriber with the specified \a parent that refers to \a path.  The
    \a filter parameter is used to limit which layers this QValueSpaceSubscriber will access.

    If a layer matching \a filter is not found, the constructed QValueSpaceSubscriber will be
    unconnected.

    \sa isConnected()
*/
QValueSpaceSubscriber::QValueSpaceSubscriber(QValueSpace::LayerOptions filter, const QString &path, QObject *parent)
    : QObject(parent)
{
    d = new QValueSpaceSubscriberPrivate(path, filter);
}

/*!
    Constructs a QValueSpaceSubscriber with the specified \a parent that refers to \a path.  This
    QValueSpaceSubscriber will only use the layer identified by \a uuid.

    Use of this constructor is not platform agnostic.  If possible use one of the constructors that
    take a QValueSpace::LayerOptions parameter instead.

    If a layer with a matching \a uuid is not found, the constructed QValueSpaceSubscriber will be
    unconnected.

    \sa QValueSpace, isConnected()
*/
QValueSpaceSubscriber::QValueSpaceSubscriber(const QUuid &uuid, const QString &path, QObject *parent)
    : QObject(parent)
{
    d = new QValueSpaceSubscriberPrivate(path, uuid);
}

/*!
    Destroys the QValueSpaceSubscriber.
*/
QValueSpaceSubscriber::~QValueSpaceSubscriber()
{
    d->disconnect(this);
}

/*!
    Sets the path to \a path.

    Calling this function causes the QValueSpaceSubscriber to disconnect and reconnect to the value
    space with the new \a path.

    Calling this function disconnects all signal/slot connections.
    \since 1.0
*/
void QValueSpaceSubscriber::setPath(const QString &path)
{
    if (this->path() == path)
        return;

    d->disconnect(this);

    disconnect();

    d = new QValueSpaceSubscriberPrivate(path);
}

/*!
    Sets the path to the same path as \a subscriber.

    Calling this function causes the QValueSpaceSubscriber to disconnect and reconnect to the value
    space with the specified \a path.

    Calling this function disconnects all signal/slot connections.
*/
void QValueSpaceSubscriber::setPath(QValueSpaceSubscriber *subscriber)
{
    d->disconnect(this);

    disconnect();

    d = subscriber->d;
}

QString QValueSpaceSubscriber::path() const
{
    return d->path;
}

/*!
    Changes the path to the absolute path if \a path starts with a '/'; otherwise changes to the
    sub path of the current path.
*/
void QValueSpaceSubscriber::cd(const QString &path)
{
    if (path.startsWith(QLatin1Char('/')))
        setPath(path);
    else
        setPath(this->path() + QLatin1Char('/') + path);
}

/*!
    Sets the path to parent of the current path.
*/
void QValueSpaceSubscriber::cdUp()
{
    if (path() == QLatin1String("/"))
        return;

    QString p(path());

    int index = p.lastIndexOf(QLatin1Char('/'));

    p.truncate(index);

    setPath(p);
}

/*!
    Returns true if this QValueSpaceSubscriber is connected to at least one available layer;
    otherwise returns false.  An unconnected QValueSpaceSubscriber is constructed if the filtering
    parameters passed to the constructor eliminate all available layers.
*/
bool QValueSpaceSubscriber::isConnected() const
{
    return !d->readers.isEmpty();
}

/*!
    Returns the value of the \a subPath under this subscriber path, or the value of this subscriber
    path if \a subPath is empty.  If the value does not exists \a def is returned.

    The following code shows how the subscriber path and \a subPath relate.

    \code
        QValueSpaceSubscriber base("/Settings");
        QValueSpaceSubscriber equiv("/Settings/Nokia/General/Mappings");

        // Is true
        equiv.value() == base.value("Nokia/General/Mappings");
    \endcode
*/
QVariant QValueSpaceSubscriber::value(const QString & subPath, const QVariant &def) const
{
    QVariant value;
    if (subPath.isEmpty()) {
        for (int ii = d->readers.count(); ii > 0; --ii) {
            if (d->readers[ii - 1].first->value(d->readers[ii - 1].second, &value))
                return value;
        }
    } else {
        const QString vpath(qCanonicalPath(subPath));
        for (int ii = d->readers.count(); ii > 0; --ii) {
            if (d->readers[ii - 1].first->value(d->readers[ii - 1].second, vpath, &value))
                return value;
        }
    }
    return def;
}

QVariant QValueSpaceSubscriber::valuex(const QVariant &def) const
{
    QMutexLocker locker(&d->lock);

    if (!d->connections || d->connections->connections.value(this) == 0) {
        locker.unlock();
        d->connect(this);
    }

    return value(QString(), def);
}

/*!
    \internal

    Registers for change notifications in response to connection to the contentsChanged()
    \a signal.
*/
void QValueSpaceSubscriber::connectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(contentsChanged())) == 0)
        d->connect(this);
    else
        QObject::connectNotify(signal);
}

/*!
    \internal

    Unregisters for change notifications in response to disconnection from the contentsChanged()
    \a signal.
*/
void QValueSpaceSubscriber::disconnectNotify(const char *signal)
{
    if (strcmp(signal, SIGNAL(contentsChanged())) == 0)
        d->disconnect(this);
    else
        QObject::disconnectNotify(signal);
}

/*!
    Returns a list of sub-paths under the current path.  For example, given a Value Space tree
    containing:

    \code
    /Settings/Nokia/Device
    /Settings/Nokia/Other
    /Settings/Qt
    /Device/Buttons
    \endcode

    \c { QValueSpaceSubscriber("/Settings").subPaths() } will return a list containing
    \c { { Nokia, Qt } } in no particular order.
*/
QStringList QValueSpaceSubscriber::subPaths() const
{
    QSet<QString> rv;
    for (int ii = 0; ii < d->readers.count(); ++ii)
        rv.unite(d->readers[ii].first->children(d->readers[ii].second));

    QStringList rvs;
    for (QSet<QString>::ConstIterator iter = rv.begin(); iter != rv.end(); ++iter)
        rvs.append(*iter);

    return rvs;
}

QT_END_NAMESPACE
