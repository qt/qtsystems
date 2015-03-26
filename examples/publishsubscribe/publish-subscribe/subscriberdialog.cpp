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

#include "subscriberdialog.h"
#include "ui_subscriberdialog.h"

#include <qpublishsubscribeglobal.h>
#include <qvaluespacesubscriber.h>

#include <QTableWidget>
#include <QListWidget>
#include <QDesktopWidget>

#include <QPushButton>
#include <QSizePolicy>

#include <QDebug>

SubscriberDialog::SubscriberDialog(QWidget *parent)
:   QMainWindow(parent), ui(new Ui::SubscriberDialog), subscriber(0), tableWidget(0), listWidget(0)
{
    ui->setupUi(this);

    QPushButton *switchButton =
        ui->buttonBox->addButton(tr("Switch"), QDialogButtonBox::ActionRole);
    connect(switchButton, SIGNAL(clicked()), this, SIGNAL(switchRequested()));

    QDesktopWidget desktopWidget;
    if (desktopWidget.availableGeometry().width() < 400) {
        // Screen is too small to fit a table widget without scrolling, use a list widget instead.
        listWidget = new QListWidget;
        listWidget->setAlternatingRowColors(true);
        ui->verticalLayout->insertWidget(2, listWidget);
    } else {
        tableWidget = new QTableWidget;
        QStringList headerLabels;
        headerLabels << tr("Key") << tr("Value") << tr("Type");
        tableWidget->setColumnCount(3);
        tableWidget->setHorizontalHeaderLabels(headerLabels);
        tableWidget->horizontalHeader()->setStretchLastSection(true);
        tableWidget->verticalHeader()->setVisible(false);

        ui->verticalLayout->insertWidget(2, tableWidget);
    }
    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(changeSubscriberPath()));
    changeSubscriberPath();

    // if the default path does not exist reset it to /
    QVariant value = subscriber->value();
    if (!subscriber->value().isValid() && subscriber->subPaths().isEmpty()) {
        ui->basePath->setText(QStringLiteral("/"));
        changeSubscriberPath();
    }
}

SubscriberDialog::~SubscriberDialog()
{
    delete ui;
}

void SubscriberDialog::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

//! [0]
void SubscriberDialog::changeSubscriberPath()
{
    if (listWidget)
        listWidget->clear();
    else if (tableWidget)
        tableWidget->clearContents();

    if (!subscriber)
        subscriber = new QValueSpaceSubscriber(ui->basePath->text(), this);
    else
        subscriber->setPath(ui->basePath->text());

    connect(subscriber, SIGNAL(contentsChanged()), this, SLOT(subscriberChanged()));

    subscriberChanged();
}
//! [0]

//! [1]
void SubscriberDialog::subscriberChanged()
{
    QStringList subPaths = subscriber->subPaths();

    if (listWidget) {
        listWidget->clear();
    } else if (tableWidget) {
        tableWidget->clearContents();
        tableWidget->setRowCount(subPaths.count());
    }

    for (int i = 0; i < subPaths.count(); ++i) {
        QVariant v = subscriber->value(subPaths.at(i));

        if (listWidget) {
            const QString itemTemplate(QStringLiteral("%1 (%2)\n%3"));

            QListWidgetItem *item =
                new QListWidgetItem(itemTemplate.arg(subPaths.at(i), v.typeName(), v.toString()));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            listWidget->addItem(item);
        } else if (tableWidget) {
            QTableWidgetItem *pathItem = new QTableWidgetItem(subPaths.at(i));
            pathItem->setFlags(pathItem->flags() & ~Qt::ItemIsEditable);
            QTableWidgetItem *valueItem = new QTableWidgetItem(v.toString());
            valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
            QTableWidgetItem *typeItem = new QTableWidgetItem(v.typeName());
            typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);

            tableWidget->setItem(i, 0, pathItem);
            tableWidget->setItem(i, 1, valueItem);
            tableWidget->setItem(i, 2, typeItem);
        }
    }
}
//! [1]
