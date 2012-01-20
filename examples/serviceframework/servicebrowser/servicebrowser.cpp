/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: http://www.qt-project.org/
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
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>
#include <QtWidgets>

#include <qservicemanager.h>
#include <qserviceinterfacedescriptor.h>

#include "servicebrowser.h"

Q_DECLARE_METATYPE(QServiceInterfaceDescriptor)

ServiceBrowser::ServiceBrowser(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    serviceManager = new QServiceManager(this);
    systemManager = new QServiceManager(QService::SystemScope);

    registerExampleServices();

    initWidgets();
    reloadServicesList();

    setWindowTitle(tr("Services Browser"));
}

ServiceBrowser::~ServiceBrowser()
{
    unregisterExampleServices();
}

void ServiceBrowser::currentInterfaceImplChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (!current)
        return;

    reloadAttributesList();
    reloadAttributesRadioButtonText();

    QServiceInterfaceDescriptor descriptor = current->data(Qt::UserRole).value<QServiceInterfaceDescriptor>();
    if (descriptor.isValid()) {
#if defined(Q_WS_MAEMO_5)
        defaultInterfaceButton->setText(tr("Set as default implementation for \n%1").arg(descriptor.interfaceName()));
#else
        defaultInterfaceButton->setText(tr("Set as default implementation for %1").arg(descriptor.interfaceName()));
#endif
                //TODO: .arg(descriptor.interfaceName()));
        defaultInterfaceButton->setEnabled(true);
    }
}

void ServiceBrowser::reloadServicesList()
{
    servicesListWidget->clear();

    QSet<QString> services;
    QList<QServiceInterfaceDescriptor> descriptors = serviceManager->findInterfaces();
    for (int i=0; i<descriptors.count(); i++) {
        QString service = descriptors[i].serviceName();

        if (descriptors[i].scope() == QService::SystemScope)
            service += tr(" (system)");

        services << service;
    }

    foreach (const QString& service, services)
        servicesListWidget->addItem(service);

    servicesListWidget->sortItems();
    servicesListWidget->addItem(showAllServicesItem);
}

void ServiceBrowser::reloadInterfaceImplementationsList()
{
    QString serviceName;
    if (servicesListWidget->currentItem()
            && servicesListWidget->currentItem() != showAllServicesItem) {
        serviceName = servicesListWidget->currentItem()->text();
        interfacesGroup->setTitle(tr("Interfaces implemented by %1").arg(serviceName));
    } else {
        interfacesGroup->setTitle(tr("All interface implementations"));
    }

    QServiceManager *manager = serviceManager;
    if (serviceName.endsWith(" (system)")) {
        serviceName.chop(9);
        manager = systemManager;
    }

    QList<QServiceInterfaceDescriptor> descriptors = manager->findInterfaces(serviceName);

    attributesListWidget->clear();
    interfacesListWidget->clear();
    for (int i=0; i<descriptors.count(); i++) {
        if (descriptors[i].scope() != manager->scope() && !serviceName.isEmpty())
            continue;

        QString text = QString("%1 %2.%3")
                .arg(descriptors[i].interfaceName())
                .arg(descriptors[i].majorVersion())
                .arg(descriptors[i].minorVersion());

        QServiceInterfaceDescriptor defaultInterfaceImpl
            = manager->interfaceDefault(descriptors[i].interfaceName());

        if (serviceName.isEmpty()) {
            text += " (" + descriptors[i].serviceName() + ")";

            if (descriptors[i].scope() == QService::SystemScope) {
                text += tr(" (system");
                defaultInterfaceImpl = systemManager->interfaceDefault(descriptors[i].interfaceName());
                if (descriptors[i] == defaultInterfaceImpl)
                    text += tr(" default)");
                else
                    text += ")";
                defaultInterfaceImpl = QServiceInterfaceDescriptor();
            }
        }

        if (descriptors[i] == defaultInterfaceImpl)
            text += tr(" (default)");

        QListWidgetItem *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, qVariantFromValue(descriptors[i]));
        interfacesListWidget->addItem(item);
    }

    defaultInterfaceButton->setEnabled(false);
}

