/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativedisplayinfo_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype DisplayInfo
    \instantiates QDisplayInfo
    \inqmlmodule QtSystemInfo
    \ingroup qml-systeminfo
    \brief The DisplayInfo element provides information about the display of the device.
*/

/*!
    \internal
*/
QDeclarativeDisplayInfo::QDeclarativeDisplayInfo(QObject *parent)
    : QObject(parent), displayInfo(new QDisplayInfo(this))
{
    connect(displayInfo, SIGNAL(backlightStateChanged(int,QDisplayInfo::BacklightState)),
            this, SLOT(_q_backlightStateChanged(int,QDisplayInfo::BacklightState)));
}

/*!
    \internal
 */
QDeclarativeDisplayInfo::~QDeclarativeDisplayInfo()
{
}

/*!
    \qmlmethod int DisplayInfo::brightness(int screen)

    Returns the display brightness of the given \a screen, in 0 - 100 scale. In case of error or
    the information is not available, -1 is returned.
 */
int QDeclarativeDisplayInfo::brightness(int screen) const
{
    return displayInfo->brightness(screen);
}

/*!
    \qmlmethod int DisplayInfo::colorDepth(int screen)

    Returns the color depth of the given \a screen, in bits per pixel. -1 is returned if not
    available or on error.
*/
int QDeclarativeDisplayInfo::colorDepth(int screen) const
{
    return displayInfo->colorDepth(screen);
}

/*!
    \qmlmethod int DisplayInfo::contrast(int screen)

    Returns the contrast of the given \a screen, in 0 - 100 scale. -1 is returned if not available
    or on error.
*/
int QDeclarativeDisplayInfo::contrast(int screen) const
{
    return displayInfo->contrast(screen);
}

/*!
    \qmlmethod int DisplayInfo::dpiX(int screen)

    Returns the horizontal resolution of the given \a screen in terms of the number of dots per inch.
    -1 is returned if not available or on error.
*/
int QDeclarativeDisplayInfo::dpiX(int screen) const
{
    return displayInfo->dpiX(screen);
}

/*!
    \qmlmethod int DisplayInfo::dpiY(int screen)

    Returns the vertical resolution of the given \a screen in terms of the number of dots per inch.
    -1 is returned if not available or on error.
*/
int QDeclarativeDisplayInfo::dpiY(int screen) const
{
    return displayInfo->dpiY(screen);
}

/*!
    \qmlmethod int DisplayInfo::physicalHeight(int screen)

    Returns the physical height of the \a screen in millimeters. -1 is returned if not available
    or on error.
*/
int QDeclarativeDisplayInfo::physicalHeight(int screen) const
{
    return displayInfo->physicalHeight(screen);
}

/*!
    \qmlmethod int DisplayInfo::physicalWidth(int screen)

    Returns the physical width of \a screen in millimeters. -1 is returned if not available or
    on error.
*/
int QDeclarativeDisplayInfo::physicalWidth(int screen) const
{
    return displayInfo->physicalWidth(screen);
}

/*!
    \qmlmethod BacklightState DisplayInfo::backlightState(int screen)

    Returns the backlight state of the given \a screen. Possible values are:
    \list
    \li DisplayInfo.BacklightUnknown  - The state of the backlight is unkown.
    \li DisplayInfo.BacklightOff      - Backlight is turned off.
    \li DisplayInfo.BacklightDimmed   - Backlight has been dimmed.
    \li DisplayInfo.BacklightOn       - Backlight is on.
    \endlist
*/
int QDeclarativeDisplayInfo::backlightState(int screen) const
{
    return displayInfo->backlightState(screen);
}

/*!
    \qmlsignal DisplayInfo::onBacklightStateChanged(int screen, BacklightState state)

    This handler is called whenever the state of \a screen has been changed to \a state.
 */
void QDeclarativeDisplayInfo::_q_backlightStateChanged(int screen, QDisplayInfo::BacklightState state)
{
    emit backlightStateChanged(screen, static_cast<BacklightState>(state));
}

QT_END_NAMESPACE
