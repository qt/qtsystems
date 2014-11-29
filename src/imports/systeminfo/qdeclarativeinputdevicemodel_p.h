/****************************************************************************
**
** Copyright (C) 2016 Canonical, Ltd. and/or its subsidiary(-ies).
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
