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

#include "qservicerequest_p.h"

// Default constructor - required by meta-type system - note that we could just
// fall back on the one automatically created by the compiler but let's be
// explicit so no mistakes are made - eg, does m_reply get set to zero?
QServiceRequest::QServiceRequest()
    : m_reply(0), m_scope(QService::UserScope), m_type(DefaultInterfaceRequest)
{
    // nothing to do here
}

QServiceRequest::QServiceRequest(const QString &interfaceName)
    : m_interfaceName(interfaceName),
      m_reply(0),
      m_scope(QService::UserScope),
      m_type(DefaultInterfaceRequest)
{
    // nothing to do here
}

QServiceRequest::QServiceRequest(const QServiceInterfaceDescriptor &descriptor)
    : m_descriptor(descriptor),
      m_reply(0),
      m_scope(QService::UserScope),
      m_type(DescriptorRequest)
{
    // nothing to do here
}

// copy constructor - required by meta-type system - again let's be explicit
QServiceRequest::QServiceRequest(const QServiceRequest &other)
    : m_interfaceName(other.m_interfaceName),
      m_descriptor(other.m_descriptor),
      m_reply(other.m_reply),
      m_scope(other.m_scope),
      m_type(other.m_type)
{
}

// public destructor - required by meta-type system
QServiceRequest::~QServiceRequest()
{
    // nothing to do here
}

// assignment operator - not needed by meta-type but handy anyway
QServiceRequest & QServiceRequest::operator=(const QServiceRequest &rhs)
{
    if (&rhs == this)
        return *this;

    m_interfaceName = rhs.m_interfaceName;
    m_descriptor = rhs.m_descriptor;
    m_reply = rhs.m_reply;
    m_type = rhs.m_type;
    m_scope = rhs.m_scope;
    return *this;
}

QServiceInterfaceDescriptor QServiceRequest::descriptor() const
{
    return m_descriptor;
}

void QServiceRequest::setDescriptor(const QServiceInterfaceDescriptor &descriptor)
{
    m_type = DescriptorRequest;
    m_descriptor = descriptor;
}

void QServiceRequest::setInterfaceName(const QString &interfaceName)
{
    m_type = DefaultInterfaceRequest;
    m_interfaceName = interfaceName;
}

QString QServiceRequest::interfaceName() const
{
    return m_interfaceName;
}

QServiceReply *QServiceRequest::reply() const
{
    return m_reply;
}

void QServiceRequest::setReply(QServiceReply *reply)
{
    m_reply = reply;
}

QService::Scope QServiceRequest::scope() const
{
    return m_scope;
}

void QServiceRequest::setScope(QService::Scope scope)
{
    m_scope = scope;
}

QServiceRequest::Request QServiceRequest::requestType() const
{
    return m_type;
}