void ServiceBrowser::reloadAttributesList()
{
    QListWidgetItem *item = interfacesListWidget->currentItem();
    if (!item)
        return;

    QServiceInterfaceDescriptor selectedImpl =
            item->data(Qt::UserRole).value<QServiceInterfaceDescriptor>();

    QObject *implementationRef;
    if (selectedImplRadioButton->isChecked())
        implementationRef = serviceManager->loadInterface(selectedImpl);
    else
        implementationRef = serviceManager->loadInterface(selectedImpl.interfaceName());

    attributesListWidget->clear();
    if (!implementationRef) {
        attributesListWidget->addItem(tr("(Error loading service plugin)"));
        return;
    }

    const QMetaObject *metaObject = implementationRef->metaObject();
    attributesGroup->setTitle(tr("Invokable attributes for %1 class")
            .arg(QString(metaObject->className())));
    for (int i=0; i<metaObject->methodCount(); i++) {
        QMetaMethod method = metaObject->method(i);
        attributesListWidget->addItem("[METHOD] " + QString(method.signature()));
    }
    for (int i=0; i<metaObject->propertyCount(); i++) {
        QMetaProperty property = metaObject->property(i);
        attributesListWidget->addItem("[PROPERTY] " + QString(property.name()));
    }
}

void ServiceBrowser::setDefaultInterfaceImplementation()
{
    QListWidgetItem *item = interfacesListWidget->currentItem();
    if (!item)
        return;

    QServiceInterfaceDescriptor descriptor = item->data(Qt::UserRole).value<QServiceInterfaceDescriptor>();
    if (descriptor.isValid()) {
        QServiceManager *manager = serviceManager;

        if (descriptor.scope() == QService::SystemScope)
            manager = systemManager;

        if (manager->setInterfaceDefault(descriptor)) {
            int currentIndex = interfacesListWidget->row(item);
            reloadInterfaceImplementationsList();
            interfacesListWidget->setCurrentRow(currentIndex);
        } else {
            qWarning() << "Unable to set default service for interface:"
                    << descriptor.interfaceName();
        }
    }
}

void ServiceBrowser::registerExampleServices()
{
//    QStringList exampleXmlFiles;
//    exampleXmlFiles << "filemanagerservice.xml" << "bluetoothtransferservice.xml" << "notesmanagerservice.xml";
//    foreach (const QString &fileName, exampleXmlFiles) {
//        const QString path = QCoreApplication::applicationDirPath() + "/xmldata/" + fileName;
//        serviceManager->addService(path);
//    }
}

void ServiceBrowser::unregisterExampleServices()
{
//    serviceManager->removeService("FileManagerService");
//    serviceManager->removeService("BluetoothTransferService");
//    serviceManager->removeService("NotesManagerService");
}

void ServiceBrowser::reloadAttributesRadioButtonText()
{
    QListWidgetItem *item = interfacesListWidget->currentItem();
    if (!item)
        return;

    QServiceInterfaceDescriptor selectedImpl =
            item->data(Qt::UserRole).value<QServiceInterfaceDescriptor>();
    QServiceInterfaceDescriptor defaultImpl =
            serviceManager->interfaceDefault(selectedImpl.interfaceName());

    defaultImplRadioButton->setText(tr("Default implementation for %1\n(currently provided by %2)")
            .arg(defaultImpl.interfaceName())
            .arg(defaultImpl.serviceName()));
}

