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

#include "qdeclarativevaluespacepublisher_p.h"
#include "qdeclarativevaluespacepublishermetaobject_p.h"

#include <QDeclarativeInfo>

QT_BEGIN_NAMESPACE

class QDeclarativeValueSpacePublisherQueueItem
{
public:
    QDeclarativeValueSpacePublisherQueueItem(const QString &subPath, const QVariant &value);

    QString subPath;
    QVariant value;
};

QDeclarativeValueSpacePublisherQueueItem::QDeclarativeValueSpacePublisherQueueItem(const QString &subPath, const QVariant &value) :
    subPath(subPath),
    value(value)
{
}

/*!
    \qmlclass ValueSpacePublisher QDeclarativeValueSpacePublisher

    \brief The ValueSpacePublisher element represents a path in the value space
           where keys can be published.

    ValueSpacePublishers are constructed with a set \a path which cannot be
    changed. If you need to publish within multiple different paths, you will
    need multiple ValueSpacePublishers.

    For the keys within the path chosen, if the key names to be published
    are alphanumeric, they may be accessed through dynamic properties by
    setting the \a keys list.

    Example:
    \code
    ValueSpacePublisher {
        id: battery
        path: "/power/battery"
        keys: ["charge", "charging"]
    }

    MouseArea {
        onClicked: {
            battery.charge = 50
            battery.charging = true
        }
    }
    \endcode

    Alternatively, for key names that can't be mapped to properties, or for
    key names shadowed by existing properties (like "value" or "path"), you
    can also access the \a value property of the Publisher itself.

    \code
    ValueSpacePublisher {
        id: nonalpha
        path: "/something/with a space/value"
    }

    MouseArea {
        onClicked: {
            nonalpha.value = "example"
        }
    }
    \endcode

    \ingroup qml-publishsubscribe

    \sa QValueSpacePublisher
    The ValueSpacePublisher element is part of the \bold{QtMobility.publishsubscribe 1.2} module.
*/

QDeclarativeValueSpacePublisher::QDeclarativeValueSpacePublisher(QObject *parent)
    : QObject(parent),
      d(new QDeclarativeValueSpacePublisherMetaObject(this)),
      m_hasSubscribers(false),
      m_complete(false),
      m_publisher(0),
      m_pathSet(false)
{
}

QDeclarativeValueSpacePublisher::~QDeclarativeValueSpacePublisher()
{
    if (m_publisher)
        delete m_publisher;
}

void QDeclarativeValueSpacePublisher::classBegin()
{
}

void QDeclarativeValueSpacePublisher::componentComplete()
{
    if (m_pathSet) {
        m_publisher = new QValueSpacePublisher(m_path, this);
        connect(m_publisher, SIGNAL(interestChanged(QString,bool)),
                this, SLOT(onInterestChanged(QString,bool)));
    }
    m_complete = true;
    doQueue();
}

void QDeclarativeValueSpacePublisher::queueChange(const QString &subPath, const QVariant &val)
{
    m_queue << QDeclarativeValueSpacePublisherQueueItem(subPath, val);
    if (m_publisher)
        doQueue();
}

void QDeclarativeValueSpacePublisher::doQueue()
{
    foreach (QDeclarativeValueSpacePublisherQueueItem i, m_queue) {
        m_publisher->setValue(i.subPath, i.value);
    }
    m_queue.clear();
}

/*!
    \qmlproperty string ValueSpacePublisher::path

    This property holds the base path of the publisher.
    This property is write-once -- after the first write, subsequent
    writes will be ignored and produce a warning.
  */
QString QDeclarativeValueSpacePublisher::path() const
{
    return m_path;
}

void QDeclarativeValueSpacePublisher::setPath(const QString &path)
{
    if (m_pathSet) {
        qmlInfo(this) << "Path has already been set";
        return;
    }

    m_path = path;
    startServer(true);
    m_pathSet = true;
    if (m_complete)
        componentComplete();
}

/*!
    \qmlproperty QVariant ValueSpacePublisher::value

    This property publishes a new value to the ValueSpace at the
    path given through the \a path property.
    This property is write only.
*/
void QDeclarativeValueSpacePublisher::setValue(const QVariant &val)
{
    queueChange("", val);
}

/*!
    \qmlproperty bool ValueSpacePublisher::server

    This property can be used to force the Publisher to start the ValueSpace
    server (if one is appropriate on the platform) before the \a path property
    has been set.

    This property is write only.
*/
void QDeclarativeValueSpacePublisher::startServer(const bool &really)
{
    if (really) {
        QValueSpace::initValueSpaceServer();
    }
}

/*!
    \qmlproperty bool ValueSpacePublisher::hasSubscribers

    This property is true if there are subscribers currently subscribed to
    the ValueSpace path being published by this Publisher.

    This property is read only.
*/
bool QDeclarativeValueSpacePublisher::hasSubscribers() const
{
    return m_hasSubscribers;
}

/*!
    \qmlproperty QStringList ValueSpacePublisher::keys

    Setting this property creates a set of dynamic properties allowing
    easy access to set the values of keys under this Publisher's path.
*/
void QDeclarativeValueSpacePublisher::setKeys(const QStringList &keys)
{
    foreach (QString key, keys) {
        d->addKey(key);
        m_keys.append(key);
    }
}

QVariant QDeclarativeValueSpacePublisher::dummyValue() const
{
    return QVariant();
}

bool QDeclarativeValueSpacePublisher::dummyServer() const
{
    return false;
}

QStringList QDeclarativeValueSpacePublisher::keys() const
{
    return m_keys;
}

void QDeclarativeValueSpacePublisher::onInterestChanged(QString /*path*/, bool state)
{
    m_hasSubscribers = state;
    // TODO: pass this on to key properties
}

QT_END_NAMESPACE
