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

#include "qsecuritypackage_p.h"
#include <QStringList>
#include <QDebug>

QT_BEGIN_NAMESPACE

QSecurityPackage::QSecurityPackage()
    : d(0)
{
}

QSecurityPackage::QSecurityPackage(const QSecurityPackage& other)
    : d(other.d)
{
}

QSecurityPackage& QSecurityPackage::operator=(const QSecurityPackage& other)
{
    d = other.d;
    return *this;
}

QSecurityPackage::~QSecurityPackage()
{
}

bool QSecurityPackage::isValid() const
{
    if (!d || d->token.isNull())
        return false;
    if (d->token.isNull())
        return false;

    return true;
}

QUuid QSecurityPackage::token() const
{
    if (!d)
        return QUuid();

    return d->token;
}

void QSecurityPackage::setToken(const QUuid &t)
{
    if (!d)
        d = new QSecurityPackagePrivate();

    d->token = t;
}

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &out, const QSecurityPackage& package)
{
    const quint32 magicNumber = 0x48ABFE23;
    out.setVersion(QDataStream::Qt_4_6);
    out << magicNumber;

    const qint8 valid = package.d ? 1 : 0;
    out << (qint8) valid;
    if (valid) {
        out << package.d->token;
    }

    return out;
}

QDataStream &operator>>(QDataStream &in, QSecurityPackage& package)
{
    const quint32 magicNumber = 0x48ABFE23;
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
            QSecurityPackagePrivate* priv = new QSecurityPackagePrivate();
            package.d = priv;
        } else {
            package.d.detach();
            package.d->clean();
        }
        in >> package.d->token;
    } else {
        if (package.d)
            package.d.reset();
    }

    return in;
}
#endif


#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QSecurityPackage& p)
{
    if (p.isValid()) {
        dbg.nospace() << "QSecurityPackage ";
        dbg.nospace() << p.d->token.toString();
    } else {
        dbg.nospace() << "QSecurityPackage(invalid)";
    }
    return dbg.space();
}
#endif




QT_END_NAMESPACE
