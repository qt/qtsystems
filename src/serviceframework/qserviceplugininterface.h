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

#ifndef QSERVICEPLUGININTERFACE_H
#define QSERVICEPLUGININTERFACE_H

#include "qserviceframeworkglobal.h"
#include <QtPlugin>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE


class QServiceInterfaceDescriptor;
class QServiceContext;
class QAbstractSecuritySession;

class Q_SERVICEFW_EXPORT QServicePluginInterface
{
public:
    QServicePluginInterface();
    virtual ~QServicePluginInterface();

    virtual QObject* createInstance(const QServiceInterfaceDescriptor& descriptor,
                                    QServiceContext* context,
                                    QAbstractSecuritySession* session) = 0;

    virtual void installService();
    virtual void uninstallService();
};

//moc doesn't understand QTM_PREPEND_NAMESPACE() macro. we have to be explicit
//Q_DECLARE_INTERFACE(QTM_PREPEND_NAMESPACE(QServicePluginInterface), "com.nokia.qt.QServicePluginInterface/1.0")
Q_DECLARE_INTERFACE(QServicePluginInterface, "com.nokia.qt.QServicePluginInterface/1.0")
QT_END_NAMESPACE

QT_END_HEADER
#endif
