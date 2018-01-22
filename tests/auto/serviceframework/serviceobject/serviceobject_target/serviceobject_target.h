/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef SERVICEOBJECT_TARGET_H
#define SERVICEOBJECT_TARGET_H

#include <qremoteserviceregister.h>
#include <QObject>
#include <QtCore>

QT_USE_NAMESPACE

class ServiceObjectTarget : public QObject
{
    Q_OBJECT
public:
    ServiceObjectTarget(QObject *parent = 0);
    ~ServiceObjectTarget();

    Q_PROPERTY(int state READ state NOTIFY stateChanged)
    int state() const;

public slots:
    void slotWithArg(const QString& number);
    void slotWithoutArg();
    bool slotWithOk();
    void closeClientSockets();
    bool closeClientSocketsBlocking();

Q_SIGNALS:
    void stateChanged();

protected:
    void timerEvent(QTimerEvent* event);

private:
    friend class QRemoteServiceRegister;
    void setNewState();
    int timerId;
    int m_state;
};

#endif // SERVICEOBJECT_TARGET_H
