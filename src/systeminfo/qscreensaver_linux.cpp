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

#include "qscreensaver_linux_p.h"

#if !defined(QT_NO_MTCORE)
#include <mtcore/notion-client.h>
#include <QtCore/qtimer.h>
#endif // QT_NO_MTCORE

#if !defined(QT_NO_X11)
#include <X11/Xlib.h>
#endif // QT_NO_X11

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_MTCORE)
const int QScreenSaverPrivate::notionDuration(25);
#endif // QT_NO_MTCORE

QScreenSaverPrivate::QScreenSaverPrivate(QScreenSaver *parent)
    : q_ptr(parent)
#if !defined(QT_NO_MTCORE)
    , notionClient(0)
    , timer(0)
    , isScreenSaverEnabled(true)
#endif // QT_NO_MTCORE
{
}

bool QScreenSaverPrivate::screenSaverEnabled()
{
#if !defined(QT_NO_MTCORE)
    return isScreenSaverEnabled;
#elif !defined(QT_NO_X11)
    int timeout = 0;
    int interval = 0;
    int preferBlanking = 0;
    int allowExposures = 0;
    Display *display = XOpenDisplay(0);
    XGetScreenSaver(display, &timeout, &interval, &preferBlanking, &allowExposures);
    XCloseDisplay(display);
    return (timeout > 0);
#else
    return false;
#endif
}

void QScreenSaverPrivate::setScreenSaverEnabled(bool enabled)
{
#if !defined(QT_NO_MTCORE)
    if (enabled != isScreenSaverEnabled) {
        if (enabled) {
            if (notionClient && timer) {
                notionClient->mediaPlaying(false);
                timer->stop();
            }
            isScreenSaverEnabled = true;
        } else {
            if (!notionClient)
                notionClient = new NotionClient(this);
            if (!timer) {
                timer = new QTimer(this);
                timer->setInterval((notionDuration - 2) * 1000);
                connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
            }
            if (!timer->isActive())
                timer->start();
            onTimeout();
            isScreenSaverEnabled = false;
        }
    }
#elif !defined(QT_NO_X11)
    int timeout = 0;
    int interval = 0;
    int preferBlanking = 0;
    int allowExposures = 0;
    Display *display = XOpenDisplay(0);
    XGetScreenSaver(display, &timeout, &interval, &preferBlanking, &allowExposures);

    if (enabled && timeout > 0)
        XSetScreenSaver(display, -1, interval, preferBlanking, allowExposures);
    else if (!enabled && timeout != 0)
        XSetScreenSaver(display, 0, interval, preferBlanking, allowExposures);

    XCloseDisplay(display);
#else
    Q_UNUSED(enabled)
#endif
}

#if !defined(QT_NO_MTCORE)
void QScreenSaverPrivate::onTimeout()
{
    notionClient->mediaPlaying(true, notionDuration);
}
#endif // QT_NO_MTCORE

QT_END_NAMESPACE
