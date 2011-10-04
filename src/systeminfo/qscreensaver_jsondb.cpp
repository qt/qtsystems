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

#include "qscreensaver_jsondb_p.h"

#include <QtCore/qtimer.h>
#include <QtCore/QDebug>

#include <mtcore/notion-client.h>

QT_BEGIN_NAMESPACE

static const int NOTION_DURATION (25);

QScreenSaverPrivate::QScreenSaverPrivate(QScreenSaver *parent)
    : QObject(parent)
    , q_ptr(parent)
    , notionclient(new NotionClient())
    , timer(0)
    , isScreenSaverEnabled(false)
{
}

QScreenSaverPrivate::~QScreenSaverPrivate()
{
    delete timer;
    delete notionclient;
}

bool QScreenSaverPrivate::screenSaverEnabled()
{
    if (isScreenSaverEnabled)
        return true;
    else
        return false;
}

void QScreenSaverPrivate::setScreenSaverEnabled(bool enabled)
{
    if (enabled) {
        notionclient->mediaPlaying(true, NOTION_DURATION);
        if (timer == 0) {
            timer = new QTimer(this);
            timer->setInterval((NOTION_DURATION - 2) * 1000);
            connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
        }
        if (!timer->isActive()) {
            timer->start();
        }
        isScreenSaverEnabled = true;

    } else {
        if (timer != 0) {
            if (timer->isActive()) {
                timer->stop();
            }
            delete timer;
            timer = 0;
        }
        notionclient->mediaPlaying(false);
        isScreenSaverEnabled = false;
    }
}

void QScreenSaverPrivate::onTimeout()
{
    notionclient->mediaPlaying(true, NOTION_DURATION);
    timer->start();
}

QT_END_NAMESPACE
