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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDECLARATIVEDISPLAYINFO_P_H
#define QDECLARATIVEDISPLAYINFO_P_H

#include <qdisplayinfo.h>

QT_BEGIN_NAMESPACE

class QDeclarativeDisplayInfo : public QObject
{
    Q_OBJECT

    Q_ENUMS(BacklightState)

public:
    enum BacklightState {
        BacklightUnknown = QDisplayInfo::BacklightUnknown,
        BacklightOff = QDisplayInfo::BacklightOff,
        BacklightDimmed = QDisplayInfo::BacklightDimmed,
        BacklightOn = QDisplayInfo::BacklightOn
    };

    QDeclarativeDisplayInfo(QObject *parent = 0);
    virtual ~QDeclarativeDisplayInfo();

    Q_INVOKABLE int brightness(int screen) const;
    Q_INVOKABLE int contrast(int screen) const;
    Q_INVOKABLE int backlightState(int screen) const;

    Q_INVOKABLE int colorDepth(int screen) const;
    Q_INVOKABLE int dpiX(int screen) const;
    Q_INVOKABLE int dpiY(int screen) const;
    Q_INVOKABLE int physicalHeight(int screen) const;
    Q_INVOKABLE int physicalWidth(int screen) const;

Q_SIGNALS:
    void backlightStateChanged(int screen, int state);

private Q_SLOTS:
    void _q_backlightStateChanged(int screen, QDisplayInfo::BacklightState state);

private:
    QDisplayInfo *displayInfo;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEDISPLAYINFO_P_H
