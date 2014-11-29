/****************************************************************************
**
** Copyright (C) 2016 Canonical, Ltd. and/or its subsidiary(-ies).
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

#include <QtTest/qtest.h>
#include <QSignalSpy>

#include "qinputinfo.h"

QT_USE_NAMESPACE

class tst_QInputDeviceInfo : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void tst_deviceMap();
    void tst_deviceFilter();
    void tst_signals();
};

void tst_QInputDeviceInfo::initTestCase()
{
    qRegisterMetaType<QInputDevice::InputType>();
    qRegisterMetaType<QInputDevice::InputTypeFlags>();
}

void tst_QInputDeviceInfo::tst_deviceMap()
{
    QInputInfoManager manager;
    QSignalSpy spy(&manager, SIGNAL(ready()));
    QTRY_COMPARE(spy.count(), 1);

    QMap <QString, QInputDevice *> map = manager.deviceMap();
    QVERIFY(map.count() > 0);
    QMapIterator<QString, QInputDevice *> i(map);
    while (i.hasNext()) {
        i.next();
        QVERIFY(!i.value()->name().isEmpty());
        QVERIFY(!i.value()->identifier().isEmpty());
        QVERIFY(!i.value()->properties().value("name").toString().isEmpty());
        QVERIFY(!i.value()->properties().value("identifier").toString().isEmpty());

        QVERIFY(i.value()->name() == i.value()->properties().value("name"));
        QVERIFY(i.value()->identifier() == i.value()->properties().value("identifier"));
        QVERIFY(!i.value()->types().testFlag(QInputDevice::UnknownType));
    }
}

void tst_QInputDeviceInfo::tst_deviceFilter()
{
    QInputInfoManager manager;
    QVERIFY(manager.filter().testFlag(QInputDevice::Keyboard)
            && manager.filter().testFlag(QInputDevice::Button)
            && manager.filter().testFlag(QInputDevice::Mouse)
            && manager.filter().testFlag(QInputDevice::TouchPad)
            && manager.filter().testFlag(QInputDevice::TouchScreen)
            && manager.filter().testFlag(QInputDevice::Switch));
    int mouseCount = manager.count(QInputDevice::Mouse);
    int keyboardCount = manager.count(QInputDevice::Keyboard);

    manager.setFilter(QInputDevice::Keyboard);
    QVERIFY(manager.filter().testFlag(QInputDevice::Keyboard));

    QVERIFY(!manager.filter().testFlag(QInputDevice::Button));
    QVERIFY(!manager.filter().testFlag(QInputDevice::Mouse));
    QVERIFY(!manager.filter().testFlag(QInputDevice::TouchPad));
    QVERIFY(!manager.filter().testFlag(QInputDevice::TouchScreen));
    QVERIFY(!manager.filter().testFlag(QInputDevice::Switch));

    QSignalSpy spy(&manager, SIGNAL(filterChanged(QInputDevice::InputTypeFlags)));
    QVERIFY(spy.isValid());
    manager.setFilter(QInputDevice::Mouse);
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(manager.filter(),QInputDevice::Mouse);
    QList<QVariant> arguments = spy.takeFirst();

    manager.setFilter(QInputDevice::Mouse | QInputDevice::Keyboard);
    QTRY_COMPARE(spy.count(), 1);

    QVERIFY(manager.count() == mouseCount + keyboardCount);
    QVERIFY(manager.count(QInputDevice::Mouse) == mouseCount);

    manager.setFilter(QInputDevice::TouchScreen);
    QCOMPARE(manager.count(QInputDevice::Mouse), 1);


}

void tst_QInputDeviceInfo::tst_signals()
{
    QInputInfoManager manager;

    QSignalSpy spy(&manager, SIGNAL(filterChanged(QInputDevice::InputTypeFlags)));
    QVERIFY(spy.isValid());

    QSignalSpy readyspy(&manager, SIGNAL(ready()));
    QVERIFY(readyspy.isValid());

    QSignalSpy deviceAddedSpy(&manager, SIGNAL(deviceAdded(QInputDevice *)));
    QVERIFY(deviceAddedSpy.isValid());

    QSignalSpy deviceRemovedSpy(&manager, SIGNAL(deviceRemoved(const QString &)));
    QVERIFY(deviceRemovedSpy.isValid());

    QSignalSpy countChangedSpy(&manager, SIGNAL(countChanged(int)));
    QVERIFY(countChangedSpy.isValid());
}


QTEST_MAIN(tst_QInputDeviceInfo)
#include "tst_qinputdeviceinfo.moc"
