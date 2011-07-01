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

#include "qdeclarativevaluespacepublishermetaobject_p.h"
#include "qdeclarativevaluespacepublisher_p.h"
#include <QVariant>

QT_BEGIN_NAMESPACE

QDeclarativeValueSpacePublisherMetaObject::QDeclarativeValueSpacePublisherMetaObject(QObject * /*obj*/)
//    : QDeclarativeOpenMetaObject(obj)
{
}

void QDeclarativeValueSpacePublisherMetaObject::addKey(const QString &key, bool /*interest*/)
{
    if (key.contains(QRegExp("[^a-zA-Z0-9]")))
        return;
    if (key == "value" || key == "path" || key == "keys" || key == "hasSubscribers")
        return;

    QString keysubs = key;
    keysubs.append("HasSubscribers");

//    int pid = createProperty(key.toLatin1().constData(), "QVariant");
//    int sid = createProperty(keysubs.toLatin1().constData(), "bool");
//    m_keyProperties.insert(pid, key);
//    m_subsProperties.insert(sid, interest);
}

void QDeclarativeValueSpacePublisherMetaObject::getValue(int id, void **a)
{
    if (m_subsProperties.contains(id)) {
        bool subs = m_subsProperties.value(id);
        *reinterpret_cast<bool*>(a[0]) = subs;
    }
}

void QDeclarativeValueSpacePublisherMetaObject::setValue(int id, void ** /*a*/)
{
    if (m_keyProperties.contains(id)) {
        //QString key = m_keyProperties.value(id);
        //QVariant &v = *reinterpret_cast<QVariant*>(a[0]);

//        QDeclarativeValueSpacePublisher *pub = qobject_cast<QDeclarativeValueSpacePublisher*>(object());
//        pub->queueChange(key, v);
    }
}

QT_END_NAMESPACE
