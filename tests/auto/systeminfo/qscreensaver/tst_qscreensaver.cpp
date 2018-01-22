/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/QString>
#include <QtTest/QtTest>

#include "qscreensaver.h"

QT_USE_NAMESPACE

class tst_QScreenSaver : public QObject
{
    Q_OBJECT

private slots:
    void tst_screensaver();
};

void tst_QScreenSaver::tst_screensaver()
{
    QScreenSaver screensaver;

    screensaver.setScreenSaverEnabled(true);
    screensaver.setScreenSaverEnabled(false);
    QVERIFY(screensaver.screenSaverEnabled() == false);
}

QTEST_MAIN(tst_QScreenSaver);
#include "tst_qscreensaver.moc"
