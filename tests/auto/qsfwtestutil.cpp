/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
#include "qsfwtestutil.h"

#include <QDir>
#include <QFileInfoList>
#include <QFile>
#include <QSettings>
#include <QDebug>
#include <QCoreApplication>

#ifdef QT_JSONDB
#include <private/jsondb-connection_p.h>
#include <private/jsondb-strings_p.h>
#include <mtcore/jsondb-constants.h>
#include <jsondb-client.h>
#include <jsondb-global.h>
#include <QSignalSpy>

const QLatin1String kQuery("query");

Q_ADDON_JSONDB_BEGIN_NAMESPACE
class JsonDbClient;
Q_ADDON_JSONDB_END_NAMESPACE
Q_USE_JSONDB_NAMESPACE

#endif

void QSfwTestUtil::setupTempUserDb()
{
    QSettings::setUserIniPath(tempUserDbDir());
}

void QSfwTestUtil::setupTempSystemDb()
{
    QSettings::setSystemIniPath(tempSystemDbDir());
}

void QSfwTestUtil::removeTempUserDb()
{
    removeDirectory(tempUserDbDir());
}

void QSfwTestUtil::removeTempSystemDb()
{
    removeDirectory(tempSystemDbDir());
}

QString QSfwTestUtil::tempUserDbDir()
{
    return tempSettingsPath("__user__");
}

QString QSfwTestUtil::tempSystemDbDir()
{
    return tempSettingsPath("__system__");
}

QString QSfwTestUtil::userDirectory()
{
    return tempSettingsPath("__user__/");
}

QString QSfwTestUtil::systemDirectory()
{
    return tempSettingsPath("__system__/");
}

QString QSfwTestUtil::tempSettingsPath(const char *path)
{
    // Temporary path for files that are specified explictly in the constructor.
    //QString tempPath = QDir::tempPath();
    QString tempPath = QCoreApplication::applicationDirPath();
    if (tempPath.endsWith("/"))
        tempPath.truncate(tempPath.size() - 1);
    return QDir::toNativeSeparators(tempPath + "/QtServiceFramework_tests/" + QLatin1String(path));
}

void QSfwTestUtil::removeDirectory(const QString &path)
{
    QDir dir(path);
    QFileInfoList fileList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
    foreach(QFileInfo file, fileList) {
        if(file.isFile()) {
            QFile::remove (file.canonicalFilePath());
        }
        if(file.isDir()) {
            QFile::Permissions perms = QFile::permissions(file.canonicalFilePath());
            perms = perms | QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner;
            QFile::setPermissions(file.canonicalFilePath(), perms);
            removeDirectory(file.canonicalFilePath());
        }
    }
    dir.rmpath(path);
}

#if defined(QT_JSONDB)

void QSfwTestUtil::clearDatabases_jsondb()
{
    JsonDbClient *db = new JsonDbClient();
    QSignalSpy response(db, SIGNAL(response(int,QVariant)));
    QSignalSpy error(db, SIGNAL(error(int,int,QString)));
    bool waiting = true;

    QList<QVariant> args;

    QVariantMap query;
    query.insert(kQuery, QString::fromLatin1("[?%1=\"com.nokia.mp.serviceframework.interface\"]")
                 .arg(JsonDbString::kTypeStr));

    db->find(query);
    do {
        QCoreApplication::processEvents();
        if (response.count()){
            args = response.first();
            waiting = false;
        }
        if (error.count()) {
            waiting = false;
        }
    } while (waiting);

    if (args.isEmpty()) {
        return;
    }

    QVariant v = args.at(1).value<QVariant>();
    QVariantMap vm = v.toMap();

    QList<QVariant> list = vm[QLatin1String("data")].toList();
    if (list.isEmpty()) {
        return;
    }
    foreach (const QVariant &v, list) {
        QVariantMap x;
        x.insert(QLatin1String("_uuid"), v.toMap()[QLatin1String("_uuid")]);
        response.clear();
        error.clear();
        db->remove(x);
        waiting = true;
        do {
            QCoreApplication::processEvents();
            if (response.count() || error.count()) {
                waiting = false;
            }
        } while (waiting);
    }
}

#endif

#if defined(Q_OS_SYMBIAN)
#include <e32base.h>
void QSfwTestUtil::removeDatabases_symbian()
{
#if defined(__WINS__) && !defined(SYMBIAN_EMULATOR_SUPPORTS_PERPROCESS_WSD)
    QDir dir("C:/Data/temp/QtServiceFW");
#else
    TFindServer findServer(_L("!qsfwdatabasemanagerserver"));
    TFullName name;
    if (findServer.Next(name) == KErrNone)
    {
        RProcess dbServer;
        if (dbServer.Open(_L("qsfwdatabasemanagerserver")) == KErrNone)
        {
            dbServer.Kill(KErrNone);
            dbServer.Close();    
        }
    }    

    QDir dir("c:/private/2002AC7F");
#endif

    QString qtVersion(qVersion());
    qtVersion = qtVersion.left(qtVersion.size() - 2); //strip off patch version
    QString dbIdentifier = "_system";
    QString dbName = QString("QtServiceFramework_") + qtVersion + dbIdentifier + QLatin1String(".db");
    QFile::remove(QDir::toNativeSeparators(dir.path() + QDir::separator() + dbName));
}
#endif

