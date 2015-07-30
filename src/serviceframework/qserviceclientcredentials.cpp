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
#include "qserviceclientcredentials.h"
#include <QDebug>

QT_BEGIN_NAMESPACE

/*!
    \class QServiceClientCredentials
    \brief The QServiceClientCredentials class holds credentials for the service client.

    This class is used when implementing service security. It allows the service to check
    the credentials of the client and then accept or reject the client.
*/

/*!
    \internal
*/
QServiceClientCredentials::QServiceClientCredentials()
    : d(new QServiceClientCredentialsPrivate)
{
    Q_ASSERT(d);
}

/*!
    \internal
*/
QServiceClientCredentials::QServiceClientCredentials(const QServiceClientCredentials &other)
    : d(other.d)
{
}

/*!
    \internal
*/
QServiceClientCredentials &QServiceClientCredentials::operator=(const QServiceClientCredentials &other)
{
    d = other.d;
    return *this;
}

/*!
    \internal
*/
QServiceClientCredentials::~QServiceClientCredentials()
{
}

/*!
    Returns the process identifier for the client. On Unix systems this is the PID
    of the client process.
*/
qintptr QServiceClientCredentials::getProcessIdentifier() const
{
    return d->pid;
}

/*!
    Returns the group identifier for the client. On Unix systems this is the GID
    of the client process.
*/
qintptr QServiceClientCredentials::getGroupIdentifier() const
{
    return d->gid;
}

/*!
    Returns the user identifier for the client. On Unix systems this is the UID
    of the client process.
*/
qintptr QServiceClientCredentials::getUserIdentifier() const
{
    return d->uid;
}

/*!
    Returns true if this object is valid.
*/
bool QServiceClientCredentials::isValid() const
{
    return d && (d->pid || d->uid || d->gid);
}

/*!
    Called by the service to accept or reject the client. Set \a isAccepted
    to true to accept the client.
*/
void QServiceClientCredentials::setClientAccepted(bool isAccepted)
{
    d->acceptedSet = true;
    d->accepted = isAccepted;
}

/*!
    Returns true if the client has been accepted, false otherwise.
*/
bool QServiceClientCredentials::isClientAccepted() const
{
    if (!d->acceptedSet) {
        qWarning() << "SFW credentials were queried, but service never called setClientAccepted(bool).  Returning default accepted.  This will break in the furture.";
    }
    return d->accepted;
}

QT_END_NAMESPACE

