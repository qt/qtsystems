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

#include "qdisplayinfo.h"

#include <QtCore/qmetaobject.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qscreen.h>

#if defined(Q_OS_LINUX)
#  include "linux/qdisplayinfo_linux_p.h"
//#elif defined(Q_OS_WIN)
//#  include "qdisplayinfo_win_p.h"
#elif defined(Q_OS_MAC)
#  include "mac/qdisplayinfo_mac_p.h"
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
    \brief The QDisplayInfo class provides various information about the display.
    \ingroup systeminfo
*/

/*!
    \enum QDisplayInfo::BacklightState
    This enum describes the state of the backlight.

    \value BacklightUnknown     The state of the backlight is unknown.
    \value BacklightOff         Backlight is turned off.
    \value BacklightDimmed      Backlight has been dimmed.
    \value BacklightOn          Backlight is on.
*/

/*!
    \fn void QDisplayInfo::backlightStateChanged(int screen, QDisplayInfo::BacklightState state)

    This signal is emitted when the backlight state for \a screen has changed to \a state.
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
