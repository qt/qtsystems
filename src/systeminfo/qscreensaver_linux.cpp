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

#include "qscreensaver_linux_p.h"

#if defined(Q_WS_X11)
#include <QtGui/qx11info_x11.h>
#include <X11/Xlib.h>
#endif // Q_WS_X11

QT_BEGIN_NAMESPACE

QScreenSaverPrivate::QScreenSaverPrivate(QScreenSaver *parent)
    : q_ptr(parent)
{
}

bool QScreenSaverPrivate::screenSaverEnabled()
{
#if defined(Q_WS_X11)
    int timeout = 0;
    int interval = 0;
    int preferBlanking = 0;
    int allowExposures = 0;
    XGetScreenSaver(QX11Info::display(), &timeout, &interval, &preferBlanking, &allowExposures);
    if (timeout > 0)
        return true;
    else
        return false;
#else
    return false;
#endif
}

void QScreenSaverPrivate::setScreenSaverEnabled(bool enabled)
{
#if defined(Q_WS_X11)
    int timeout = 0;
    int interval = 0;
    int preferBlanking = 0;
    int allowExposures = 0;
    XGetScreenSaver(QX11Info::display(), &timeout, &interval, &preferBlanking, &allowExposures);

    if (enabled && timeout > 0)
        XSetScreenSaver(QX11Info::display(), -1, interval, preferBlanking, allowExposures);
    else if (!enabled && timeout != 0)
        XSetScreenSaver(QX11Info::display(), 0, interval, preferBlanking, allowExposures);
#else
    Q_UNUSED(enabled)
#endif
}

QT_END_NAMESPACE
