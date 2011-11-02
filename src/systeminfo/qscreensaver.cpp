/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qscreensaver.h>

#if defined(Q_OS_LINUX)
#include "qscreensaver_linux_p.h"
#elif defined(Q_OS_WIN)
#  include "qscreensaver_win_p.h"
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
    \inmodule QtSystems
    \brief The QScreenSaver class provides various information of the screen saver.
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
    Returns if the screen saver is enabled.
*/
bool QScreenSaver::screenSaverEnabled() const
{
    return d_ptr->screenSaverEnabled();
}

/*!
    Set the screen saver to be \a enabled.
*/
void QScreenSaver::setScreenSaverEnabled(bool enabled)
{
    d_ptr->setScreenSaverEnabled(enabled);
}

QT_END_NAMESPACE
