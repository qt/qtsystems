/****************************************************************************
**
** Copyright (C) 2018 Canonical, Ltd. and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDECLARATIVEINPUTDEVICEMODEL_H
#define QDECLARATIVEINPUTDEVICEMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "qinputinfo.h"

QT_BEGIN_NAMESPACE

class QDeclarativeInputDeviceModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QInputDevice::InputTypeFlags filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum ItemRoles {
        ServiceRole = Qt::UserRole + 1,
        NameRole,
        IdentifierRole,
        ButtonsRole,
        SwitchesRole,
        RelativeAxesRole,
        AbsoluteAxesRole,
        TypesRole
    };
    Q_ENUMS(ItemRoles)

    explicit QDeclarativeInputDeviceModel(QObject *parent = 0);
    virtual ~QDeclarativeInputDeviceModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    void setFilter(QInputDevice::InputTypeFlags filterFlags);
    QInputDevice::InputTypeFlags filter();

    Q_INVOKABLE int indexOf(const QString &devicePath) const;

    Q_INVOKABLE QInputDevice *get(int index) const;
    QHash<int, QByteArray> roleNames() const;

Q_SIGNALS:
    void added(QInputDevice *inputDevice);
    void removed(const QString &deviceId);
    void filterChanged(QInputDevice::InputTypeFlags filterFlags);
    void countChanged(int devices);

public Q_SLOTS:
    void updateDeviceList();
private:
    QInputInfoManager *deviceInfoManager;
    QVector<QInputDevice *> inputDevices;
    QInputDevice::InputTypeFlags currentFilter;
    QInputDevice *m_lastAddedDevice;

private slots:
    void addedDevice(QInputDevice *device);
    void removedDevice(const QString &path);

};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDeclarativeInputDeviceModel::ItemRoles)

#endif // QDECLARATIVEINPUTDEVICEMODEL_H
