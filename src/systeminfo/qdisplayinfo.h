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

#ifndef QDISPLAYINFO_H
#define QDISPLAYINFO_H

#include "qsysteminfo_p.h"
#include <QtCore/qobject.h>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QDisplayInfoPrivate;

class Q_SYSTEMINFO_EXPORT QDisplayInfo : public QObject
{
    Q_OBJECT

    Q_ENUMS(BacklightState)
    Q_ENUMS(Orientation)

public:
    enum BacklightState {
        BacklightUnknown = 0,
        BacklightOff,
        BacklightDimmed,
        BacklightOn
    };

    enum Orientation {
        OrientationUnknown = 0,
        Landscape,
        Portrait,
        InvertedLandscape,
        InvertedPortrait
    };

    QDisplayInfo(QObject *parent = 0);
    virtual ~QDisplayInfo();

    Q_INVOKABLE int brightness(int screen) const;
    Q_INVOKABLE int colorDepth(int screen) const;
    Q_INVOKABLE int contrast(int screen) const;
    Q_INVOKABLE int dpiX(int screen) const;
    Q_INVOKABLE int dpiY(int screen) const;
    Q_INVOKABLE int physicalHeight(int screen) const;
    Q_INVOKABLE int physicalWidth(int screen) const;
    Q_INVOKABLE QDisplayInfo::BacklightState backlightState(int screen) const;
    Q_INVOKABLE QDisplayInfo::Orientation orientation(int screen) const;

Q_SIGNALS:
    void orientationChanged(int screen, QDisplayInfo::Orientation orientation);

private:
    Q_DISABLE_COPY(QDisplayInfo)
    QDisplayInfoPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(QDisplayInfo)
};

QT_END_NAMESPACE
QT_END_HEADER

#endif // QDISPLAYINFO_H
