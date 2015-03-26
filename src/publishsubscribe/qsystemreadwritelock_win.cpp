/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsystemreadwritelock_p.h"

#include <QSharedMemory>
#include <QSystemSemaphore>

///#define QSYSTEMREADWRITELOCK_DEBUG 1
#ifdef QSYSTEMREADWRITELOCK_DEBUG
#include <QDebug>
#endif

QT_BEGIN_NAMESPACE

class QSystemReadWriteLockPrivate
{
public:
    QSystemReadWriteLockPrivate(const QString &key, QSystemReadWriteLock::AccessMode mode);
    ~QSystemReadWriteLockPrivate();

    int &accessCount();
    unsigned int &waitingReaders();
    unsigned int &waitingWriters();

    QSystemReadWriteLock::SystemReadWriteLockError convertError(QSystemSemaphore::SystemSemaphoreError error);
    QSystemReadWriteLock::SystemReadWriteLockError convertError(QSharedMemory::SharedMemoryError error);

    QString m_key;

    QSharedMemory m_counts;
    QSystemSemaphore m_readerWait;
    QSystemSemaphore m_writerWait;
    bool m_isInitialized;

    QSystemReadWriteLock::SystemReadWriteLockError m_error;
    QString m_errorString;
};

QSystemReadWriteLockPrivate::QSystemReadWriteLockPrivate(const QString &key, QSystemReadWriteLock::AccessMode mode)
    :m_key(key), m_counts(key + QLatin1String("_counts")),
    m_readerWait(key + QLatin1String("_readerWait"), 0, (mode==QSystemReadWriteLock::Create)?QSystemSemaphore::Create:QSystemSemaphore::Open),
    m_writerWait(key + QLatin1String("_writerWait"), 0, (mode==QSystemReadWriteLock::Create)?QSystemSemaphore::Create:QSystemSemaphore::Open),
    m_isInitialized(false), m_error(QSystemReadWriteLock::NoError), m_errorString(QString())
{
    bool isAttached = false;
    int retries=5;
    for (int i=0; i < retries; ++i){
        if (m_counts.attach()) {
            isAttached = true;
            break;
        }
        else {
            if (m_counts.error() == QSharedMemory::NotFound) {
                if (m_counts.create(3 * sizeof(int))) {
                    isAttached = true;
                    break;
                }
            }
        }
    }

    if (!isAttached) {
        m_error = convertError(m_counts.error());
        m_errorString = QObject::tr("QSystemReadWriteLockPrivate::QSystemReadWriteLockPrivate: "
                                        "Unable to create/attach to shared memory");
        return;
    } else {
        if (mode == QSystemReadWriteLock::Create) {
            if(!m_counts.lock()) {
                m_error = convertError(m_counts.error());
                m_errorString = QObject::tr("QSystemReadWriteLockPrivate::QSystemReadWriteLockPrivate: "
                                                "Unable to initialize shared memory");
                return;
            }
            int * data = (int *)m_counts.data();
            ::memset(data, 0, 3 * sizeof(int));
            m_counts.unlock();
        }
        m_isInitialized = true;
    }
}

QSystemReadWriteLockPrivate::~QSystemReadWriteLockPrivate()
{
}

QSystemReadWriteLock::SystemReadWriteLockError QSystemReadWriteLockPrivate::convertError(QSystemSemaphore::SystemSemaphoreError error)
{
    switch(error) {
        case QSystemSemaphore::NoError:
            return QSystemReadWriteLock::NoError;
        case QSystemSemaphore::PermissionDenied:
            return QSystemReadWriteLock::PermissionDenied;
        case QSystemSemaphore::KeyError:
            return QSystemReadWriteLock::KeyError;
        case QSystemSemaphore::NotFound:
            return QSystemReadWriteLock::NotFound;
        case QSystemSemaphore::OutOfResources:
            return QSystemReadWriteLock::OutOfResources;
        case QSystemSemaphore::AlreadyExists:
        case QSystemSemaphore::UnknownError:
        default:
            return QSystemReadWriteLock::UnknownError;
    }
}

QSystemReadWriteLock::SystemReadWriteLockError QSystemReadWriteLockPrivate::convertError(QSharedMemory::SharedMemoryError error)
{
    switch(error){
        case QSharedMemory::NoError:
            return QSystemReadWriteLock::NoError;
        case QSharedMemory::PermissionDenied:
            return QSystemReadWriteLock::PermissionDenied;
        case QSharedMemory::KeyError:
            return QSystemReadWriteLock::KeyError;
        case QSharedMemory::NotFound:
            return QSystemReadWriteLock::NotFound;
        case QSharedMemory::LockError:
            return QSystemReadWriteLock::LockError;
        case QSharedMemory::UnknownError:
        default:
            return QSystemReadWriteLock::UnknownError;
    }
}

QSystemReadWriteLock::QSystemReadWriteLock(const QString &key, AccessMode mode)
    :d(new QSystemReadWriteLockPrivate(key,mode))
{
}

QSystemReadWriteLock::~QSystemReadWriteLock()
{
    Q_ASSERT(d);
    delete d;
    d = 0;
}

