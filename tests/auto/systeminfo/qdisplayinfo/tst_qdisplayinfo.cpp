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

#include <QtGui/qguiapplication.h>
#include <QtTest/QtTest>

#include "qdisplayinfo.h"

QT_USE_NAMESPACE

class tst_QDisplayInfo : public QObject
{
    Q_OBJECT

private slots:
    void tst_colorDepth();
    void tst_contrast();
    void tst_brightness();
    void tst_dpiX();
    void tst_dpiY();
    void tst_physicalHeight();
    void tst_physicalWidth();
};

void tst_QDisplayInfo::tst_brightness()
{
    QDisplayInfo displayInfo;

    QVERIFY(-1 == displayInfo.brightness(-1));
    QVERIFY(-1 == displayInfo.brightness(QGuiApplication::screens().size()));
}

void tst_QDisplayInfo::tst_contrast()
{
    QDisplayInfo displayInfo;

    QVERIFY(-1 == displayInfo.contrast(-1));
    QVERIFY(-1 == displayInfo.contrast(QGuiApplication::screens().size()));
}

void tst_QDisplayInfo::tst_colorDepth()
{
    QDisplayInfo displayInfo;

    QVERIFY(-1 == displayInfo.colorDepth(-1));
    QVERIFY(-1 == displayInfo.colorDepth(QGuiApplication::screens().size()));
}

void tst_QDisplayInfo::tst_dpiX()
{
    QDisplayInfo displayInfo;

    QVERIFY(-1 == displayInfo.dpiX(-1));
    QVERIFY(-1 == displayInfo.dpiX(QGuiApplication::screens().size()));
}

void tst_QDisplayInfo::tst_dpiY()
{
    QDisplayInfo displayInfo;

    QVERIFY(-1 == displayInfo.dpiY(-1));
    QVERIFY(-1 == displayInfo.dpiY(QGuiApplication::screens().size()));
}

void tst_QDisplayInfo::tst_physicalHeight()
{
    QDisplayInfo displayInfo;

    QVERIFY(-1 == displayInfo.physicalHeight(-1));
    QVERIFY(-1 == displayInfo.physicalHeight(QGuiApplication::screens().size()));
}

void tst_QDisplayInfo::tst_physicalWidth()
{
    QDisplayInfo displayInfo;

    QVERIFY(-1 == displayInfo.physicalWidth(-1));
    QVERIFY(-1 == displayInfo.physicalWidth(QGuiApplication::screens().size()));
}

QTEST_MAIN(tst_QDisplayInfo)
#include "tst_qdisplayinfo.moc"
