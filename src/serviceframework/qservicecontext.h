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

#ifndef QSERVICECONTEXT_H
#define QSERVICECONTEXT_H

#include "qserviceframeworkglobal.h"
#include <QObject>
#include <QVariant>
#include <QString>


QT_BEGIN_HEADER

QTM_BEGIN_NAMESPACE

class Q_SERVICEFW_EXPORT QServiceContext : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString clientId READ clientId WRITE setClientId)
    Q_PROPERTY(QString clientName READ clientName WRITE setClientName)
public:
    enum ContextType {
        DisplayContext = 0,
        ScriptContext,
        UserDefined = 100
    };

    QServiceContext(QObject* parent = 0);
    virtual ~QServiceContext();

    virtual void notify( ContextType type, const QVariant& variant) = 0;


    QString clientId() const;
    void setClientId(const QString& clientId);

    QString clientName() const;
    void setClientName(const QString& name);

    QVariant clientData(const QString& key) const;
    void setClientData(const QString& key, const QVariant& value);
    void resetClientData();

private:
    QString m_id;
    QString m_displayName;

};

QTM_END_NAMESPACE

QT_END_HEADER
#endif //QSERVICECONTEXT_H

