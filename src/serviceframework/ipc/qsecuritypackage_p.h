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

#ifndef QSECURITY_PACKAGE_H
#define QSECURITY_PACKAGE_H

#include "qserviceframeworkglobal.h"
#include <QExplicitlySharedDataPointer>
#include <QSharedData>
#include <QUuid>
#include <QVariant>
#include "qremoteserviceregister.h"

QT_BEGIN_NAMESPACE

class QDataStream;
class QDebug;

class QSecurityPackagePrivate;
class Q_AUTOTEST_EXPORT QSecurityPackage
{
public:
    QSecurityPackage();
    QSecurityPackage(const QSecurityPackage& other);
    QSecurityPackage& operator=(const QSecurityPackage& other);
    ~QSecurityPackage();

    bool isValid() const;

    QUuid token() const;
    void setToken(const QUuid &t);

    QExplicitlySharedDataPointer<QSecurityPackagePrivate> d;

#ifndef QT_NO_DATASTREAM
    friend QDataStream &operator<<(QDataStream &, const QSecurityPackage&);
    friend QDataStream &operator>>(QDataStream &, QSecurityPackage&);
#endif

};

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &, const QSecurityPackage&);
QDataStream &operator>>(QDataStream &, QSecurityPackage&);
#endif

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug, const QSecurityPackage&);
#endif

class QSecurityPackagePrivate : public QSharedData
{
public:
    QSecurityPackagePrivate()
    {
    }

    QUuid token;

    virtual void clean()
    {
        token = QUuid();
    }
};

QT_END_NAMESPACE

#endif
