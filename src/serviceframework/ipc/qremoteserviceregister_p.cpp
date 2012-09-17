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
