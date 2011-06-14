/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystemKit module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
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
