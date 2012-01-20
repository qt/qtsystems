/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qvaluespacesubscriber.h>

#include "qvaluespace_p.h"

#include <QtCore/qmutex.h>

QT_BEGIN_NAMESPACE

class QValueSpaceSubscriberPrivateProxy : public QObject
{
    Q_OBJECT

public:
    QHash<const QValueSpaceSubscriber *, int> connections;
    QList<QPair<QAbstractValueSpaceLayer *, QAbstractValueSpaceLayer::Handle> > readers;

public Q_SLOTS:
    void handleChanged(quintptr handle);

Q_SIGNALS:
    void changed();
};

typedef QList<QPair<QAbstractValueSpaceLayer *, QAbstractValueSpaceLayer::Handle> > LayerList;

class QValueSpaceSubscriberPrivate : public QSharedData
{
public:
    QValueSpaceSubscriberPrivate(const QString &path, QValueSpace::LayerOptions filter = QValueSpace::UnspecifiedLayer);
    QValueSpaceSubscriberPrivate(const QString &path, const QUuid &uuid);
    ~QValueSpaceSubscriberPrivate();

    bool disconnect(QValueSpaceSubscriber * space);
    void connect(const QValueSpaceSubscriber *space) const;

    const QString path;
    const LayerList readers;

    mutable QMutex lock;
    mutable QValueSpaceSubscriberPrivateProxy *connections;
};

QT_END_NAMESPACE
