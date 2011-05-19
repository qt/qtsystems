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

#include <QtGui/qdesktopwidget.h>
#include <QtTest/QtTest>

#include "qdisplayinfo.h"

QT_USE_NAMESPACE

class tst_QDisplayInfo : public QObject
{
    Q_OBJECT

private slots:
    void tst_colorDepth();
    void tst_contrast();
    void tst_displayBrightness();
    void tst_dpiX();
    void tst_dpiY();
    void tst_physicalHeight();
    void tst_physicalWidth();
};

void tst_QDisplayInfo::tst_contrast()
{
    QDisplayInfo displayInfo;
    qDebug()<<QApplication::desktop()->screenGeometry();
    QVERIFY(-1 == displayInfo.contrast(-1));
    QVERIFY(-1 == displayInfo.contrast(QApplication::desktop()->screenCount()));
}

void tst_QDisplayInfo::tst_colorDepth()
{
    QDisplayInfo displayInfo;

    QVERIFY(-1 == displayInfo.colorDepth(-1));
    QVERIFY(-1 == displayInfo.colorDepth(QApplication::desktop()->screenCount()));
}

void tst_QDisplayInfo::tst_displayBrightness()
{
    QDisplayInfo displayInfo;

    QVERIFY(-1 == displayInfo.displayBrightness(-1));
    QVERIFY(-1 == displayInfo.displayBrightness(QApplication::desktop()->screenCount()));
}

void tst_QDisplayInfo::tst_dpiX()
{
    QDisplayInfo displayInfo;

    QVERIFY(-1 == displayInfo.dpiX(-1));
    QVERIFY(-1 == displayInfo.dpiX(QApplication::desktop()->screenCount()));
}

void tst_QDisplayInfo::tst_dpiY()
{
    QDisplayInfo displayInfo;

    QVERIFY(-1 == displayInfo.dpiY(-1));
    QVERIFY(-1 == displayInfo.dpiY(QApplication::desktop()->screenCount()));
}

void tst_QDisplayInfo::tst_physicalHeight()
{
    QDisplayInfo displayInfo;

    QVERIFY(-1 == displayInfo.physicalHeight(-1));
    QVERIFY(-1 == displayInfo.physicalHeight(QApplication::desktop()->screenCount()));
}

void tst_QDisplayInfo::tst_physicalWidth()
{
    QDisplayInfo displayInfo;

    QVERIFY(-1 == displayInfo.physicalWidth(-1));
    QVERIFY(-1 == displayInfo.physicalWidth(QApplication::desktop()->screenCount()));
}

QTEST_MAIN(tst_QDisplayInfo)
#include "tst_qdisplayinfo.moc"
