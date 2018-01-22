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
#ifndef QSERVICEDEBUGLOG_P_H
#define QSERVICEDEBUGLOG_P_H

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

#include <QString>
#include <QStringList>
#include <QBuffer>
#include <QDataStream>
#include <QSharedPointer>
#include <QMutex>
#include <QVector>

QT_BEGIN_NAMESPACE

class QServiceDebugMessage
{
public:
    enum DataType {
        Int32Type = 1,
        FloatType = 2,
        StringType = 3
    };

    QServiceDebugMessage();
    ~QServiceDebugMessage();

#ifdef QT_SFW_IPC_DEBUG
    QBuffer *buffer;
    QDataStream ds;
#endif
};

class QServiceDebugValue;
class QServiceDebugKey;
class QUdpSocket;

class QServiceDebugLog
{
public:
    QServiceDebugLog();

    static QServiceDebugLog* instance();

    QServiceDebugValue operator<<(const char *key);
    void logMessage(QServiceDebugMessage *msg);

private:
    QList<QBuffer *> queue;
    QVector<QUdpSocket *> sockets;
    void makeSockets();
    QMutex socketLock;
};

inline QServiceDebugLog &qServiceLog()
{
    return (*QServiceDebugLog::instance());
}

class QServiceDebugKey
{
public:
#ifdef QT_SFW_IPC_DEBUG
    inline QServiceDebugKey(const QSharedPointer<QServiceDebugMessage> &ptr)
        : ptr(ptr) {}

#else
    inline QServiceDebugKey() {}
#endif
    QServiceDebugValue operator<<(const char *key);
private:
#ifdef QT_SFW_IPC_DEBUG
    QSharedPointer<QServiceDebugMessage> ptr;
#endif
};

class QServiceDebugValue
{
public:
#ifdef QT_SFW_IPC_DEBUG
    inline QServiceDebugValue(const QSharedPointer<QServiceDebugMessage> &ptr)
        : ptr(ptr) {}
#else
    inline QServiceDebugValue() {}
#endif
    QServiceDebugKey operator<<(const qint32 &val);
    QServiceDebugKey operator<<(const float &val);
    QServiceDebugKey operator<<(const QString &val);
    QServiceDebugKey operator<<(const char *val);
private:
#ifdef QT_SFW_IPC_DEBUG
    QSharedPointer<QServiceDebugMessage> ptr;
#endif
};

inline QServiceDebugValue QServiceDebugKey::operator<<(const char *key)
{
#ifdef QT_SFW_IPC_DEBUG
    ptr->ds.writeBytes(key, ::strlen(key));
    return QServiceDebugValue(ptr);
#else
    Q_UNUSED(key)
    return QServiceDebugValue();
#endif
}

inline QServiceDebugKey QServiceDebugValue::operator<<(const qint32 &val)
{
#ifdef QT_SFW_IPC_DEBUG
    ptr->ds << (qint8)QServiceDebugMessage::Int32Type;
    ptr->ds << val;
    return QServiceDebugKey(ptr);
#else
    Q_UNUSED(val)
    return QServiceDebugKey();
#endif
}

inline QServiceDebugKey QServiceDebugValue::operator<<(const float &val)
{
#ifdef QT_SFW_IPC_DEBUG
    ptr->ds << (qint8)QServiceDebugMessage::FloatType;
    ptr->ds << val;
    return QServiceDebugKey(ptr);
#else
    Q_UNUSED(val)
    return QServiceDebugKey();
#endif
}

inline QServiceDebugKey QServiceDebugValue::operator<<(const QString &val)
{
#ifdef QT_SFW_IPC_DEBUG
    ptr->ds << (qint8)QServiceDebugMessage::StringType;
    QByteArray ba = val.toLatin1();
    ptr->ds.writeBytes(ba.constData(), ba.size());
    return QServiceDebugKey(ptr);
#else
    Q_UNUSED(val)
    return QServiceDebugKey();
#endif
}

inline QServiceDebugKey QServiceDebugValue::operator<<(const char *val)
{
#ifdef QT_SFW_IPC_DEBUG
    ptr->ds << (qint8)QServiceDebugMessage::StringType;
    ptr->ds.writeBytes(val, ::strlen(val));
    return QServiceDebugKey(ptr);
#else
    Q_UNUSED(val)
    return QServiceDebugKey();
#endif
}

inline QServiceDebugValue QServiceDebugLog::operator<<(const char *key)
{
#ifdef QT_SFW_IPC_DEBUG
    QSharedPointer<QServiceDebugMessage> msg(new QServiceDebugMessage());
    return (QServiceDebugKey(msg) << key);
#else
    Q_UNUSED(key)
    return QServiceDebugValue();
#endif
}

QT_END_NAMESPACE

#endif // QSERVICEDEBUGLOG_P_H
