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

#ifndef QVALUESPACE_P_H
#define QVALUESPACE_P_H

#include <qvaluespace.h>

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QValueSpacePublisher;

QString qCanonicalPath(const QString &path);

class QAbstractValueSpaceLayer : public QObject
{
    Q_OBJECT

public:
    typedef quintptr Handle;
    static const Handle InvalidHandle = ~Handle(0);

    enum Properties { Publish = 0x00000001 };

    virtual bool value(Handle handle, QVariant *data) = 0;
    virtual bool value(Handle handle, const QString &subPath, QVariant *data) = 0;
    virtual void removeHandle(Handle handle) = 0;
    virtual void setProperty(Handle handle, Properties properties) = 0;
    virtual Handle item(Handle parent, const QString &subPath) = 0;
    virtual QSet<QString> children(Handle handle) = 0;
    virtual QUuid id() = 0;
    virtual QValueSpace::LayerOptions layerOptions() const = 0;

    // QValueSpaceSubscriber functions
    virtual bool notifyInterest(Handle handle, bool interested) = 0;
    virtual bool supportsInterestNotification() const = 0;

    // QValueSpacePublisher functions
    virtual bool setValue(QValueSpacePublisher *creator, Handle handle, const QString &subPath, const QVariant &value) = 0;
    virtual bool removeSubTree(QValueSpacePublisher *creator, Handle handle) = 0;
    virtual bool removeValue(QValueSpacePublisher *creator, Handle handle, const QString &subPath) = 0;
    virtual void addWatch(QValueSpacePublisher *creator, Handle handle) = 0;
    virtual void removeWatches(QValueSpacePublisher *creator, Handle parent) = 0;
    virtual void sync() = 0;

protected:
    // QValueSpacePublisher functions
    void emitInterestChanged(QValueSpacePublisher *publisher, const QString &attribute, bool interested);

Q_SIGNALS:
    void handleChanged(quintptr);
};

QT_END_NAMESPACE

#endif // QVALUESPACE_P_H
