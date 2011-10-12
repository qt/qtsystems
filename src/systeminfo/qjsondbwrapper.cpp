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

#include "qjsondbwrapper_p.h"

#include <QTimer>
#include <QEventLoop>

#include <jsondb-client.h>

Q_USE_JSONDB_NAMESPACE

QT_BEGIN_NAMESPACE

static const int JSONDB_EXPIRATION_TIMER (500);

QJsonDbWrapper::QJsonDbWrapper(QObject *parent)
    : QObject(parent)
    , jsonclient(new JsonDbClient(this))
{
    connect(jsonclient, SIGNAL(error(int, int, const QString&)),
            this, SLOT(onError(int,int,QString)));
    connect(jsonclient,SIGNAL(response(int, QVariant)),
            this,SLOT(onResponse(int, QVariant)));
}

QJsonDbWrapper::~QJsonDbWrapper()
{
    if (jsonclient != 0)
        delete jsonclient;
}

QString QJsonDbWrapper::getUniqueDeviceID()
{
    return getProperty(QStringLiteral("SystemDeviceInfo"), QStringLiteral("uniqueDeviceId")).toString();
}

QVariant QJsonDbWrapper::getProperty(const QString objectType, const QString property)
{
    int reqId = 0;

    if (!jsonclient->isConnected())
        return QVariant();

    QEventLoop waitResponseLoop(this);
    connect(this, SIGNAL(responseAvailable()), &waitResponseLoop, SLOT(quit()));
    reqId = jsonclient->query(QString::fromAscii("[?_type=\"%1\"]").arg(objectType));
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), this, SLOT(onTimerExpired()));
    timer.start(JSONDB_EXPIRATION_TIMER);
    waitResponseLoop.exec();
    timer.stop();

    QVariantMap temp = responses.take(reqId).toMap();
    if (temp.isEmpty())
        return QVariant();

    QVariantList response = temp.value(QString::fromAscii("data")).toList();
    if (response.isEmpty())
        return QVariant();

    QVariant propertyValue = response[0].toMap().value(property);
    if ( !propertyValue.isValid())
        return QVariant();

    return propertyValue;
}

void QJsonDbWrapper::onResponse(int reqId,
                                    const QVariant& response)
{
        responses.insert(reqId, response);
        emit responseAvailable();
}

void QJsonDbWrapper::onError(int reqId, int param2, QString param3)
{
    Q_UNUSED (reqId)
    Q_UNUSED (param2)
    Q_UNUSED (param3)

    emit responseAvailable();
}

void QJsonDbWrapper::onTimerExpired()
{
    emit responseAvailable();
}

QT_END_NAMESPACE
