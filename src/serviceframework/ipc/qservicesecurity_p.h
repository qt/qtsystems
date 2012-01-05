/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef QSERVICE_SECURITY_H
#define QSERVICE_SECURITY_H

#include "qserviceframeworkglobal.h"
#include <QExplicitlySharedDataPointer>
#include <QSharedData>
#include <QUuid>
#include <QVariant>
#include "qremoteserviceregister.h"
#include "qsecuritypackage_p.h"
#include "qservicepackage_p.h"

QT_BEGIN_NAMESPACE

class QDataStream;
class QDebug;

class QServiceSecurityPrivate;
class Q_AUTOTEST_EXPORT QServiceSecurity : public QObject
{
    Q_OBJECT
public:
    enum Type {
        Service = 0,
        Client
    };

    QServiceSecurity(QServiceSecurity::Type m, QObject *parent = NULL);
    ~QServiceSecurity();

    Type getSessionType() const;

    void setAuthToken(const QUuid &token);
    void setAuthorizedClients(QHash<QString, QStringList> *authorized);

    Type isService() const;
    bool isTokenValid(const QSecurityPackage &pkg);

    bool isAuthorized(QServicePackage::Type type, const QRemoteServiceRegister::Entry &entry) const;

    bool waitingOnToken() const;

    QSecurityPackage getSecurityPackage() const;

#ifndef QT_NO_DATASTREAM
    friend QDataStream &operator<<(QDataStream &, const QServiceSecurity&);
    friend QDataStream &operator>>(QDataStream &, QServiceSecurity&);
#endif

private:
    Q_DISABLE_COPY(QServiceSecurity)

    QServiceSecurityPrivate *d;
};

class QServiceSecurityPrivate
{
public:
    QServiceSecurityPrivate()
        : type(QServiceSecurity::Client)
    {
    }

    QServiceSecurity::Type type;
    QUuid token;
    bool needToken;

    QHash<QString, QStringList> *authorizedClients;
};

QT_END_NAMESPACE

#endif
