/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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

#include "qservicecontext.h"

QTM_BEGIN_NAMESPACE

#define CLIENT_DATA_INDEX 0

class ServiceContextClientData : public QObjectUserData
{
public:
    virtual ~ServiceContextClientData() {}

    QHash<QString, QVariant> clientData;
};


/*!
    \class QServiceContext
    \inmodule QtServiceFramework
    \ingroup servicefw
    \brief The QServiceContext class provides context information to
    services.
    \since 1.0

    A service context is created by clients and passed on to the service.
    It enables the opportunity to pass additional context information
    and errors between services, clients and the service framework.

    Clients must implement this abstract class to receive context information.

    \sa QServiceManager

*/

/*!
    \enum QServiceContext::ContextType

    This enum describes the type of context information.

    \value  DisplayContext              The service provides user visible display
                                        text such as an error message.
    \value  ScriptContext               The service provides a script which may
                                        be executed by the client.
    \value  UserDefined                 The first context type that can be used for service
                                        specific context information.
*/

/*!
    \fn void QServiceContext::notify(ContextType type, const QVariant& data) = 0

    Services may call this function to notify the service client about service related
    context information of the given \a type. The contextual information is stored in \a data.
    \since 1.0
*/


/*!
    Constructs a service context with the given \a parent.
*/
QServiceContext::QServiceContext(QObject* parent)
    : QObject(parent)
{
#ifndef QT_NO_USERDATA
    //Ideally we would use a new data member to store the client information.
    //However since a d-pointer was forgotten when designing QServiceContext
    //we need to fall back to QObject user data.
    ServiceContextClientData* data = new ServiceContextClientData();
    setUserData(CLIENT_DATA_INDEX, data);
#endif
}

/*!
    Destroys the service context object.
*/
QServiceContext::~QServiceContext()
{
    //ServiceContextUserData deleted by QObject
}

/*!
    \property QServiceContext::clientId
    \brief the id of the client using the service.

    By default, this value is empty but you can change this by calling
    setClientId().
    \since 1.0
*/
QString QServiceContext::clientId() const
{
    return m_id;
}

/*!
    Sets the \a id of the client using the service.
    \since 1.0
*/
void QServiceContext::setClientId(const QString& id)
{
    m_id = id;
}

/*!
    \property QServiceContext::clientName
    \brief the name of the client using the service.

    By default, this value is empty but you can change this by calling
    setClientName(). This string is translated and can be shown to the user.
    \since 1.0
*/
QString QServiceContext::clientName() const
{
    return m_displayName;
}

void QServiceContext::setClientName(const QString& name)
{
    m_displayName = name;
}

/*!
    Returns the client data associated to \a key.

    \sa setClientData(), resetClientData()
    \since 1.1
*/
QVariant QServiceContext::clientData(const QString& key) const
{
#ifndef QT_NO_USERDATA
    ServiceContextClientData* data =
        static_cast<ServiceContextClientData*>(userData(CLIENT_DATA_INDEX));
    return data->clientData.value(key);
#else
    return QVariant();
#endif
}

/*!
    Attaches arbitrary data \a value to the context object. The value
    can be retrieved via \a key.

    \sa clientData(), resetClientData()
    \since 1.1
*/
void QServiceContext::setClientData(const QString& key, const QVariant& value)
{
#ifndef QT_NO_USERDATA
    ServiceContextClientData* data =
        static_cast<ServiceContextClientData*>(userData(CLIENT_DATA_INDEX));
    data->clientData[key] = value;
#endif
}

/*!
    Deletes all client data associated to the service context.

    \sa clientData(), setClientData()
    \since 1.1
*/
void QServiceContext::resetClientData()
{
#ifndef QT_NO_USERDATA
    ServiceContextClientData* data =
        static_cast<ServiceContextClientData*>(userData(CLIENT_DATA_INDEX));
    data->clientData.clear();
#endif
}

#include "moc_qservicecontext.cpp"

QTM_END_NAMESPACE
