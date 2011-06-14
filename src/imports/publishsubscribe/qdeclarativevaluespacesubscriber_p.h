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

#ifndef QDECLARATIVEVALUESPACESUBSCRIBER_P_H
#define QDECLARATIVEVALUESPACESUBSCRIBER_P_H

#include <QObject>
#include <QtCore/qstringlist.h>
#include "qvaluespacesubscriber.h"

QT_BEGIN_NAMESPACE

class QDeclarativeValueSpaceSubscriberPrivate;
class QDeclarativeValueSpaceSubscriber : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QVariant value READ value NOTIFY contentsChanged)
    Q_PROPERTY(bool connected READ isConnected)
    Q_PROPERTY(QStringList subPaths READ subPaths)

public:
    QDeclarativeValueSpaceSubscriber();
    ~QDeclarativeValueSpaceSubscriber();

    QString path() const;

    QVariant value(const QString &subPath = QString(), const QVariant &def=QVariant()) const;

    QStringList subPaths() const;

    bool isConnected() const;

public slots:
    void setPath(QString path);

signals:
    void contentsChanged();
    void pathChanged();

private:
    QDeclarativeValueSpaceSubscriberPrivate *d;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEVALUESPACESUBSCRIBER_P_H
