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

#ifndef QSERVICE_PACKAGE_H
#define QSERVICE_PACKAGE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qserviceframeworkglobal.h"
#include <QExplicitlySharedDataPointer>
#include <QSharedData>
#include <QUuid>
#include <QVariant>
#include "qserviceclientcredentials_p.h"
#include "qremoteserviceregister.h"

QT_BEGIN_NAMESPACE

class QDataStream;
class QDebug;

class QServicePackagePrivate;
class Q_AUTOTEST_EXPORT QServicePackage
{
    Q_GADGET
public:
    QServicePackage();
    QServicePackage(const QServicePackage& other);
    QServicePackage& operator=(const QServicePackage& other);
    ~QServicePackage();

    enum Type {
        ObjectCreation = 0,
        MethodCall,
        PropertyCall
    };
    Q_ENUMS(Type)

    enum ResponseType {
        NotAResponse = 0,
        Success,
        Failed
    };
    Q_ENUMS(ResponseType)

    QServicePackage createResponse() const;

    bool isValid() const;

    QExplicitlySharedDataPointer<QServicePackagePrivate> d;

#ifndef QT_NO_DATASTREAM
    friend QDataStream &operator<<(QDataStream &, const QServicePackage&);
    friend QDataStream &operator>>(QDataStream &, QServicePackage&);
#endif
};

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &, const QServicePackage&);
QDataStream &operator>>(QDataStream &, QServicePackage&);
#endif

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug, const QServicePackage&);
#endif

class QServicePackagePrivate : public QSharedData
{
public:
    QServicePackagePrivate()
        :   packageType(QServicePackage::ObjectCreation),
            entry(QRemoteServiceRegister::Entry()), payload(QVariant()),
            messageId(QUuid()), instanceId(QUuid()), responseType(QServicePackage::NotAResponse)
    {
    }

    QServicePackage::Type packageType;
    QRemoteServiceRegister::Entry entry;
    QVariant payload;
    QUuid messageId;
    QUuid instanceId;
    QServicePackage::ResponseType responseType;

    void clean()
    {
        packageType = QServicePackage::ObjectCreation;
        messageId = QUuid();
        instanceId = QUuid();
        payload = QVariant();
        entry = QRemoteServiceRegister::Entry();
        responseType = QServicePackage::NotAResponse;
    }
};

QT_END_NAMESPACE

#endif //QSERVICE_PACKAGE_H
