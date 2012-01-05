/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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

#include <qdisplayinfo.h>

#include <QtGui/qguiapplication.h>
#include <QtGui/qscreen.h>
#include <QtGui/qplatformscreen_qpa.h>

#if defined(Q_OS_LINUX)
#  include "qdisplayinfo_linux_p.h"
#else
QT_BEGIN_NAMESPACE
class QDisplayInfoPrivate
{
public:
    QDisplayInfoPrivate(QDisplayInfo *) {}

    int brightness(int) { return -1; }
    int contrast(int) const { return -1; }
    QDisplayInfo::BacklightState backlightState(int) { return QDisplayInfo::BacklightUnknown; }
};
QT_END_NAMESPACE
#endif

QT_BEGIN_NAMESPACE

/*!
    \class QDisplayInfo
    \inmodule QtSystemInfo
    \brief The QDisplayInfo class provides various information of the display.
    \ingroup systeminfo
*/

/*!
    \enum QDisplayInfo::BacklightState
    This enum describes the state of the backlight.

    \value BacklightUnknown     The state of the backlight is unkown.
    \value BacklightOff         Backlight is turned off.
    \value BacklightDimmed      Backlight has been dimmed.
    \value BacklightOn          Backlight is on.
*/

/*!
    Constructs a QDisplayInfo object with the given \a parent.
*/
QDisplayInfo::QDisplayInfo(QObject *parent)
    : QObject(parent)
    , d_ptr(new QDisplayInfoPrivate(this))
{
}

/*!
    Destroys the object
*/
QDisplayInfo::~QDisplayInfo()
{
    delete d_ptr;
}

/*!
    Returns the display brightness of the given \a screen, in 0 - 100 scale. In case of error or
    the information is not available, -1 is returned.
*/
int QDisplayInfo::brightness(int screen) const
{
    if (screen < 0 || screen >= QGuiApplication::screens().size())
        return -1;
    return d_ptr->brightness(screen);
}

/*!
    Returns the contrast of the given \a screen, in 0 - 100 scale. -1 is returned if not available
    or on error.
*/
int QDisplayInfo::contrast(int screen) const
{
    if (screen < 0 || screen >= QGuiApplication::screens().size())
        return -1;
    return d_ptr->contrast(screen);
}

/*!
    Returns the backlight state of the given \a screen.
*/
QDisplayInfo::BacklightState QDisplayInfo::backlightState(int screen) const
{
    if (screen < 0 || screen >= QGuiApplication::screens().size())
        return QDisplayInfo::BacklightUnknown;
    return d_ptr->backlightState(screen);
}

// the following are merely wrapper of the QScreen API, and will be remove

static inline const QScreen *screenAt(int number)
{
    const QList<QScreen *> screens = QGuiApplication::screens();
    if (number >= 0 && number < screens.size())
        return screens.at(number);
    return 0;
}

/*!
    Returns the color depth of the given \a screen, in bits per pixel. -1 is returned if not
    available or on error.

    Please use QScreen::depth() instead.
*/
int QDisplayInfo::colorDepth(int screen) const
{
    if (const QScreen *qScreen = screenAt(screen))
        return qScreen->depth();
    return -1;
}

/*!
    Returns the horizontal resolution of the given \a screen in terms of the number of dots per inch.
    -1 is returned if not available or on error.

    Please use QScreen::logicalDotsPerInchX() instead.
*/
int QDisplayInfo::dpiX(int screen) const
{
    if (const QScreen *qScreen = screenAt(screen))
        return qRound(qScreen->logicalDotsPerInchX());
    return -1;
}

/*!
    Returns the vertical resolution of the given \a screen in terms of the number of dots per inch.
    -1 is returned if not available or on error.

    Please use QScreen::logicalDotsPerInchY() instead.
*/
int QDisplayInfo::dpiY(int screen) const
{
    if (const QScreen *qScreen = screenAt(screen))
        return qRound(qScreen->logicalDotsPerInchY());
    return -1;
}

/*!
    Returns the physical height of the \a screen in millimeters. -1 is returned if not available
    or on error.

    Please use QScreen::physicalSize().height() instead.
*/
int QDisplayInfo::physicalHeight(int screen) const
{
    if (const QScreen *qScreen = screenAt(screen))
        return qRound(qScreen->physicalSize().height());
    return -1;
}

/*!
    Returns the physical width of \a screen in millimeters. -1 is returned if not available or
    on error.

    Please use QScreen::physicalSize().width() instead.
*/
int QDisplayInfo::physicalWidth(int screen) const
{
    if (const QScreen *qScreen = screenAt(screen))
        return qRound(qScreen->physicalSize().width());
    return -1;
}

QT_END_NAMESPACE
