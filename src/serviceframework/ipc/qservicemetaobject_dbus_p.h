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

#ifndef QSERVICE_METAOBJECT_DBUS_H
#define QSERVICE_METAOBJECT_DBUS_H

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
