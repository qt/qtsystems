/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSERVICE_METAOBJECT_DBUS_H
#define QSERVICE_METAOBJECT_DBUS_H

#include "qserviceframeworkglobal.h"
#include "objectendpoint_dbus_p.h"
#include <QObject>

QT_BEGIN_NAMESPACE

class QServiceMetaObjectDBusPrivate;
class QServiceMetaObjectDBus : public QDBusAbstractAdaptor
{

public:
    QServiceMetaObjectDBus(QObject* service, bool signalsObject=false);
    virtual ~QServiceMetaObjectDBus();

    virtual const QMetaObject* metaObject() const;
    int qt_metacall(QMetaObject::Call c, int id, void **a);
    void *qt_metacast(const char* className);

    void activateMetaSignal(int id, const QVariantList& args);

protected:
    //void connectNotify(const char* signal);
    //void disconnectNotify(const char* signal);

private:
    const QMetaObject* dbusMetaObject(bool signalsObject) const;
    void connectMetaSignals(bool signalsObject);

    QServiceMetaObjectDBusPrivate* d;
    QVector<bool> localSignals;
};


struct QServiceUserTypeDBus
{
    QByteArray typeName;
    QByteArray variantBuffer;
};

QDBusArgument &operator<<(QDBusArgument &argument, const QServiceUserTypeDBus &myType);
const QDBusArgument &operator>>(const QDBusArgument &argument, QServiceUserTypeDBus &myType);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QServiceUserTypeDBus)

#endif //QSERVICE_METAOBJECT_DBUS_H
