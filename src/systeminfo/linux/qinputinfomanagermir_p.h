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

#ifndef QINPUTINFOMANAGER_MIR_P_H
#define QINPUTINFOMANAGER_MIR_P_H

#include "qinputinfomanager_p.h"
#include <mir_toolkit/client_types.h>
#include <mir_toolkit/mir_connection.h>
#include <mir_toolkit/mir_input_device.h>

#include <memory>

QT_BEGIN_NAMESPACE

class QInputInfoManagerMir : public QInputInfoManagerPrivate
{
    Q_OBJECT
public:
    using MirConnectionPtr = std::unique_ptr<MirConnection,void (*)(MirConnection*)>;
    explicit QInputInfoManagerMir(MirConnectionPtr connection, QObject *parent = NULL);
    ~QInputInfoManagerMir();

private:
    void update_devices();
    MirConnectionPtr connection;
    using MirInputConfigPtr = std::unique_ptr<MirInputConfig const,void (*)(MirInputConfig const*)>;
    MirInputConfigPtr config;

private Q_SLOTS:
    void init();
};

QT_END_NAMESPACE

#endif // QINPUTINFOMANAGER_MIR_P_H
