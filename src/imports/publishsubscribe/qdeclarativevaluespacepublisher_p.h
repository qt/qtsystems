/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystems module of the Qt Toolkit.
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

#ifndef QDECLARATIVEVALUESPACEPUBLISHER_H
#define QDECLARATIVEVALUESPACEPUBLISHER_H

#include <QHash>
#include <QStringList>

#include "qvaluespace.h"
#include "qvaluespacepublisher.h"
#include "qvaluespacesubscriber.h"

#include <QDeclarativeListProperty>
#include <QDeclarativeParserStatus>

QT_BEGIN_NAMESPACE

class QDeclarativeValueSpacePublisherMetaObject;
class QDeclarativeValueSpacePublisherQueueItem;

class QDeclarativeValueSpacePublisher : public QObject, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QDeclarativeParserStatus)

    Q_PROPERTY(QString path READ path WRITE setPath)
    Q_PROPERTY(bool hasSubscribers READ hasSubscribers NOTIFY subscribersChanged)
    Q_PROPERTY(QStringList keys READ keys WRITE setKeys)

    // these should be write-only
    // but MSVC can't cope with write-only Q_PROPERTYs?
    Q_PROPERTY(QVariant value READ dummyValue WRITE setValue)
    Q_PROPERTY(bool server READ dummyServer WRITE startServer)

public:
    QDeclarativeValueSpacePublisher(QObject *parent=0);
    ~QDeclarativeValueSpacePublisher();

    QString path() const;
    void setPath(const QString &path);

    void setValue(const QVariant &val);
    void startServer(const bool &really);
    bool dummyServer() const;

    QStringList keys() const;
    QVariant dummyValue() const;

    bool hasSubscribers() const;

    void setKeys(const QStringList &keys);

    void classBegin();
    void componentComplete();

signals:
    void subscribersChanged();

private:
    QDeclarativeValueSpacePublisherMetaObject *d;
    friend class QDeclarativeValueSpacePublisherMetaObject;

    void queueChange(const QString &subPath, const QVariant &val);
    void doQueue();

    QList<QDeclarativeValueSpacePublisherQueueItem> m_queue;
    bool m_hasSubscribers;
    bool m_complete;
    QValueSpacePublisher *m_publisher;
    QString m_path;
    QStringList m_keys;
    bool m_pathSet;

private slots:
    void onInterestChanged(QString path, bool state);

};

QT_END_NAMESPACE

#endif // QDECLARATIVEVALUESPACEPUBLISHER_H
