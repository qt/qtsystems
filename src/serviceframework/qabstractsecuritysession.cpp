/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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

#include "qabstractsecuritysession.h"

QTM_BEGIN_NAMESPACE

/*!
    \class QAbstractSecuritySession
    \inmodule QtServiceFramework
    \ingroup servicefw
    \brief The QAbstractSecuritySession class provides a generic mechanism to enable
    permission checks for services.
    \since 1.0

    QAbstractSecuritySession describes the abstract interface that security/permission
    engines must implement in order to provide capability related functionality.

    A QAbstractSecuritySession encapsulates the service client's capabilities. QServiceManager
    can match those capabilites with the capabilites required by a particular service.
    Service capabilites are declared via the services XML description.

    The use of a security session is not mandated by the service manager. If the client
    is passing a security session object QServiceManager ensures that the permissions
    are checked before the requested service is loaded and forwards the session to the
    service in case the service intends to implement additional checks. If no security
    session is passed to QServiceManager capability checks are not performed. Note that
    the security session is no substitute for platform security such as control over
    a processes ability to load arbitrary plug-ins.

    Since the service loader controls whether a security session is passed to the
    QServiceManager instance it is assumed that the calling context can be trusted. Possible
    use cases for a security session could be arbitrary Javascript applications which run
    within a trusted browser environment. The QAbstractSecuritySession interface would allow
    the browser to provide access to platform services while at the same time being able to
    ensure that certain Javascript application (depending on e.g their context, URL or signatures)
    can not access more sensitive system services.

    Framework clients with purely native code bases are likely to never have any security sessions.

    \sa QServiceManager, QServicePluginInterface
*/

/*!
    Constructs an abstract security session with the given \a parent.
*/
QAbstractSecuritySession::QAbstractSecuritySession(QObject* parent)
    : QObject(parent)
{
}

/*!
    Destroys the abstract security session.
*/
QAbstractSecuritySession::~QAbstractSecuritySession()
{
}

/*!
    \fn bool QAbstractSecuritySession::isAllowed(const QStringList& capabilities) = 0;

    Returns true if the security session has sufficient rights to access the required
    service \a capabilities.
    \since 1.0
*/

#include "moc_qabstractsecuritysession.cpp"

QTM_END_NAMESPACE
