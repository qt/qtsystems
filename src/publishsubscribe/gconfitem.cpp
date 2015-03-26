/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
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

#if !defined(QT_NO_GCONFLAYER)

#include "gconfitem_p.h"

#include <QtCore/qdebug.h>
#include <QtCore/qstringlist.h>

#include <glib.h>
#include <gconf/gconf-value.h>
#include <gconf/gconf-client.h>

QT_BEGIN_NAMESPACE

struct GConfItemPrivate
{
    QString key;
    QVariant value;
    bool monitor;
    guint notify_id;

    static void notify_trampoline(GConfClient *, guint, GConfEntry *, gpointer);
};

#define withClient(c) for (GConfClient *c = (g_type_init(), gconf_client_get_default()); c; g_object_unref(c), c = NULL)

static QByteArray convertKey(QString key)
{
    if (key.startsWith(QString::fromLatin1("/"))) {
        return key.toUtf8();
    } else {
        qWarning() << "Using dot-separated key names with GConfItem is deprecated.";
        qWarning() << "Please use" << QString::fromLatin1("/") + key.replace(QString::fromLatin1("."), QString::fromLatin1("/")) << "instead of" << key;
        return QString(QString::fromLatin1("/") + key.replace(QString::fromLatin1("."), QString::fromLatin1("/"))).toUtf8();
    }
}

static QString convertKey(const char *key)
{
    return QString::fromUtf8(key);
}

static QVariant convertValue(GConfValue *src)
{
    if (!src) {
        return QVariant();
    } else {
        switch (src->type) {
        case GCONF_VALUE_INVALID:
            return QVariant(QVariant::Invalid);
        case GCONF_VALUE_BOOL:
            return QVariant((bool)gconf_value_get_bool(src));
        case GCONF_VALUE_INT:
            return QVariant(gconf_value_get_int(src));
        case GCONF_VALUE_FLOAT:
            return QVariant(gconf_value_get_float(src));
        case GCONF_VALUE_STRING:
            return QVariant(QString::fromUtf8(gconf_value_get_string(src)));
        case GCONF_VALUE_LIST:
            switch (gconf_value_get_list_type(src)) {
            case GCONF_VALUE_STRING: {
                QStringList result;
                for (GSList *elts = gconf_value_get_list(src); elts; elts = elts->next)
                    result.append(QString::fromUtf8(gconf_value_get_string((GConfValue *)elts->data)));
                return QVariant(result);
            }
            default: {
                QList<QVariant> result;
                for (GSList *elts = gconf_value_get_list(src); elts; elts = elts->next)
                    result.append(convertValue((GConfValue *)elts->data));
                return QVariant(result);
            }
            }
        case GCONF_VALUE_SCHEMA:
        default:
            return QVariant();
        }
    }
}

static GConfValue *convertString(const QString &str)
{
    GConfValue *v = gconf_value_new (GCONF_VALUE_STRING);
    gconf_value_set_string (v, str.toUtf8().data());
    return v;
}

static GConfValueType primitiveType (const QVariant &elt)
{
    switch(elt.type()) {
    case QVariant::String:
        return GCONF_VALUE_STRING;
    case QVariant::Int:
        return GCONF_VALUE_INT;
    case QVariant::Double:
        return GCONF_VALUE_FLOAT;
    case QVariant::Bool:
        return GCONF_VALUE_BOOL;
    default:
        return GCONF_VALUE_INVALID;
    }
}

static GConfValueType uniformType(const QList<QVariant> &list)
{
    GConfValueType result = GCONF_VALUE_INVALID;

    foreach (const QVariant &elt, list) {
        GConfValueType elt_type = primitiveType (elt);

        if (elt_type == GCONF_VALUE_INVALID)
            return GCONF_VALUE_INVALID;

        if (result == GCONF_VALUE_INVALID)
            result = elt_type;
        else if (result != elt_type)
            return GCONF_VALUE_INVALID;
    }

    if (result == GCONF_VALUE_INVALID)
        return GCONF_VALUE_STRING;  // empty list.
    else
        return result;
}

