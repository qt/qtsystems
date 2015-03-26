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

#include "qserviceclientcredentials_p.h"
#include "qremoteserviceregister_p.h"
#include "instancemanager_p.h"

#include <QCoreApplication>

QT_BEGIN_NAMESPACE

QRemoteServiceRegisterPrivate::QRemoteServiceRegisterPrivate(QObject* parent)
    : QObject(parent), iFilter(0),
      securityOptions(QRemoteServiceRegister::NoOptions),
      userIdentifier(0), userIdentifierSet(false),
      groupIdentifier(0), groupIdentifierSet(false)
{
    setQuitOnLastInstanceClosed(true);
}

QRemoteServiceRegisterPrivate::~QRemoteServiceRegisterPrivate()
{
}

//void QRemoteServiceRegisterPrivate::publishServices( const QString& ident)
//{
//  qWarning("QRemoteServiceregisterPrivate::publishServices has not been reimplemented");
//}
//
//void QRemoteServiceRegisterPrivate::processIncoming()
//{
//  qWarning("QRemoteServiceRegisterPrivate::processIncoming has not been reimplemented");
//}

bool QRemoteServiceRegisterPrivate::quitOnLastInstanceClosed() const
{
    return m_quit;
}

void QRemoteServiceRegisterPrivate::setQuitOnLastInstanceClosed(bool quit)
{
    m_quit = quit;
    if (m_quit) {
        connect(InstanceManager::instance(), SIGNAL(allInstancesClosed()), QCoreApplication::instance(), SLOT(quit()));
    }
    else {
        disconnect(InstanceManager::instance(), SIGNAL(allInstancesClosed()), QCoreApplication::instance(), SLOT(quit()));
    }
}

QRemoteServiceRegister::SecurityFilter QRemoteServiceRegisterPrivate::setSecurityFilter(QRemoteServiceRegister::SecurityFilter filter)
{
    QRemoteServiceRegister::SecurityFilter f;
    f = filter;
    iFilter = filter;
    return f;
}

void QRemoteServiceRegisterPrivate::setSecurityOptions(QRemoteServiceRegister::SecurityAccessOptions options)
{
    securityOptions = options;
}

QRemoteServiceRegister::SecurityFilter QRemoteServiceRegisterPrivate::getSecurityFilter()
{
    return iFilter;
}

QRemoteServiceRegister::SecurityAccessOptions QRemoteServiceRegisterPrivate::getSecurityOptions() const
{
    return securityOptions;
}

void QRemoteServiceRegisterPrivate::setBaseUserIdentifier(qintptr uid)
{
    userIdentifier = uid;
    userIdentifierSet = true;
}

qintptr QRemoteServiceRegisterPrivate::getBaseUserIdentifier() const
{
    return userIdentifier;
}

bool QRemoteServiceRegisterPrivate::isBaseUserIdentifierSet() const
{
    return userIdentifierSet;
}

void QRemoteServiceRegisterPrivate::setBaseGroupIdentifier(qintptr gid)
{
    groupIdentifier = gid;
    groupIdentifierSet = true;
}

qintptr QRemoteServiceRegisterPrivate::getBaseGroupIdentifier() const
{
    return groupIdentifier;
}

bool QRemoteServiceRegisterPrivate::isBaseGroupIdentifierSet() const
{
    return groupIdentifierSet;
}

QRemoteServiceRegisterPrivate *QRemoteServiceRegisterPrivate::constructPrivateObject(QService::Type serviceType, QObject *parent)
{
    QRemoteServiceRegisterPrivate *d = 0;
    switch (serviceType) {
    case QService::InterProcess:
        d = QRemoteServiceRegisterPrivate::constructPrivateObject(parent);
        break;
    default:
        qFatal("Cannot create a QRemoteServiceRegister with unknown service type %d", serviceType);
    }
    return d;
}

#include "moc_qremoteserviceregister_p.cpp"
QT_END_NAMESPACE
