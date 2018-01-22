/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSystems module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
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
