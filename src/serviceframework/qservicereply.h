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

#ifndef QSERVICEREPLY_H
#define QSERVICEREPLY_H

#include "qserviceframeworkglobal.h"
#include "qservicemanager.h"

#include <QObject>
#include <QMetaObject>

QT_BEGIN_NAMESPACE

class QServiceReplyPrivate;

class Q_SERVICEFW_EXPORT QServiceReplyBase : public QObject
{
    Q_OBJECT
public:
    explicit QServiceReplyBase(QObject *parent = Q_NULLPTR);
    virtual ~QServiceReplyBase();

    bool isFinished() const;
    bool isRunning() const;
    QServiceManager::Error error() const;

    QString request() const;
    void setRequest(const QString &request);

Q_SIGNALS:
    void started();
    void finished();
    void errorChanged();

protected:

private Q_SLOTS:
    void start();
    void finish();
    void setError(QServiceManager::Error error);
    virtual void setProxyObject(QObject *proxyObject) = 0;

private:
    friend class QServiceOperationProcessor;

    Q_DISABLE_COPY(QServiceReplyBase)

    QServiceReplyPrivate *d;
};

class Q_SERVICEFW_EXPORT QServiceReply : public QServiceReplyBase
{
    Q_OBJECT
    public:
    explicit QServiceReply(QObject *p = Q_NULLPTR)
        : QServiceReplyBase(p),
          m_proxyObject(Q_NULLPTR)
    {
        // nothing to do
    }

    ~QServiceReply()
    {
        // nothing to do
    }

    QObject *proxyObject() const
    {
        return m_proxyObject;
    }

    void setProxyObject(QObject *newProxyObject)
    {
        m_proxyObject = newProxyObject;
    }

private:
    QObject *m_proxyObject;
};

QT_END_NAMESPACE

#endif // QSERVICEREPLY_H
