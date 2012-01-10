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

#include "qservicepackage_p.h"
#include <QDebug>

QT_BEGIN_NAMESPACE

QServicePackage::QServicePackage()
    : d(0)
{

}

QServicePackage::QServicePackage(const QServicePackage& other)
    : d(other.d)
{
}

QServicePackage& QServicePackage::operator=(const QServicePackage& other)
{
    d = other.d;
    return *this;
}

QServicePackage::~QServicePackage()
{
}

bool QServicePackage::isValid() const
{
    return d;
}

QServicePackage QServicePackage::createResponse() const
{
    Q_ASSERT(d->responseType == QServicePackage::NotAResponse);
    QServicePackage response;
    response.d = new QServicePackagePrivate();
    response.d->packageType = d->packageType;
    response.d->messageId = d->messageId;
    response.d->instanceId = d->instanceId;
    response.d->responseType = QServicePackage::Failed;

    return response;
}

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &out, const QServicePackage& package)
{
    const quint32 magicNumber = 0x78AFAFB;
    out.setVersion(QDataStream::Qt_4_6);
    out << magicNumber;

    const qint8 valid = package.d ? 1 : 0;
    out << (qint8) valid;
    if (valid) {
        out << (qint8) package.d->packageType;
        out << (qint8) package.d->responseType;
        out << package.d->messageId;
        out << package.d->instanceId;
        out << package.d->entry;
        out << package.d->payload;
    }

    return out;
}

QDataStream &operator>>(QDataStream &in, QServicePackage& package)
{
    const quint32 magicNumber = 0x78AFAFB;
    in.setVersion(QDataStream::Qt_4_6);

    quint32 storedMagicNumber;
    in >> storedMagicNumber;
    if (storedMagicNumber != magicNumber) {
        qWarning() << "Datastream doesn't provide serialized QServiceFilter";
        return in;
    }

    qint8 valid;
    in >> valid;
    if (valid) {
        if (!package.d) {
            QServicePackagePrivate* priv = new QServicePackagePrivate();
            package.d = priv;
        } else {
            package.d.detach();
            package.d->clean();
        }
        qint8 data;
        in >> data;
        package.d->packageType = (QServicePackage::Type) data;
        in >> data;
        package.d->responseType = (QServicePackage::ResponseType) data;
        in >> package.d->messageId;
        in >> package.d->instanceId;
        in >> package.d->entry;
        in >> package.d->payload;
    } else {
        if (package.d)
            package.d.reset();
    }

    return in;
}
#endif


#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QServicePackage& p)
{
    if (p.isValid()) {
        QString type;
        switch (p.d->packageType) {
            case QServicePackage::ObjectCreation:
                type = QLatin1String("ObjectCreation");
                break;
            case QServicePackage::MethodCall:
                type = QLatin1String("MethodCall");
                break;
            case QServicePackage::PropertyCall:
                type = QLatin1String("PropertyCall");
                break;
            default:
                break;
        }
        dbg.nospace() << "QServicePackage ";
        dbg.nospace() << type << " " << p.d->responseType ; dbg.space();
        dbg.nospace() << p.d->messageId.toString(); dbg.space();
        dbg.nospace() << p.d->entry;dbg.space();
    } else {
        dbg.nospace() << "QServicePackage(invalid)";
    }
    return dbg.space();
}
#endif



QT_END_NAMESPACE
