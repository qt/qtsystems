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
