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

#ifndef GCONFLAYER_P_H
#define GCONFLAYER_P_H

#if !defined(QT_NO_GCONFLAYER)

#include <qvaluespacepublisher.h>

#include "gconfitem_p.h"
#include "qvaluespace_p.h"

#include <QtCore/qmutex.h>
#include <QtCore/qset.h>

QT_BEGIN_NAMESPACE

class GConfLayer : public QAbstractValueSpaceLayer
{
    Q_OBJECT

public:
    GConfLayer();
    virtual ~GConfLayer();

    static GConfLayer *instance();

protected:
    bool value(Handle handle, QVariant *data);
    bool value(Handle handle, const QString &subPath, QVariant *data);
    void removeHandle(Handle handle);
    void setProperty(Handle handle, Properties properties);
    Handle item(Handle parent, const QString &subPath);
    QSet<QString> children(Handle handle);
    QUuid id();
    QValueSpace::LayerOptions layerOptions() const;

    // QValueSpaceSubscriber functions
    bool notifyInterest(Handle handle, bool interested);
    bool supportsInterestNotification() const;

    // QValueSpacePublisher functions
    void addWatch(QValueSpacePublisher *creator, Handle handle);
    bool removeSubTree(QValueSpacePublisher *creator, Handle handle);
    void removeWatches(QValueSpacePublisher *creator, Handle parent);
    bool removeValue(QValueSpacePublisher *creator, Handle handle, const QString &subPath);
    bool setValue(QValueSpacePublisher *creator, Handle handle, const QString &subPath, const QVariant &value);
    void sync();

private Q_SLOTS:
    void notifyChanged(const QString &key, const QVariant &value);

private:
    struct GConfHandle
    {
        GConfHandle(const QString &p)
            : path(p), refCount(1)
        {
        }

        QString path;
        unsigned int refCount;
    };

    QHash<QString, GConfHandle *> m_handles;

    GConfHandle *gConfHandle(Handle handle)
    {
        if (handle == InvalidHandle)
            return 0;

        GConfHandle *h = reinterpret_cast<GConfHandle *>(handle);
        if (m_handles.values().contains(h))
            return h;

        return 0;
    }

    //private methods not locking a mutex
    bool getValue(Handle handle, const QString &subPath, QVariant *data);
    void doRemoveHandle(Handle handle);
    Handle getItem(Handle parent, const QString &subPath);

private:
    QMap<QString, GConfItem *> m_monitoringItems;
    QMutex m_mutex;
    QSet<GConfHandle *> m_monitoringHandles;
};

QT_END_NAMESPACE

#endif // QT_NO_GCONFLAYER

#endif //GCONFLAYER_P_H
