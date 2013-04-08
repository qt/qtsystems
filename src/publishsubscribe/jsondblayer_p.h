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

#ifndef JSONDBLAYER_P_H
#define JSONDBLAYER_P_H

#if !defined(QT_NO_JSONDBLAYER)

#include "qvaluespace_p.h"

#include <QJsonDbConnection>
#include <QJsonDbWatcher>
#include <QJsonDbReadRequest>
#include <QJsonDbUpdateRequest>

QT_BEGIN_NAMESPACE

#ifndef QStringLiteral
#define QStringLiteral(a) a
#endif

/*
 This implementation is heavely influenced by Context Kit Layer implementation from MeeGo.

 Path: you can use either paths beginning with a / or such without a leading /. Example:
     * /com.nokia/ofono/call
     * com.nokia/ofono/call
     All leading and trailing / are removed.

     The last part of the path is expected to be the name of a setting in the settings object. If
     this is not the case then the whole path is used to access the whole settings object.
     Flow:
        * Remove the last part of the path
        * Construct the JSON query this way:
            QString path = "com.nokia.email.asetting";
            parts[0] = "com.nokia.email"; // Assumed to be the identifier
            parts[1] = "asetting"; // Last part of the path assumed to be the name of a setting
            QString("[?identifier=\"%1\"][?_type="ApplicationSettings"][={value:%2}]").arg(parts[0]).arg(parts[1])
        * Try to access the setting and return or set its value if successfull
        * If the query doesn't match anything then use the whole path to search for the settings
          object and return it if found

 Notifications:
    * If you register for a path then you will be notified if that path or any path bellow it changes
    * A notification is sent if:
        * Object is created
        * Object is deleted
        * Object is updated
*/

#define IDENTIFIER QStringLiteral("identifier")
#define SETTINGS_FILTER QStringLiteral("[?_type in [\"com.nokia.mt.settings.ApplicationSettings\", \"com.nokia.mt.settings.SystemSettings\"]]")

class JsonDbSyncCall: public QObject
{
    Q_OBJECT

    public:
        JsonDbSyncCall(const QString &partition, const QString &query, QList<QJsonObject> *result);
        JsonDbSyncCall(const QString &partition, const QJsonObject *object);
        ~JsonDbSyncCall();

        bool wasSuccessful() const;

    public Q_SLOTS:
        void createSyncReadRequest();
        void createSyncUpdateRequest();

    private Q_SLOTS:
        void handleResponse(int id);
        void handleFinished();
        void handleError(QtJsonDb::QJsonDbRequest::ErrorCode id, QString code);

    private:
        QString mQuery;
        const QJsonObject *mObject;
        const QString mPartition;
        QList<QJsonObject> *mResult;
        bool mSuccess;
        QtJsonDb::QJsonDbConnection *mConnection;
        QtJsonDb::QJsonDbReadRequest *mReadRequest;
        QtJsonDb::QJsonDbUpdateRequest *mUpdateRequest;

        void timerEvent(QTimerEvent *event);
};



class JsonDbPath
{
    public:
        JsonDbPath();
        explicit JsonDbPath(const QString &path);
        JsonDbPath(const JsonDbPath &other);

        inline QString getPath() const { return path; }

        // This method splits a path like this:
        //    com.nokia.mail.settingname
        //        [0] = "com.nokia.mail"
        //        [1] = "settingname"
        static QStringList getIdentifier(const QString &path);

        static QString normalizePath(const QString &path);

        JsonDbPath &operator=(const JsonDbPath &other);
        bool operator==(const JsonDbPath &other) const;

        JsonDbPath operator+(const QString &str) const;
        JsonDbPath operator+(const JsonDbPath &other) const;

    private:
        QString path;
};



class JsonDbHandle : public QObject
{
    Q_OBJECT

    public:
        JsonDbHandle(JsonDbHandle *parent,
                     const QString &root,
                     QValueSpace::LayerOptions opts);
        ~JsonDbHandle();

        bool value(const QString &path, QVariant *data);
        bool setValue(const QString &path, const QVariant &data);

        // Is called by JsonDbLayer::removeValue()
        // Deletes the object assotiated with the path from JSON DB.
        bool unsetValue(const QString &path);

        // Removes all objects under current path
        bool removeSubTree();

        // Subscribes for JSON DB notifications
        void subscribe();

        // Removes the subscription for JSON DB notifications
        void unsubscribe();

