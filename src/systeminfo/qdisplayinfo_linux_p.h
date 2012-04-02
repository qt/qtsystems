/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDISPLAYINFO_LINUX_P_H
#define QDISPLAYINFO_LINUX_P_H

#include <qdisplayinfo.h>

#if !defined(QT_NO_JSONDB)
#include <QMap>
#endif //QT_NO_JSONDB

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_JSONDB)
class QJsonDbWrapper;
#endif //QT_NO_JSONDB

class QDisplayInfoPrivate : public QObject
{
    Q_OBJECT

public:
    QDisplayInfoPrivate(QDisplayInfo *parent);

    int brightness(int screen);
    int contrast(int screen);
    QDisplayInfo::BacklightState backlightState(int screen);

#if !defined(QT_NO_JSONDB)
Q_SIGNALS:
    void backlightStateChanged(int screen, QDisplayInfo::BacklightState state);

private Q_SLOTS:
    void onBacklightStateChanged(int screen, QDisplayInfo::BacklightState state);

protected:
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);
#endif

private:
    QDisplayInfo * const q_ptr;
    Q_DECLARE_PUBLIC(QDisplayInfo)


#if !defined(QT_NO_JSONDB)
    QMap<int, QDisplayInfo::BacklightState> backlightStates;
    bool backlightStateWatcher;
    QJsonDbWrapper *jsondbWrapper;

    void initScreenMap();
#endif //QT_NO_JSONDB
};

QT_END_NAMESPACE

#endif // QDISPLAYINFO_LINUX_P_H
