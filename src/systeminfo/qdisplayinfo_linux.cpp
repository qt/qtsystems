/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystemKit module of the Qt Toolkit.
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

#include "qdisplayinfo_linux_p.h"

#include <QtCore/qdir.h>
#include <QtGui/qapplication.h>
#include <QtGui/qdesktopwidget.h>
#include <QtGui/qpixmap.h>

#if defined(Q_WS_X11)
#include <QtGui/qx11info_x11.h>
#include <X11/extensions/Xrandr.h>
#endif // Q_WS_X11

QT_BEGIN_NAMESPACE

QDisplayInfoPrivate::QDisplayInfoPrivate(QDisplayInfo *parent)
    : q_ptr(parent)
{
}

int QDisplayInfoPrivate::colorDepth(int screen)
{
#if defined(Q_WS_X11)
    return QX11Info::appDepth(screen);
#else
    Q_UNUSED(screen)
#endif // Q_WS_X11

    return QPixmap::defaultDepth();
}

int QDisplayInfoPrivate::contrast(int screen)
{
    Q_UNUSED(screen)
    return -1;
}

int QDisplayInfoPrivate::displayBrightness(int screen)
{
    const QString sysfsPath(QString::fromAscii("/sys/class/backlight/"));
    const QStringList dirs = QDir(sysfsPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (dirs.size() < screen + 1)
        return -1;

    bool ok = false;
    int max = 0;
    int actual = 0;
    QFile brightness(dirs.at(screen) + QString::fromAscii("/max_brightness"));
    if (brightness.open(QIODevice::ReadOnly)) {
        max = brightness.readAll().simplified().toInt(&ok);
        if (!ok)
            return -1;
        brightness.close();

        brightness.setFileName(dirs.at(screen) + QString::fromAscii("/actual_brightness"));
        if (brightness.open(QIODevice::ReadOnly)) {
            actual = brightness.readAll().simplified().toInt(&ok);
            if (!ok)
                return -1;

            if (max != 0)
                return actual / max;
        }
    }

    return -1;
}

int QDisplayInfoPrivate::dpiX(int screen)
{
#if defined(Q_WS_X11)
    return QX11Info::appDpiX(screen);
#else
    Q_UNUSED(screen)
    return -1;
#endif
}

int QDisplayInfoPrivate::dpiY(int screen)
{
#if defined(Q_WS_X11)
    return QX11Info::appDpiY(screen);
#else
    Q_UNUSED(screen)
    return -1;
#endif
}

int QDisplayInfoPrivate::physicalHeight(int screen)
{
#if defined(Q_WS_X11)
    int height = -1;
    XRRScreenResources *sr = XRRGetScreenResources(QX11Info::display(), RootWindow(QX11Info::display(), screen));
    for (int i = 0; i < sr->noutput; ++i) {
        XRROutputInfo *output = XRRGetOutputInfo(QX11Info::display(), sr, sr->outputs[i]);
        if (output->crtc) {
           height = output->mm_height;
           XRRFreeOutputInfo(output);
           break;
        }
        XRRFreeOutputInfo(output);
    }
    XRRFreeScreenResources(sr);
    return height;
#else
    Q_UNUSED(screen)
    return -1;
#endif
}

int QDisplayInfoPrivate::physicalWidth(int screen)
{
#if defined(Q_WS_X11)
    int width = -1;
    XRRScreenResources *sr = XRRGetScreenResources(QX11Info::display(), RootWindow(QX11Info::display(), screen));
    for (int i = 0; i < sr->noutput; ++i) {
        XRROutputInfo *output = XRRGetOutputInfo(QX11Info::display(), sr, sr->outputs[i]);
        if (output->crtc) {
           width = output->mm_width;
           XRRFreeOutputInfo(output);
           break;
        }
        XRRFreeOutputInfo(output);
    }
    XRRFreeScreenResources(sr);
    return width;
#else
    Q_UNUSED(screen)
    return -1;
#endif
}

QDisplayInfo::BacklightState QDisplayInfoPrivate::backlightState(int screen)
{
    Q_UNUSED(screen);
    return QDisplayInfo::BacklightUnknown;
}

QT_END_NAMESPACE
