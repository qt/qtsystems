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

#include "qdeclarativevaluespacepublisher_p.h"
#include "qdeclarativevaluespacepublishermetaobject_p.h"
#include <QtCore/qregexp.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ValueSpacePublisher
    \instantiates QDeclarativeValueSpacePublisher
    \inqmlmodule QtPublishSubscribe
    \ingroup qml-publishsubscribe

    \brief The ValueSpacePublisher allows application to publish values to Value Space.

    ValueSpacePublishers are constructed with a fixed \a path which cannot be changed. And you should
    set the \a path property before publishing any values. If you need to publish within multiple
    different paths, you will need multiple publishers.

    For the keys within the path chosen, if the key names to be published are alphanumeric, they can
    be accessed through dynamic properties by setting the \a keys list.

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

    Alternatively, for key names that can't be mapped to properties, or for key names shadowed by
    existing properties (like "value" or "path"), you can also access the \a value property of the
    publisher itself.

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
*/

QDeclarativeValueSpacePublisher::QDeclarativeValueSpacePublisher(QObject *parent)
    : QObject(parent)
    , metaObj(new QDeclarativeValueSpacePublisherMetaObject(this))
    , hasSubscriber(false)
    , publisher(0)
{
    d_ptr.data()->metaObject = metaObj;
}

QDeclarativeValueSpacePublisher::~QDeclarativeValueSpacePublisher()
{
    d_ptr.data()->metaObject = 0;
    delete metaObj;
}

/*!
    \qmlproperty string ValueSpacePublisher::path

    This property holds the base path of the publisher, and it should be written before publishing
    any data. Note it can only be written once, and further writing has no effects.
  */
QString QDeclarativeValueSpacePublisher::path() const
{
    if (publisher)
        return publisher->path();
    else
        return QString::null;
}

void QDeclarativeValueSpacePublisher::setPath(const QString &path)
{
    if (!publisher) {
        publisher = new QValueSpacePublisher(path);
        connect(publisher, SIGNAL(interestChanged(QString,bool)),
                this, SLOT(onInterestChanged(QString,bool)));
    }
}

/*!
    \qmlproperty QVariant ValueSpacePublisher::value

    This property publishes a new value to the ValueSpace at the
    path given through the \a path property.
    This property is write only.
*/
void QDeclarativeValueSpacePublisher::setValue(const QVariant &value)
{
    if (publisher)
        publisher->setValue(QString::null, value);
}

QVariant QDeclarativeValueSpacePublisher::dummyValue() const
{
    return QVariant();
}

/*!
    \qmlproperty bool ValueSpacePublisher::hasSubscribers

    This property is true if there are subscribers currently subscribed to
    the ValueSpace path being published by this Publisher.

    This property is read only.
*/
bool QDeclarativeValueSpacePublisher::hasSubscribers() const
{
    return hasSubscriber;
}

/*!
    \qmlproperty QStringList ValueSpacePublisher::keys

    Setting this property creates a set of dynamic properties allowing
    easy access to set the values of keys under this Publisher's path.
*/
void QDeclarativeValueSpacePublisher::setKeys(const QStringList &keys)
{
    foreach (const QString &key, keys) {
        if (key.contains(QRegExp(QString(QStringLiteral("[^a-zA-Z0-9]"))))
            || key == QString(QStringLiteral("value"))
            || key == QString(QStringLiteral("path"))
            || key == QString(QStringLiteral("keys"))
            || key == QString(QStringLiteral("hasSubscribers"))) {
            continue;
        }

        metaObj->createProperty(key.toUtf8().constData(), "QVariant");
    }
}

QStringList QDeclarativeValueSpacePublisher::keys() const
{
    QStringList keys;
    QList<QPair<QString, QVariant> > properties = metaObj->dynamicProperties.values();
    for (QList<QPair<QString, QVariant> >::const_iterator i = properties.constBegin(); i != properties.constEnd(); ++i)
        keys << (*i).first;
    return keys;
}

/*!
    \internal
*/
void QDeclarativeValueSpacePublisher::onInterestChanged(const QString &path, bool interested)
{
    Q_UNUSED(path)

    hasSubscriber = interested;
    emit subscribersChanged();
}

QT_END_NAMESPACE
