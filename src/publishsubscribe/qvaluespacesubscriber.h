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

#ifndef QVALUESPACESUBSCRIBER_H
#define QVALUESPACESUBSCRIBER_H

#include <QtPublishSubscribe/qvaluespace.h>

#include <QtCore/qshareddata.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QValueSpaceSubscriberPrivate;

class Q_PUBLISHSUBSCRIBE_EXPORT QValueSpaceSubscriber : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString path READ path WRITE setPath)
    Q_PROPERTY(QVariant value READ valuex NOTIFY contentsChanged)

public:
    explicit QValueSpaceSubscriber(QObject *parent = Q_NULLPTR);
    explicit QValueSpaceSubscriber(const QString &path, QObject *parent = Q_NULLPTR);
    explicit QValueSpaceSubscriber(QValueSpace::LayerOptions filter, const QString &path, QObject *parent = Q_NULLPTR);
    explicit QValueSpaceSubscriber(const QUuid &uuid, const QString &path, QObject *parent = Q_NULLPTR);
    virtual ~QValueSpaceSubscriber();

    bool isConnected() const;
    void cd(const QString &path);
    void cdUp();
    void setPath(const QString &path);
    void setPath(QValueSpaceSubscriber *subscriber);
    QString path() const;
    QStringList subPaths() const;
    QVariant value(const QString &subPath = QString(), const QVariant &def = QVariant()) const;

Q_SIGNALS:
    void contentsChanged();

protected:
    virtual void connectNotify(const QMetaMethod &signal);
    virtual void disconnectNotify(const QMetaMethod &signal);

private:
    QVariant valuex(const QVariant &def = QVariant()) const;

private:
    Q_DISABLE_COPY(QValueSpaceSubscriber)
    QExplicitlySharedDataPointer<QValueSpaceSubscriberPrivate> d;
};

QT_END_NAMESPACE

#endif // QVALUESPACESUBSCRIBER_H
