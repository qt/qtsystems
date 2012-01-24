/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QVALUESPACESUBSCRIBER_H
#define QVALUESPACESUBSCRIBER_H

#include <qvaluespace.h>

#include <QtCore/qshareddata.h>
#include <QtCore/qvariant.h>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QValueSpaceSubscriberPrivate;

class Q_PUBLISHSUBSCRIBE_EXPORT QValueSpaceSubscriber : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString path READ path WRITE setPath)
    Q_PROPERTY(QVariant value READ valuex NOTIFY contentsChanged)

public:
    explicit QValueSpaceSubscriber(QObject *parent = 0);
    explicit QValueSpaceSubscriber(const QString &path, QObject *parent = 0);
    QValueSpaceSubscriber(QValueSpace::LayerOptions filter, const QString &path, QObject *parent = 0);
    QValueSpaceSubscriber(const QUuid &uuid, const QString &path, QObject *parent = 0);
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
    virtual void connectNotify(const char *signal);
    virtual void disconnectNotify(const char *signal);

private:
    QVariant valuex(const QVariant &def = QVariant()) const;

private:
    Q_DISABLE_COPY(QValueSpaceSubscriber)
    QExplicitlySharedDataPointer<QValueSpaceSubscriberPrivate> d;
};

QT_END_NAMESPACE
QT_END_HEADER

#endif // QVALUESPACESUBSCRIBER_H
