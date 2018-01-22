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

#include "qvaluespacemanager_p.h"
#include "gconflayer_p.h"
#include "registrylayer_win_p.h"

#include <QtCore/qmutex.h>
#include <QtCore/qthread.h>
#include <QtCore/private/qcoreapplication_p.h>

QT_BEGIN_NAMESPACE

static QBasicAtomicPointer<QValueSpaceManager> valueSpaceManager_ptr;

QValueSpaceManager *QValueSpaceManager::instance()
{
    QValueSpaceManager *ptr = valueSpaceManager_ptr.loadAcquire();
    if (!ptr) {
        static QBasicMutex valueSpaceManager_mutex;
        QMutexLocker locker(&valueSpaceManager_mutex);
        if (!(ptr = valueSpaceManager_ptr.loadAcquire())) {
            ptr = new QValueSpaceManager;

            // make sure layers are installed in the priority order from high to low
#if defined(Q_OS_LINUX)
#if !defined(QT_NO_GCONFLAYER)
            ptr->layers.append(GConfLayer::instance());
#endif
#elif defined(Q_OS_WIN)
            ptr->layers.append(NonVolatileRegistryLayer::instance());
            ptr->layers.append(VolatileRegistryLayer::instance());
#endif

            // move all layers to a worker thread
            if (ptr->layers.size() > 0) {
                QThread *valueSpaceThread = new QThread();
                valueSpaceThread->setObjectName(QStringLiteral("valueSpaceThread"));
                valueSpaceThread->moveToThread(QCoreApplicationPrivate::mainThread());
                for (int i = 0; i < ptr->layers.size(); ++i)
                    ptr->layers[i]->moveToThread(valueSpaceThread);
                valueSpaceThread->start();
            }

            valueSpaceManager_ptr.storeRelease(ptr);
        }
    }
    return ptr;
}

QValueSpaceManager::QValueSpaceManager()
{
}

QList<QAbstractValueSpaceLayer *> const &QValueSpaceManager::getLayers()
{
    return layers;
}

QT_END_NAMESPACE