static int convertValue(const QVariant &src, GConfValue **valp)
{
    GConfValue *v;

    switch(src.type()) {
    case QVariant::Invalid:
        v = NULL;
        break;
    case QVariant::Bool:
        v = gconf_value_new (GCONF_VALUE_BOOL);
        gconf_value_set_bool (v, src.toBool());
        break;
    case QVariant::Int:
        v = gconf_value_new (GCONF_VALUE_INT);
        gconf_value_set_int (v, src.toInt());
        break;
    case QVariant::Double:
        v = gconf_value_new (GCONF_VALUE_FLOAT);
        gconf_value_set_float (v, src.toDouble());
        break;
    case QVariant::String:
        v = convertString(src.toString());
        break;
    case QVariant::StringList: {
        GSList *elts = NULL;
        v = gconf_value_new(GCONF_VALUE_LIST);
        gconf_value_set_list_type(v, GCONF_VALUE_STRING);
        foreach (const QString &str, src.toStringList())
            elts = g_slist_prepend(elts, convertString(str));
        gconf_value_set_list_nocopy(v, g_slist_reverse(elts));
        break;
    }
    case QVariant::List: {
        GConfValueType elt_type = uniformType(src.toList());
        if (elt_type == GCONF_VALUE_INVALID) {
            v = NULL;
        } else {
            GSList *elts = NULL;
            v = gconf_value_new(GCONF_VALUE_LIST);
            gconf_value_set_list_type(v, elt_type);
            foreach (const QVariant &elt, src.toList()) {
                GConfValue *val = NULL;
                convertValue(elt, &val);  // guaranteed to succeed.
                elts = g_slist_prepend(elts, val);
            }
            gconf_value_set_list_nocopy(v, g_slist_reverse(elts));
        }
        break;
    }
    default:
        return 0;
    }

    *valp = v;
    return 1;
}

void GConfItemPrivate::notify_trampoline(GConfClient *, guint, GConfEntry *entry, gpointer data)
{
    GConfItem *item = (GConfItem *)data;
    item->update_value(true, QString::fromUtf8(entry->key), convertValue(entry->value));
}

/*!
    \class GConfItem
    \brief GConfItem is a simple C++ wrapper for GConf.
    \internal

    Creating a GConfItem instance gives you access to a single GConf
    key.  You can get and set its value, and connect to its
    valueChanged() signal to be notified about changes.

    The value of a GConf key is returned to you as a QVariant, and you
    pass in a QVariant when setting the value.  GConfItem converts
    between a QVariant and GConf values as needed, and according to the
    following rules:

    - A QVariant of type QVariant::Invalid denotes an unset GConf key.

    - QVariant::Int, QVariant::Double, QVariant::Bool are converted to
      and from the obvious equivalents.

    - QVariant::String is converted to/from a GConf string and always
      uses the UTF-8 encoding.  No other encoding is supported.

    - QVariant::StringList is converted to a list of UTF-8 strings.

    - QVariant::List (which denotes a QList<QVariant>) is converted
      to/from a GConf list.  All elements of such a list must have the
      same type, and that type must be one of QVariant::Int,
      QVariant::Double, QVariant::Bool, or QVariant::String.  (A list of
      strings is returned as a QVariant::StringList, however, when you
      get it back.)

    - Any other QVariant or GConf value is essentially ignored.

    \warning GConfItem is as thread-safe as GConf.
*/

/*!
    \internal
    \fn void GConfItem::subtreeChanged(const QString &key, const QVariant &value)

    This signal is emitted whenever some \a key in subtree of this item has changed to \a value.
*/

/*!
    \internal
    \fn void GConfItem::valueChanged()

    This signal is emitted whenever the value of this item has changed.
*/

