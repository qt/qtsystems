/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSERVICEINTERFACEDESCRIPTOR_P_H
#define QSERVICEINTERFACEDESCRIPTOR_P_H

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

#include "qserviceinterfacedescriptor.h"

#include <QString>
#include <QHash>

QT_BEGIN_NAMESPACE

#define SERVICE_INITIALIZED_ATTR    "INITIALIZED"

class QServiceInterfaceDescriptorPrivate
{
public:
    QServiceInterfaceDescriptorPrivate()
    {
        major = -1;
        minor = -1;
        scope = QService::UserScope;
    }

    bool operator==(const QServiceInterfaceDescriptorPrivate& other) const
    {
        if (major == other.major
                && minor == other.minor
                && interfaceName == other.interfaceName
                && serviceName == other.serviceName
                && attributes == other.attributes
                && customAttributes == other.customAttributes
                && scope == other.scope)
            return true;
        return false;
    }

    QServiceInterfaceDescriptorPrivate& operator=(const QServiceInterfaceDescriptorPrivate& other)
    {
        if (&other == this)
            return *this;

        serviceName = other.serviceName;
        interfaceName = other.interfaceName;
        minor = other.minor;
        major = other.major;
        attributes = other.attributes;
        customAttributes = other.customAttributes;
        scope = other.scope;

        return *this;
    }

    static QServiceInterfaceDescriptorPrivate *getPrivate(QServiceInterfaceDescriptor *descriptor)
    {
        return descriptor->d;
    }

    static const QServiceInterfaceDescriptorPrivate *getPrivate(const QServiceInterfaceDescriptor *descriptor)
    {
        return descriptor->d;
    }

    static void setPrivate(QServiceInterfaceDescriptor *descriptor, QServiceInterfaceDescriptorPrivate *p)
    {
        descriptor->d = p;
    }

    QString serviceName;
    QString interfaceName;
    QHash<QServiceInterfaceDescriptor::Attribute, QVariant> attributes;
    QHash<QString, QString> customAttributes;
    int major;
    int minor;
    QService::Scope scope;
};
QT_END_NAMESPACE

#endif //QSERVICEINTERFACEDESCRIPTOR_P_H
