/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSERVICEREQUEST_P_H
#define QSERVICEREQUEST_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qserviceframeworkglobal.h"
#include "qservicemanager.h"
#include "qserviceinterfacedescriptor.h"
#include "qservicereply.h"

#include <QString>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QServiceRequest
{
public:
    enum Request {
        DescriptorRequest = 0,
        DefaultInterfaceRequest
    };


    QServiceRequest();
    explicit QServiceRequest(const QString &interfaceName);
    explicit QServiceRequest(const QServiceInterfaceDescriptor &descriptor);
    QServiceRequest(const QServiceRequest &other);
    ~QServiceRequest();
    QServiceRequest & operator=(const QServiceRequest &rhs);

    QServiceInterfaceDescriptor descriptor() const;
    void setDescriptor(const QServiceInterfaceDescriptor &descriptor);

    QString interfaceName() const;
    void setInterfaceName(const QString &interfaceName);

    QServiceReply *reply() const;
    void setReply(QServiceReply *reply);

    QService::Scope scope() const;
    void setScope(QService::Scope scope);

    Request requestType() const;

private:
    QString m_interfaceName;
    QServiceInterfaceDescriptor m_descriptor;
    QServiceReply *m_reply;
    QService::Scope m_scope;
    Request m_type;
};

QT_END_NAMESPACE

QT_END_HEADER

Q_DECLARE_METATYPE(QServiceRequest)

#endif // QSERVICEREQUEST_P_H