bool QSystemReadWriteLock::lockForRead()
{
#if defined QSYSTEMREADWRITELOCK_DEBUG
    qDebug() << "QSystemReadWriteLock::lockForRead() <start>\naccessCount ="
        << d->accessCount() << "\twaitingWriters=" << d->waitingWriters() << "\twaitingReaders=" << d->waitingReaders();
#endif
    if (!d->m_isInitialized) {
        d->m_error = QSystemReadWriteLock::FailedToInitialize;
        d->m_errorString = QObject::tr("QSystemReadWriteLock::lockForRead(): cannot peform operation, lock initialization had not been successful");
        return false;
    }

    if(!d->m_counts.lock()) {
        d->m_error = d->convertError(d->m_counts.error());
        d->m_errorString = QObject::tr("QSystemReadWriteLock::lockForRead(): cannot perform operation, locking of shared memory was unsuccessful");
        return false;
    }

    while(d->accessCount() < 0 || d->waitingWriters() > 0 ) {
        ++d->waitingReaders();
        d->m_counts.unlock();
        d->m_readerWait.acquire();
        d->m_counts.lock();
        --d->waitingReaders();
    }
    ++d->accessCount();
    Q_ASSERT_X(d->accessCount() > 0, "QSystemReadWriteLock::lockForRead()", "Overflow in lock counter");
#if defined QSYSTEMREADWRITELOCK_DEBUG
    qDebug() << "QSystemReadWriteLock::lockForRead() <end>\naccessCount ="
        << d->accessCount() << "\twaitingWriters=" << d->waitingWriters() << "\twaitingReaders=" << d->waitingReaders();
#endif

    d->m_error = QSystemReadWriteLock::NoError;
    d->m_errorString.clear();
    d->m_counts.unlock();
    return true;
}

bool QSystemReadWriteLock::lockForWrite()
{
#if defined QSYSTEMREADWRITELOCK_DEBUG
    qDebug() << "QSystemReadWriteLock::lockForWrite() <start>\naccessCount ="
        << d->accessCount() << "\twaitingWriters=" << d->waitingWriters() << "\twaitingReaders=" << d->waitingReaders();
#endif
    if (!d->m_isInitialized) {
        d->m_error = QSystemReadWriteLock::FailedToInitialize;
        d->m_errorString = QObject::tr("QSystemReadWriteLock::lockForWrite(): cannot peform operation, lock initialization had not been successful");
        return false;
    }

    if(!d->m_counts.lock()) {
        d->m_error = d->convertError(d->m_counts.error());
        d->m_errorString = QObject::tr("QSystemReadWriteLock::lockForwrite(): cannot perform operation, locking of shared memory was unsuccessful");
        return false;
    }

#if defined QSYSTEMREADWRITELOCK_DEBUG
    qDebug() << "QSystemReadWriteLock::lockForWrite() <start>\naccessCount ="
        << d->accessCount() << "\twaitingWriters=" << d->waitingWriters() << "\twaitingReaders=" << d->waitingReaders();
#endif
    while(d->accessCount() != 0) {
        ++d->waitingWriters();
        d->m_counts.unlock();
        d->m_writerWait.acquire();
        d->m_counts.lock();
        --d->waitingWriters();
    }
    --d->accessCount();
#if defined QSYSTEMREADWRITELOCK_DEBUG
    qDebug() << "QSystemReadWriteLock::lockForWrite() <end>\naccessCount ="
        << d->accessCount() << "\twaitingWriters=" << d->waitingWriters() << "\twaitingReaders=" << d->waitingReaders();
#endif

    Q_ASSERT_X(d->accessCount() < 0, "QSystemReadWriteLock::lockForWrite()", "Overflow in lock counter");

    d->m_error = QSystemReadWriteLock::NoError;
    d->m_errorString.clear();
    d->m_counts.unlock();
    return true;
}

void QSystemReadWriteLock::unlock()
{
    if (!d->m_isInitialized) {
        d->m_error = QSystemReadWriteLock::FailedToInitialize;
        d->m_errorString = QObject::tr("QSystemReadWriteLock::unlock(): cannot peform operation, lock initialization had not been successful");
        return;
    }

    if(!d->m_counts.lock()) {
        d->m_error = d->convertError(d->m_counts.error());
        d->m_errorString = QObject::tr("QSystemReadWriteLock::lockForwrite(): cannot perform operation, locking of shared memory was unsuccessful");
        return;
    }

    Q_ASSERT_X(d->accessCount() != 0, "QSystemReadWriteLock::unlock()", "Cannot unlock an unlocked lock");

    bool unlocked = false;
    if (d->accessCount() > 0) {
        unlocked = --d->accessCount() == 0;
    } else if (d->accessCount() < 0 && ++d->accessCount() == 0) {
        unlocked = true;
    }

    if (unlocked) {
        if (d->waitingWriters() > 0 ) {
            d->m_writerWait.release();
        } else if (d->waitingReaders()) {
            d->m_readerWait.release(d->waitingReaders());
        }
    }
#if defined QSYSTEMREADWRITELOCK_DEBUG
    qDebug() << "QSystemReadWriteLock::unlock() <end>\naccessCount ="
        << d->accessCount() << "\twaitingWriters=" << d->waitingWriters() << "\twaitingReaders=" << d->waitingReaders();
#endif
    d->m_error = QSystemReadWriteLock::NoError;
    d->m_errorString.clear();
    d->m_counts.unlock();
    return;
}

int& QSystemReadWriteLockPrivate::accessCount()
{
    return *((int *)m_counts.data());
}

unsigned int &QSystemReadWriteLockPrivate::waitingReaders()
{
    return *(((unsigned int*)m_counts.data())+1);
}

unsigned int &QSystemReadWriteLockPrivate::waitingWriters()
{
    return *(((unsigned int*)m_counts.data())+2);
}

QSystemReadWriteLock::SystemReadWriteLockError QSystemReadWriteLock::error() const
{
    return d->m_error;
}

QString QSystemReadWriteLock::errorString() const
{
    return d->m_errorString;
}

QString QSystemReadWriteLock::key() const
{
    return d->m_key;
}

QT_END_NAMESPACE
