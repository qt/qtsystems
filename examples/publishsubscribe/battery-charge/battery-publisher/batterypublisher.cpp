/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "batterypublisher.h"
#include "ui_batterypublisher.h"

#include <qvaluespacepublisher.h>

#include <QTimer>

BatteryPublisher::BatteryPublisher(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BatteryPublisher)
    , chargeTimer(0)
{
    ui->setupUi(this);

    publisher = new QValueSpacePublisher(QStringLiteral("/power/battery"));

    connect(ui->batteryCharge, SIGNAL(valueChanged(int)),
            this, SLOT(chargeChanged(int)));
    connect(ui->charging, SIGNAL(toggled(bool)),
            this, SLOT(chargingToggled(bool)));

    chargeChanged(ui->batteryCharge->value());
}

BatteryPublisher::~BatteryPublisher()
{
    delete ui;
    delete publisher;
}

void BatteryPublisher::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void BatteryPublisher::timerEvent(QTimerEvent *)
{
    int newCharge = ui->batteryCharge->value() + 1;
    ui->batteryCharge->setValue(newCharge);

    if (newCharge >= 100)
        ui->charging->setChecked(false);
}

void BatteryPublisher::chargeChanged(int newCharge)
{
    publisher->setValue(QStringLiteral("charge"), newCharge);
}

void BatteryPublisher::chargingToggled(bool charging)
{
    ui->batteryCharge->setEnabled(!charging);
    publisher->setValue(QStringLiteral("charging"), charging);

    if (charging)
        chargeTimer = startTimer(2000);
    else
        killTimer(chargeTimer);
}
