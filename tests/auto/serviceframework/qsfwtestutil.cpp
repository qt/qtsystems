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
#include "qsfwtestutil.h"

#include <QDir>
#include <QFileInfoList>
#include <QFile>
#include <QSettings>
#include <QDebug>
#include <QCoreApplication>
#include <QTimer>

#ifdef QT_ADDON_JSONDB_LIB
#include <QSignalSpy>
#include <QtJsonDb/qjsondbconnection.h>
#include <QtJsonDb/qjsondbreadrequest.h>
#include <QtJsonDb/qjsondbwriterequest.h>

Q_DECLARE_METATYPE(QtJsonDb::QJsonDbRequest::ErrorCode)

QT_USE_NAMESPACE_JSONDB

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

#if defined(QT_ADDON_JSONDB_LIB)

Q_GLOBAL_STATIC(QList<QJsonObject>, _q_storedData);

void QSfwTestUtil::saveDatabases_jsondb()
{

    qRegisterMetaType<QtJsonDb::QJsonDbRequest::ErrorCode>("QtJsonDb::QJsonDbRequest::ErrorCode");

    QJsonDbConnection *db = QJsonDbConnection::defaultConnection();
    db->connectToServer();

    bool waiting = true;

    QList<QJsonObject> args;

    QJsonDbReadRequest request;
    request.setPartition(QStringLiteral("com.nokia.mt.Settings"));
    request.setQuery(QStringLiteral("[?_type=\"com.nokia.mt.serviceframework.interface\"]"));

    QSignalSpy response(&request, SIGNAL(finished()));
    QSignalSpy error(&request, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));
    QSignalSpy discon(db, SIGNAL(disconnected()));

    db->send(&request);
    do {
        QCoreApplication::processEvents();
        if (response.count()){
            args = request.takeResults();
            waiting = false;
        }
        if (error.count() || discon.count()) {
            waiting = false;
        }
    } while (waiting);

    if (args.isEmpty()) {
        return;
    }

    QList<QJsonObject> *storage = _q_storedData();
    storage->clear();
    storage->append(args);

    QJsonDbRemoveRequest rm(args);
    rm.setPartition(QStringLiteral("com.nokia.mt.Settings"));

    QSignalSpy response2(&rm, SIGNAL(finished()));
    QSignalSpy error2(&rm, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));

    db->send(&rm);
    waiting = true;
    do {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
        if (response2.count() || error2.count() || discon.count()) {
            waiting = false;
        }
    } while (waiting);
}

void QSfwTestUtil::clearDatabases_jsondb()
{
    qRegisterMetaType<QtJsonDb::QJsonDbRequest::ErrorCode>("QtJsonDb::QJsonDbRequest::ErrorCode");

    QJsonDbConnection *db = QJsonDbConnection::defaultConnection();
    db->connectToServer();

    bool waiting = true;

    QList<QJsonObject> args;

    QJsonDbReadRequest request;
    request.setPartition(QStringLiteral("com.nokia.mt.Settings"));
    request.setQuery(QStringLiteral("[?_type=\"com.nokia.mt.serviceframework.interface\"]"));

    QSignalSpy response(&request, SIGNAL(finished()));
    QSignalSpy error(&request, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));
    QSignalSpy discon(db, SIGNAL(disconnected()));

    db->send(&request);
    do {
        QCoreApplication::processEvents();
        if (response.count()){
            args = request.takeResults();
            waiting = false;
        }
        if (error.count() || discon.count()) {
            waiting = false;
        }
    } while (waiting);

    if (args.isEmpty()) {
        return;
    }

    QJsonDbRemoveRequest rm(args);
    rm.setPartition(QStringLiteral("com.nokia.mt.Settings"));

    QSignalSpy response2(&rm, SIGNAL(finished()));
    QSignalSpy error2(&rm, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));

    db->send(&rm);
    waiting = true;
    do {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
        if (response2.count() || error2.count() || discon.count()) {
            waiting = false;
        }
    } while (waiting);
}



void QSfwTestUtil::restoreDatabases_jsondb()
{

    qRegisterMetaType<QtJsonDb::QJsonDbRequest::ErrorCode>("QtJsonDb::QJsonDbRequest::ErrorCode");

    QJsonDbConnection *db = QJsonDbConnection::defaultConnection();
    db->connectToServer();

    bool waiting = true;

    QList<QJsonObject> args;

    QJsonDbReadRequest request;
    request.setPartition(QStringLiteral("com.nokia.mt.Settings"));
    request.setQuery(QStringLiteral("[?_type=\"com.nokia.mt.serviceframework.interface\"]"));

    QSignalSpy response(&request, SIGNAL(finished()));
    QSignalSpy error(&request, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));
    QSignalSpy discon(db, SIGNAL(disconnected()));

    db->send(&request);
    do {
        QCoreApplication::processEvents();
        if (response.count()){
            args = request.takeResults();
            waiting = false;
        }
        if (error.count() || discon.count()) {
            waiting = false;
        }
    } while (waiting);

    if (!args.isEmpty()) {

        QJsonDbRemoveRequest rm(args);
        rm.setPartition(QStringLiteral("com.nokia.mt.Settings"));

        QSignalSpy response2(&rm, SIGNAL(finished()));
        QSignalSpy error2(&rm, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));

        db->send(&rm);
        waiting = true;
        do {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
            if (response2.count() || error2.count() || discon.count()) {
                waiting = false;
            }
        } while (waiting);
    }

    QList<QJsonObject> *storage = _q_storedData();

    if (!storage->isEmpty()) {
        QJsonDbWriteRequest add;
        add.setPartition(QStringLiteral("com.nokia.mt.Settings"));
        add.setObjects(*storage);

        QSignalSpy response_w(&add, SIGNAL(finished()));
        QSignalSpy error_w(&add, SIGNAL(error(QtJsonDb::QJsonDbRequest::ErrorCode,QString)));

        db->send(&add);
        waiting = true;
        do {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
            if (response_w.count() || error_w.count() || discon.count()) {
                waiting = false;
            }
        } while (waiting);
    }

}


#endif

