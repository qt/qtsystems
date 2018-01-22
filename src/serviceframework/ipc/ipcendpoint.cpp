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

#include "ipcendpoint_p.h"

#include "qservicedebuglog_p.h"

#include <QEventLoop>
#include <QTimer>

QT_BEGIN_NAMESPACE
/*!
    QServiceIpcEndPoint
*/
QServiceIpcEndPoint::QServiceIpcEndPoint(QObject* parent)
    : QObject( parent )
{
}

QServiceIpcEndPoint::~QServiceIpcEndPoint()
{
    qServiceLog() << "event" << "delete"
                  << "class" << "QServiceIpcEndPoint"
                  << "name" << objectName();
    incoming.clear();
}

bool QServiceIpcEndPoint::packageAvailable() const
{
    return !incoming.isEmpty();
}

QServicePackage QServiceIpcEndPoint::nextPackage()
{
    if (!incoming.isEmpty())
        return incoming.dequeue();
    return QServicePackage();
}

void QServiceIpcEndPoint::writePackage(QServicePackage newPackage)
{
    flushPackage(newPackage);
}

void QServiceIpcEndPoint::getSecurityCredentials(QServiceClientCredentials &)
{
}

void QServiceIpcEndPoint::terminateConnection()
{
    qWarning() << "SFW Terminate connection called on base class, should be reimplemented to do something";
}

int QServiceIpcEndPoint::waitForData()
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(this, SIGNAL(packageReceived()), &loop, SLOT(quit()));

    timer.start(30000);
    loop.exec();
    return 0;
}

void QServiceIpcEndPoint::waitingDone()
{
    emit packageReceived();
}

#include "moc_ipcendpoint_p.cpp"
QT_END_NAMESPACE
