/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: http://www.qt-project.org/
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "dberror_p.h"

QT_BEGIN_NAMESPACE

DBError::DBError()
{
    setError(NoError);
}

void DBError::setError(ErrorCode error, const QString &text)
{
    m_error = error;
    switch (error) {
        case (NoError):
            m_text = QLatin1String("No error");
            break;
        case(DatabaseNotOpen):
            m_text = QLatin1String("Database not open");
            break;
        case(InvalidDatabaseConnection):
            m_text = QLatin1String("Invalid database connection");
            break;
        case(ExternalIfaceIDFound):
            m_text = QLatin1String("External InterfaceID found");
            break;
        case(SqlError):
        case(NotFound):
        case(LocationAlreadyRegistered):
        case(IfaceImplAlreadyRegistered):
        case(CannotCreateDbDir):
        case(InvalidDescriptorScope):
        case(IfaceIDNotExternal):
        case(InvalidDatabaseFile):
        case(NoWritePermissions):
        case(CannotOpenServiceDb):
            m_text = text;
            break;
        default:
            m_text= QLatin1String("Unknown error");
            m_error = UnknownError;
    }
}
QT_END_NAMESPACE