        // Returns children of the current path (only first level)
        QSet<QString> children();

        inline QString getPath() { return path.getPath(); }

    Q_SIGNALS:
        void valueChanged();
        void interestChanged(QString path, bool status);

    private Q_SLOTS:
        void onError(int id, int code, const QString & message);
        void onNotificationsAvailable(int);

    private:
        JsonDbPath path;
        QtJsonDb::QJsonDbConnection *client;
        QtJsonDb::QJsonDbWatcher *watcher;
        bool connected;

        static QJsonObject getObject(const QString &identifier, const QString &property);
        static QList<QJsonObject> getResponse(const QString &partition, const QString& query);
        QString getWholePath(const QString &path) const;
        static QString getObjectQuery(const QString &identifier);
        static QString getSettingQuery(const QString &identifier);

        void getNotificationQueryAndActions(QString path, QString &query, QtJsonDb::QJsonDbWatcher::Actions &actions);

        bool doUpdateRequest(const QString &partition, const QJsonObject&);
};



class JsonDbLayer : public QAbstractValueSpaceLayer
{
    Q_OBJECT

    public:
        JsonDbLayer();

        // Virtual methods inherited from QAbstractValueSpaceLayer

        // Registers creator for change notifications to values under handle.
        void addWatch(QValueSpacePublisher *creator, Handle handle);

        // Removes all registered change notifications for creator under parent.
        void removeWatches(QValueSpacePublisher *creator, Handle parent);

        // Returns the set of children of handle.
        QSet<QString> children(Handle handle);

        // Returns a globally unique identifier for the layer.
        QUuid id();

        // Returns a new opaque handle for the requested subPath of parent. If parent is an
        // InvalidHandle, subPath is interpreted as an absolute path. The caller should call
        // removeHandle() to free resources used by the handle when it is no longer required.
        Handle item(Handle parent, const QString &subPath);

        // Returns the QValueSpace::LayerOptions describing this layer.
        QValueSpace::LayerOptions layerOptions() const;

        // Registers or unregisters that the caller is interested in handle and any subpaths under
        // it. If interested is true interest in handle is registered; otherwise it is unregistered.
        // The caller should ensure that all calls to this function with interested set to true
        // have a matching call with interested set to false.
        // Returns true if the notification was successfully sent; otherwise returns false.
        bool notifyInterest(Handle handle, bool interested);

        // Releases a handle previously returned from QAbstractValueSpaceLayer::item().
        void removeHandle(Handle handle);

        // Process calls to QValueSpacePublisher::~QValueSpacePublisher() by removing the entire
        // sub tree created by creator under handle.
        // Returns true on success; otherwise returns false.
        bool removeSubTree(QValueSpacePublisher *creator, Handle handle);

        // Process calls to QValueSpacePublisher::resetValue() by removing the Value Space item
        // identified by handle and subPath and created by creator.
        // Returns true on success; otherwise returns false.
        bool removeValue(QValueSpacePublisher *creator, Handle handle, const QString &subPath);

        // Apply the specified property mask to handle.
        void setProperty(Handle handle, Properties property);

        // Process calls to QValueSpacePublisher::setValue() by setting the value specified by the
        // subPath under handle to value. Ownership of the Value Space item is assigned to creator.
        // Returns true on success; otherwise returns false.
        // IMPORTANT: if there is no object with given path then no new object is created!
        bool setValue(QValueSpacePublisher *creator, Handle handle, const QString &subPath, const QVariant &value);

        // Returns true if the layer supports interest notifications; otherwise returns false.
        bool supportsInterestNotification() const;

        // Flushes all pending changes made by calls to setValue(), removeValue() and removeSubTree().
        void sync();

        // Returns the value for a particular handle. If a value is available, the layer will set
        // data and return true. If no value is available, false is returned.
        bool value(Handle handle, QVariant *data);

        // Returns the value for a particular subPath of handle. If a value is available, the layer
        // will set data and return true. If no value is available, false is returned.
        bool value(Handle handle, const QString &subPath, QVariant *data);

        // Other methods
        static JsonDbLayer *instance();

    Q_SIGNALS:
        void valueChanged();
        void interestChanged(QString path, bool status);

    private Q_SLOTS:
        void jsonDbHandleChanged();
};

QT_END_NAMESPACE

#endif // QT_NO_JSONDBLAYER

#endif // JSONDBLAYER_P_H
