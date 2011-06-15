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
#include "databasemanagerserver_p.h"
#include "clientservercommon.h"
#include "databasemanagersession_p.h"
#include "servicedatabase_p.h"

#include <QFileSystemWatcher>

#include <QCoreApplication>
//#include <QThread>

QTM_BEGIN_NAMESPACE

#define SEC_TOKEN 0x00000000

static TInt Timeout(TAny *aObject);

const TInt CDatabaseManagerServer::timeoutInterval = 30000000; // 30 seconds

TInt Timeout(TAny *aObject)
  {
    ((CDatabaseManagerServer *)aObject)->Shutdown();
    return 1;
  }

CDatabaseManagerServer::CDatabaseManagerServer()
    : CServer2(EPriorityNormal, ESharableSessions)
    , iSessionCount(0)
    {
    iPeriodic = CPeriodic::NewL(0);
    iPeriodic->Start(timeoutInterval, timeoutInterval, TCallBack(Timeout, this));
    iDb = new ServiceDatabase();
    initDbPath();

    iDatabaseManagerServerSignalHandler = new DatabaseManagerServerSignalHandler(this);
    iWatcher = new QFileSystemWatcher();
    QObject::connect(iWatcher, SIGNAL(directoryChanged(QString)),
            iDatabaseManagerServerSignalHandler, SLOT(importChanged(QString)));

    QString path = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());
    path += QDir::separator() + QString("import");

    // Make the directory incase no xml services are installed
    QDir dir;
    dir.mkdir(path);
    iWatcher->addPath(path);

    DiscoverServices();

    }

CSession2* CDatabaseManagerServer::NewSessionL(const TVersion& aVersion, const RMessage2& /*aMessage*/) const
    {
        if (!User::QueryVersionSupported(TVersion(KServerMajorVersionNumber,
                KServerMinorVersionNumber, KServerBuildVersionNumber), aVersion))
            {
            User::Leave(KErrNotSupported);
            }

        return CDatabaseManagerServerSession::NewL(*const_cast<CDatabaseManagerServer*>(this), iDb->databasePath());
    }

void CDatabaseManagerServer::PanicServer(TDatabaseManagerSerververPanic aPanic)
    {
    _LIT(KTxtServerPanic,"Database manager server panic");
    User::Panic(KTxtServerPanic, aPanic);
    }

void CDatabaseManagerServer::IncreaseSessions()
    {
    iSessionCount++;
    iPeriodic->Cancel();
    }

void CDatabaseManagerServer::DecreaseSessions()
    {
    iSessionCount--;
    if (iSessionCount <= 0)
        {
        iPeriodic->Start(timeoutInterval, timeoutInterval, TCallBack(Timeout, this));
        }
    }

void CDatabaseManagerServer::Shutdown()
  {
  QCoreApplication::exit(0);
  }

void CDatabaseManagerServer::initDbPath()
  {
  QString dbIdentifier = "_system";

  QDir dir(QDir::toNativeSeparators(QCoreApplication::applicationDirPath()));
  QString qtVersion(qVersion());
  qtVersion = qtVersion.left(qtVersion.size() -2); //strip off patch version
  QString dbName = QString("QtServiceFramework_") + qtVersion + dbIdentifier + QLatin1String(".db");
  iDb->setDatabasePath(dir.path() + QDir::separator() + dbName);

  // check if database is copied from Z drive; also valid for emulator
  QFile dbFile(iDb->databasePath());
  QFileInfo dbFileInfo(dbFile);
  if (!dbFileInfo.exists()) {
      // create folder first
      if (!dbFileInfo.dir().exists())
          QDir::root().mkpath(dbFileInfo.path());
      // copy file from ROM
      QFile romDb(QLatin1String("z:\\private\\2002ac7f\\") + dbFileInfo.fileName());
      // why not use QFile::copy?
      if (romDb.open(QIODevice::ReadOnly) && dbFile.open(QFile::WriteOnly)) {
          QByteArray data = romDb.readAll();
          dbFile.write(data);
          dbFile.close();
          romDb.close();
      }
  }

  iDb->open();
  }

void CDatabaseManagerServer::DiscoverServices()
{
  QString path = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());
  QSettings settings(path + QDir::separator() + "autoimport.ini",
      QSettings::NativeFormat);

  QString imports = path + QDir::separator() + "import";

  QDir dir(imports);
  dir.setFilter(QDir::Files);
  QStringList filters;
  filters << "*.xml";
  dir.setNameFilters(filters);
  QSet<QString> seen;

  QString tok = QString::number(SEC_TOKEN);

  QStringList files = dir.entryList();
  while(!files.isEmpty()){
      QString file = files.takeFirst();
      seen << file;

      QFileInfo fileinfo(imports + QDir::separator() + file);

      if(settings.contains(file)){
         if(fileinfo.lastModified() == settings.value(file).toDateTime()) {
             continue;
         }
      }
      QFile *f = new QFile(imports + QDir::separator() + file);
      // read contents, register
      ServiceMetaData parser(f);
      if (!parser.extractMetadata()) {
          f->remove();
          f->close();
          continue;
      }
      const ServiceMetaDataResults data = parser.parseResults();
      ServiceMetaDataResults results = parser.parseResults();
      QString servicename = results.name;

      if(iDb->registerService(results, tok)){
          iDb->serviceInitialized(results.name, tok);
      }
      f->close();
      settings.setValue(file, fileinfo.lastModified());
      settings.setValue(file + "/service_name", servicename);
  }

  QSet<QString> oldfiles = settings.allKeys().toSet();
  oldfiles -= seen;
  foreach(QString old, oldfiles){
      if(old.contains('/'))
        continue;
      QString servicename = settings.value(old + "/service_name").toString();
      iDb->unregisterService(servicename, QString("Auto Registration"));
      settings.remove(old);
  }
}

void DatabaseManagerServerSignalHandler::importChanged(const QString& path)
{
  iDatabaseManagerServerSession->DiscoverServices();
}


QTM_END_NAMESPACE


#include "moc_databasemanagerserver_p.cpp"
// End of File