void ServiceBrowser::initWidgets()
{
    showAllServicesItem = new QListWidgetItem(tr("(All registered services)"));

    servicesListWidget = new QListWidget;
    interfacesListWidget = new QListWidget;
    interfacesListWidget->addItem(tr("(Select a service)"));
    attributesListWidget = new QListWidget;
    attributesListWidget->addItem(tr("(Select an interface implementation)"));

#if !defined(Q_OS_SYMBIAN) && !defined(Q_WS_MAEMO_5)
    interfacesListWidget->setMinimumWidth(450);
#else
    interfacesListWidget->setMaximumWidth(360);
#endif

    connect(servicesListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(reloadInterfaceImplementationsList()));

    connect(interfacesListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(currentInterfaceImplChanged(QListWidgetItem*,QListWidgetItem*)));

    defaultInterfaceButton = new QPushButton(tr("Set as default implementation"));
    defaultInterfaceButton->setEnabled(false);
    connect(defaultInterfaceButton, SIGNAL(clicked()),
            this, SLOT(setDefaultInterfaceImplementation()));

    selectedImplRadioButton = new QRadioButton(tr("Selected interface implementation"));
    defaultImplRadioButton = new QRadioButton(tr("Default implementation"));
    selectedImplRadioButton->setChecked(true);

    QButtonGroup *radioButtons = new QButtonGroup(this);
    radioButtons->addButton(selectedImplRadioButton);
    radioButtons->addButton(defaultImplRadioButton);
    connect(radioButtons, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(reloadAttributesList()));

    QGroupBox *servicesGroup = new QGroupBox(tr("Show services for:"));
    QVBoxLayout *servicesLayout = new QVBoxLayout;
    servicesLayout->addWidget(servicesListWidget);
    servicesGroup->setLayout(servicesLayout);

    interfacesGroup = new QGroupBox(tr("Interface implementations"));
    QVBoxLayout *interfacesLayout = new QVBoxLayout;
    interfacesLayout->addWidget(interfacesListWidget);
    interfacesLayout->addWidget(defaultInterfaceButton);
    interfacesGroup->setLayout(interfacesLayout);

    attributesGroup = new QGroupBox(tr("Invokable attributes"));
    QVBoxLayout *attributesLayout = new QVBoxLayout;
    attributesLayout->addWidget(attributesListWidget);
#if !defined(Q_WS_MAEMO_5)
    // No space on the screen to show following title in Maemo
    attributesLayout->addWidget(new QLabel(tr("Show attributes for:")));
#endif
    attributesLayout->addWidget(selectedImplRadioButton);
    attributesLayout->addWidget(defaultImplRadioButton);
    attributesGroup->setLayout(attributesLayout);

#if defined(Q_WS_MAEMO_5)
    // Maemo 5 style doesn't take group box titles into account.
    int spacingHack = QFontMetrics(QFont()).height();
    interfacesLayout->setContentsMargins(0, spacingHack, 0, 0);
    attributesLayout->setContentsMargins(0, spacingHack, 0, 0);
    servicesLayout->setContentsMargins(0, spacingHack, 0, 0);
#endif
#if !defined(Q_OS_SYMBIAN) && !defined(Q_WS_MAEMO_5)
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(servicesGroup, 0, 0);
    layout->addWidget(attributesGroup, 0, 1, 2, 1);
    layout->addWidget(interfacesGroup, 1, 0);
#else
    QVBoxLayout *layout = new QVBoxLayout;
    QStackedLayout *stackedLayout = new QStackedLayout;
    stackedLayout->addWidget(servicesGroup);
    stackedLayout->addWidget(interfacesGroup);
    stackedLayout->addWidget(attributesGroup);

    QComboBox *pageComboBox = new QComboBox;
    pageComboBox->addItem(tr("Services"));
    pageComboBox->addItem(tr("Interfaces"));
    pageComboBox->addItem(tr("Attributes"));
    connect(pageComboBox, SIGNAL(activated(int)),
            stackedLayout, SLOT(setCurrentIndex(int)));

    layout->addWidget(pageComboBox);
    layout->addLayout(stackedLayout);
#endif

    setLayout(layout);
}
