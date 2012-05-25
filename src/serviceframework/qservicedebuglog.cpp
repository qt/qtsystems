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

#include "qservicedebuglog_p.h"
#include <QDebug>
#include <QTime>

#include <QMutex>
#include <QMutexLocker>

#ifdef Q_OS_UNIX
#include <signal.h>
#endif

QT_BEGIN_NAMESPACE

#ifdef QT_SFW_IPC_DEBUG
static sighandler_t _qt_service_old_winch = 0;

void dump_op_log(int num) {

    qWarning() << "SFW OP LOG";
    QServiceDebugLog::instance()->dumpLog();

    if (_qt_service_old_winch)
        _qt_service_old_winch(num);
}

void dump_live_log(int num) {
    Q_UNUSED(num);

    QServiceDebugLog::instance()->setLiveDump(!QServiceDebugLog::instance()->liveDump());
    qWarning() << "SFW toggle live logging enabled:" << QServiceDebugLog::instance()->liveDump();
}
#endif

QServiceDebugLog::QServiceDebugLog()
    : logCount(0), length(500), autoDump(false), liveDumping(false)
{

#ifdef QT_SFW_IPC_DEBUG

    if (::getenv("SFW_LOG_STDOUT"))
        liveDumping = true;

    // Set to ignore winch once
    static QBasicAtomicInt atom_winch = Q_BASIC_ATOMIC_INITIALIZER(0);
    if (atom_winch.testAndSetRelaxed(0, 1)) {
        _qt_service_old_winch = ::signal(SIGWINCH, dump_op_log);
    }

    static QBasicAtomicInt atom_usr = Q_BASIC_ATOMIC_INITIALIZER(0);
    if (atom_usr.testAndSetRelaxed(0, 1)) {
        ::signal(SIGSYS, dump_live_log);
    }
#endif

}

QServiceDebugLog *QServiceDebugLog::instance()
{
    static QServiceDebugLog *dbg = 0;
    static QMutex m;
    QMutexLocker l(&m);

    if (!dbg)
        dbg = new QServiceDebugLog();
    return dbg;
}

bool QServiceDebugLog::liveDump() const
{
    return liveDumping;
}

void QServiceDebugLog::setLiveDump(bool enabled)
{
    liveDumping = enabled;
}

void QServiceDebugLog::appendToLog(const QString &message)
{
#ifdef QT_SFW_IPC_DEBUG
    if (liveDumping)
        qWarning() << message;

    logCount++;
    log.append(QTime::currentTime().toString(QStringLiteral("hh:mm:ss.zzz")) +
               QString::fromLatin1(" %1 ").arg(logCount) +
               message);
    if (autoDump && ((logCount%length) == 0))
        dumpLog();
    while (log.length() > length)
        log.removeFirst();
#else
    Q_UNUSED(message);
#endif
}

const QStringList QServiceDebugLog::fetchLog()
{
    return log;
}

void QServiceDebugLog::dumpLog()
{
    foreach (QString line, log)
        qDebug() << line;
}

QT_END_NAMESPACE
