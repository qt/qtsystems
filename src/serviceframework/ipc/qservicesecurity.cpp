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

#include "qservicesecurity_p.h"
#include <QStringList>
#include <QDebug>

QT_BEGIN_NAMESPACE

QServiceSecurity::QServiceSecurity(Type m, QObject *parent)
    : QObject(parent),
      d(new QServiceSecurityPrivate)
{
    d->type = m;
    if (d->type == Service)
        d->needToken = true;
    else
        d->needToken = false;
}

QServiceSecurity::~QServiceSecurity()
{
}

void QServiceSecurity::setAuthToken(const QUuid &token)
{
    d->token = token;
}

void QServiceSecurity::setAuthorizedClients(QHash<QString, QStringList> *authorized)
{
    d->authorizedClients = authorized;
}

QServiceSecurity::Type QServiceSecurity::getSessionType() const
{
    return d->type;
}

bool QServiceSecurity::isTokenValid(const QSecurityPackage &pkg)
{
    if (d->type == Client) {
        qWarning() << Q_FUNC_INFO << "called for a client, services are always authorized";
        return true;
    }

    if (!pkg.isValid()) {
        qWarning() << Q_FUNC_INFO << "Security Package from client is invalid! Auth is going to fail";
        return false;
    }

    if (d->authorizedClients->contains(pkg.token().toString())) {
        d->token = pkg.token();
        d->needToken = false;
        return true;
    }

    qWarning() << Q_FUNC_INFO << "We don't have the client token stored locally";

    return false;

}

bool QServiceSecurity::waitingOnToken() const
{
    return d->needToken;
}

bool QServiceSecurity::isAuthorized(QServicePackage::Type type, const QRemoteServiceRegister::Entry &entry) const
{
    if (d->needToken)
        return false;

    if (type == QServicePackage::ObjectCreation) {
        if (d->authorizedClients->contains(d->token.toString())) {
            QStringList ifaces = d->authorizedClients->value(d->token.toString());
            foreach (QString iface, ifaces) {
                if (iface == entry.interfaceName()) {
                    return true;
                }
            }
            if (ifaces.count() == 1 && ifaces.at(0) == QLatin1String("*all")) {
                qWarning() << Q_FUNC_INFO << "authed based on tempoary security backdoor, giving access by *all";
                return true;
            }
        }
    }
    else {
        return true;
    }
    qDebug() << Q_FUNC_INFO << "FAILED AUTH" << "no mathing interfaces to" << entry.interfaceName();
    return false;
}

QSecurityPackage QServiceSecurity::getSecurityPackage() const
{
    QSecurityPackage pkg;
    if (d->type == Client) {
        pkg.setToken(d->token);
    }
    return pkg;
}

QT_END_NAMESPACE