/*!
    \internal
    Constructs a GConfItem object with the \a parent to access the GConf \a key.

    Note that key names should follow the normal GConf conventions like "/myapp/settings/first".
*/
GConfItem::GConfItem(const QString &key, bool monitor, QObject *parent)
    : QObject (parent)
{
    priv = new GConfItemPrivate;
    priv->key = key;
    priv->monitor = monitor;
    withClient(client) {
        update_value(false, QString(), QVariant());
        if (priv->monitor) {
            QByteArray k = convertKey(priv->key);
            gconf_client_add_dir (client, k.data(), GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
            priv->notify_id = gconf_client_notify_add (client, k.data(), GConfItemPrivate::notify_trampoline, this, NULL, NULL);
        }
    }
}

/*!
    \internal
    Destroys the object.
*/
GConfItem::~GConfItem()
{
    withClient(client) {
        QByteArray k = convertKey(priv->key);
        if (priv->monitor) {
            gconf_client_notify_remove (client, priv->notify_id);
            gconf_client_remove_dir (client, k.data(), NULL);
        }
    }
    delete priv;
}

/*!
    \internal
    Returns the key of this item, as given to the constructor.
*/
QString GConfItem::key() const
{
    return priv->key;
}

/*!
    \internal
    Returns the current value of this item, as a QVariant.
*/
QVariant GConfItem::value() const
{
    return priv->value;
}

/*!
    \internal
    Returns the current value of this item, or \a def if there is no value for it.
*/
QVariant GConfItem::value(const QVariant &def) const
{
    if (priv->value.isNull())
        return def;
    else
        return priv->value;
}

/*!
    \internal
    Set the value of this item to \a val.  If \a val can not be represented in GConf, or GConf
    refuses to accept it for other reasons, the current value is not changed and nothing happens.

    When the new value is different from the old value, the changedValue() signal is emitted on
    this GConfItem as part of calling set(), but other GConfItem:s for the same key do only receive
    a notification once the main loop runs.
*/
void GConfItem::set(const QVariant &val)
{
    withClient(client) {
        QByteArray k = convertKey(priv->key);
        GConfValue *v;
        if (convertValue(val, &v)) {
            GError *error = NULL;

            if (v) {
                gconf_client_set(client, k.data(), v, &error);
                gconf_value_free(v);
            } else {
                gconf_client_unset(client, k.data(), &error);
            }

            if (error) {
                qWarning() << error->message;
                g_error_free(error);
            } else if (priv->value != val) {
                priv->value = val;
                emit valueChanged();
            }

        } else {
            qWarning() << "Can't store a" << val.typeName();
        }
    }
}

/*!
    \internal
    Unset this item. This is equivalent to set an invalid or null value.
*/
void GConfItem::unset()
{
    set(QVariant());
}

/*!
    \internal
    Unset this item and all its sub items recursively.
*/
void GConfItem::recursiveUnset()
{
    withClient(client) {
        QByteArray k = convertKey(priv->key);
        GError *error = NULL;

        GConfUnsetFlags flags;
        gconf_client_recursive_unset(client, k.data(), flags, &error);

        if (error) {
            qWarning() << error->message;
            g_error_free(error);
        } else {
            priv->value = QVariant();
            emit valueChanged();
        }
    }
}

/*!
    \internal
    Return a list of the directories below this item. The returned strings are absolute key names
    like "/myapp/settings".

    A directory is a key that has children.  The same key might also have a value, but that is
    confusing and best avoided.
*/
QList<QString> GConfItem::listDirs() const
{
    QList<QString> children;

    withClient(client) {
        QByteArray k = convertKey(priv->key);
        GSList *dirs = gconf_client_all_dirs(client, k.data(), NULL);
        for (GSList *d = dirs; d; d = d->next) {
            children.append(convertKey((char *)d->data));
            g_free (d->data);
        }
        g_slist_free (dirs);
    }

    return children;
}

/*!
    \internal
    Return a list of entries below this item.  The returned strings are absolute key names like
    "/myapp/settings/first".

    A entry is a key that has a value.  The same key might also have children, but that is confusing
    and is best avoided.
*/
QList<QString> GConfItem::listEntries() const
{
    QList<QString> children;

    withClient(client) {
        QByteArray k = convertKey(priv->key);
        GSList *entries = gconf_client_all_entries(client, k.data(), NULL);
        for (GSList *e = entries; e; e = e->next) {
            children.append(convertKey(((GConfEntry *)e->data)->key));
            gconf_entry_free ((GConfEntry *)e->data);
        }
        g_slist_free (entries);
    }

    return children;
}

/*!
    \internal
*/
void GConfItem::update_value(bool emit_signal, const QString &key, const QVariant &value)
{
    QVariant new_value;

    if (emit_signal)
        emit subtreeChanged(key, value);

    withClient(client) {
        GError *error = NULL;
        QByteArray k = convertKey(priv->key);
        GConfValue *v = gconf_client_get(client, k.data(), &error);

        if (error) {
            qWarning() << error->message;
            g_error_free (error);
            new_value = priv->value;
        } else {
            new_value = convertValue(v);
            if (v)
                gconf_value_free(v);
        }
    }

    if (new_value != priv->value) {
        priv->value = new_value;
        if (emit_signal)
            emit valueChanged();
    }
}

QT_END_NAMESPACE

#endif // QT_NO_GCONFLAYER
