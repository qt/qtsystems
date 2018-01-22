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

#include "qscreensaver.h"

#if defined(Q_OS_LINUX)
#  if defined(QT_UNITY8)
#    include "linux/qscreensaver_mir_p.h"
#  else
#    include "linux/qscreensaver_linux_p.h"
#  endif
#elif defined(Q_OS_WIN)
#  include "windows/qscreensaver_win_p.h"
#elif defined(Q_OS_MAC)
#  include "mac/qscreensaver_mac_p.h"
#else
QT_BEGIN_NAMESPACE
class QScreenSaverPrivate
{
public:
    QScreenSaverPrivate(QScreenSaver *) {}

    bool screenSaverEnabled() { return false; }
    void setScreenSaverEnabled(bool) {}
};
QT_END_NAMESPACE
#endif

QT_BEGIN_NAMESPACE

/*!
    \class QScreenSaver
    \inmodule QtSystemInfo
    \brief The QScreenSaver class provides various information about the screen saver.

    \ingroup systeminfo
*/

/*!
    Constructs a QScreenSaver object with the given \a parent.
*/
QScreenSaver::QScreenSaver(QObject *parent)
    : QObject(parent)
    , d_ptr(new QScreenSaverPrivate(this))
{
}

/*!
    Destroys the object
*/
QScreenSaver::~QScreenSaver()
{
    delete d_ptr;
}

/*!
    \property QScreenSaver::screenSaverEnabled
    \brief The state of the screen saver.

    Returns if the screen saver is enabled.
*/
bool QScreenSaver::screenSaverEnabled() const
{
    return d_ptr->screenSaverEnabled();
}

/*!
    Sets the screen saver to be \a enabled.
*/
void QScreenSaver::setScreenSaverEnabled(bool enabled)
{
    d_ptr->setScreenSaverEnabled(enabled);
}

QT_END_NAMESPACE
