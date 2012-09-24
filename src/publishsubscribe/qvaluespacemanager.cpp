/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qvaluespacemanager_p.h"
#include "gconflayer_p.h"
#include "jsondblayer_p.h"
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
#if !defined(QT_NO_JSONDBLAYER)
            ptr->layers.append(JsonDbLayer::instance());
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
