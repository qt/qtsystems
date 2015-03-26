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

#include <stdio.h>

#include <QtCore>
#include <QTextStream>
#include <qservicemanager.h>
#include <servicemetadata_p.h>
#include <QString>
#include <QDir>

static const char * const errorTable[] = {
    "No error", //0
    "Storage read error",
    "Invalid service location",
    "Invalid service xml",
    "Invalid service interface descriptor",
    "Service already exists",
    "Interface implementation already exists",
    "Loading of plug-in failed",
    "Service or interface not found",
    "Insufficient capabilities to access service",
    "Unknown error",
};

class CommandProcessor : public QObject
{
    Q_OBJECT

public:
    CommandProcessor(QObject *parent = 0);
    ~CommandProcessor();

    void execute(const QStringList &options, const QString &cmd, const QStringList &args);
    void showUsage();
    static void showUsage(QTextStream *stream);
    int errorCode();

public slots:
    void browse(const QStringList &args);
    void search(const QStringList &args);
    void add(const QStringList &args);
    void remove(const QStringList &args);
    void dbusservice(const QStringList &args);
    void setdefault(const QStringList &args);

private:
    bool setOptions(const QStringList &options);
    void setErrorCode(int error);
    void showAllEntries();
    void showInterfaceInfo(const QServiceFilter &filter);
    void showInterfaceInfo(QList<QServiceInterfaceDescriptor> descriptors);
    void showServiceInfo(const QString &service);

    QServiceManager *serviceManager;
    QTextStream *stdoutStream;
    int m_error;
};

CommandProcessor::CommandProcessor(QObject *parent)
    : QObject(parent),
      serviceManager(0),
      stdoutStream(new QTextStream(stdout)),
      m_error(0)
{
}

CommandProcessor::~CommandProcessor()
{
    delete stdoutStream;
}

void CommandProcessor::execute(const QStringList &options, const QString &cmd, const QStringList &args)
{
    if (cmd.isEmpty()) {
        *stdoutStream << "Error: no command given\n\n";
        showUsage();
        return;
    }

    if (!setOptions(options))
        return;

    int methodIndex = metaObject()->indexOfMethod(cmd.toLatin1() + "(QStringList)");
    if (methodIndex < 0) {
        *stdoutStream << "Bad command: " << cmd << "\n\n";
        showUsage();
        return;
    }

    if (!metaObject()->method(methodIndex).invoke(this, Q_ARG(QStringList, args)))
        *stdoutStream << "Cannot invoke method for command:" << cmd << '\n';
}

void CommandProcessor::showUsage()
{
    showUsage(stdoutStream);
}

void CommandProcessor::showUsage(QTextStream *stream)
{
    *stream << "Usage: servicefw [options] <command> [command parameters]\n\n"
            "Commands:\n"
            "\tbrowse         List all registered services\n"
            "\tsearch         Search for a service or interface\n"
            "\tadd            Register a service\n"
            "\tremove         Unregister a service\n"
            "\tdbusservice    Generates a .service file for D-Bus service autostart\n"
            "\n"
            "Options:\n"
            "\t--system       Use the system-wide services database instead of the\n"
            "\t               user-specific database\n"
            "\t--user         Use the user-specific services database for add/remove.\n"
            "\t               This is the default\n"
            "\n";
}

void CommandProcessor::browse(const QStringList &args)
{
    Q_UNUSED(args);
    showAllEntries();
}

void CommandProcessor::setdefault(const QStringList &args)
{
    if (args.isEmpty() || args.count() < 2) {
        *stdoutStream << "Usage:\n\tsetdefault <interface> <service>\n\n"
                         "Examples:\n"
                         "\tsetdefault com.nokia.SomeInterface foo";
        return;
    }


    const QString &serviceInterface = args[0];
    const QString &service = args[1];
    bool res = serviceManager->setInterfaceDefault(service, serviceInterface);
    if(!res) {
        *stdoutStream << "Failed to set interface default"
                      << errorTable[serviceManager->error()];
    }
}

void CommandProcessor::search(const QStringList &args)
{
    if (args.isEmpty()) {
        *stdoutStream << "Usage:\n\tsearch <service|interface [version]>\n\n"
                "Examples:\n"
                "\tsearch SomeService\n"
                "\tsearch com.nokia.SomeInterface\n"
                "\tsearch com.nokia.SomeInterface 3.5\n"
                "\tsearch com.nokia.SomeInterface 3.5+\n";
        return;
    }

    const QString &name = args[0];
    if (name.contains('.')) {
        QServiceFilter filter;
        if (args.count() > 1) {
            const QString &version = args[1];
            bool minimumMatch = version.endsWith('+');
            filter.setInterface(name,
                    minimumMatch ? (version.mid(0, version.length()-1)) : version,
                    minimumMatch ? QServiceFilter::MinimumVersionMatch : QServiceFilter::ExactVersionMatch);
            if (filter.interfaceName().isEmpty()) { // setInterface() failed
                *stdoutStream << "Error: invalid interface version: " << version << '\n';
                return;
            }
        } else {
            filter.setInterface(name);
        }
        showInterfaceInfo(filter);
    } else {
        showServiceInfo(name);
    }
}


void CommandProcessor::add(const QStringList &args)
{
    if (args.isEmpty()) {
        *stdoutStream << "Usage:\n\tadd <service-xml-file>\n";
        return;
    }

    const QString &xmlPath = args[0];
    if (!QFile::exists(xmlPath)) {
        *stdoutStream << "Error: cannot find file " << xmlPath << '\n';
        setErrorCode(11);
        return;
    }

    if (serviceManager->addService(xmlPath)) {
        *stdoutStream << "Registered service at " << xmlPath << '\n';
    } else {
        int error = serviceManager->error();
        if (error > 10) //map anything larger than 10 to 10
            error = 10;
        *stdoutStream << "Error: cannot register service at " << xmlPath
                << " (" << errorTable[error] << ")" << '\n';

        setErrorCode(error);
    }
}

void CommandProcessor::remove(const QStringList &args)
{
    if (args.isEmpty()) {
        *stdoutStream << "Usage:\n\tremove <service-name>\n";
        return;
    }

    const QString &service = args[0];
    if (serviceManager->removeService(service))
        *stdoutStream << "Unregistered service " << service << '\n';
    else
        *stdoutStream << "Error: cannot unregister service " << service << '\n';
}

void CommandProcessor::dbusservice(const QStringList &args)
{
    if (args.isEmpty() || args.size() == 1) {
        *stdoutStream << "Usage:\n\tdbusservice <service-xml-file> <service-executable>\n";
        return;
    }

#if defined(QT_NO_DBUS)
    *stdoutStream << "Error: no D-Bus module found in Qt\n";
    return;
#endif

    const QString &xmlPath = args[0];
    if (!QFile::exists(xmlPath)) {
        *stdoutStream << "Error: cannot find xml file at " << xmlPath << '\n';
        return;
    }

    const QString &servicePath = args[1];
    if (!QFile::exists(servicePath)) {
        *stdoutStream << "Error: cannot find service file " << servicePath << '\n';
        return;
    }

    QFile *f = new QFile(xmlPath);
    ServiceMetaData parser(f);
    if (!parser.extractMetadata()) {
        *stdoutStream << "Error: invalid service xml at " << xmlPath << '\n';
        return;
    }

    const ServiceMetaDataResults results = parser.parseResults();
    if (results.type != QService::InterProcess) {
        *stdoutStream << "Error: not an inter-process service xml at " << xmlPath << '\n';
        return;
    }

    QString path;
    if (serviceManager->scope() == QService::UserScope) {
        // the d-bus xdg environment variable for the local service paths
        const QByteArray xdgPath = qgetenv("XDG_DATA_HOME");
        if (xdgPath.isEmpty()) {
            // if not supplied generate in default
            QDir dir(QDir::homePath());
            dir.mkpath(".local/share/dbus-1/services/");
            path = QDir::homePath() + "/.local/share/dbus-1/services/";
        } else {
            path = QString::fromLocal8Bit(xdgPath);
        }
    } else {
        path = "/usr/share/dbus-1/services/";
    }

    const QString &name = "com.nokia.qtmobility.sfw." + results.name;
    const QString &exec = QFileInfo(args[1]).absoluteFilePath();

    QStringList names;
    foreach (const QServiceInterfaceDescriptor &serviceInterface, results.interfaces) {
        names << serviceInterface.interfaceName();
    }
    names.removeDuplicates();

    for (int i = 0; i < names.size(); i++) {
        const QString &file = path + names.at(i) + "." + results.name + ".service";
        QFile data(file);
        if (data.open(QFile::WriteOnly)) {
            QTextStream out(&data);
            out << "[D-BUS Service]\n"
                << "Name=" << name << '\n'
                << "Exec=" << exec;
            data.close();
        } else {
            *stdoutStream << "Error: cannot write to " << file << " (insufficient permissions)" << '\n';
            return;
        }

        *stdoutStream << "Generated D-Bus autostart file " << file << '\n';
    }
}

bool CommandProcessor::setOptions(const QStringList &options)
{
    if (serviceManager)
        delete serviceManager;

    QService::Scope scope = QService::UserScope;

    QStringList opts = options;
    QMutableListIterator<QString> i(opts);
    while (i.hasNext()) {
        QString option = i.next();
        if (option == "--system") {
            scope = QService::SystemScope;
            i.remove();
        } else if (option == "--user") {
            scope = QService::UserScope;
            i.remove();
        }
    }

    if (!opts.isEmpty()) {
        *stdoutStream << "Bad options: " << opts.join(QLatin1Char(' ')) << "\n\n";
        showUsage();
        return false;
    }

    serviceManager = new QServiceManager(scope, this);

    return true;
}

void CommandProcessor::showAllEntries()
{
    QStringList services = serviceManager->findServices();
    if (services.isEmpty())
        *stdoutStream << "No services found.\n";
    else if (services.count() == 1)
        *stdoutStream << "Found 1 service.\n\n";
    else
        *stdoutStream << "Found " << services.count() << " services.\n\n";

    foreach (const QString &service, services)
        showServiceInfo(service);
}

void CommandProcessor::showInterfaceInfo(const QServiceFilter &filter)
{
    QString serviceInterface = filter.interfaceName();
    if (filter.majorVersion() >= 0 && filter.minorVersion() >= 0) {
        serviceInterface += QString(" %1.%2").arg(filter.majorVersion()).arg(filter.minorVersion());
        if (filter.versionMatchRule() == QServiceFilter::MinimumVersionMatch)
            serviceInterface += '+';
    }

    QList<QServiceInterfaceDescriptor> descriptors = serviceManager->findInterfaces(filter);
    if (descriptors.isEmpty()) {
        *stdoutStream << "Interface " << serviceInterface << " not found.\n";
        return;
    }

    *stdoutStream << serviceInterface << " is implemented by:\n";
    foreach (const QServiceInterfaceDescriptor &desc, descriptors)
        *stdoutStream << '\t' << desc.serviceName() << '\n';
}

void CommandProcessor::showServiceInfo(const QString &service)
{
    QList<QServiceInterfaceDescriptor> descriptors = serviceManager->findInterfaces(service);
    if (descriptors.isEmpty()) {
        *stdoutStream << "Service " << service << " not found.\n";
        return;
    }

    QList<QServiceInterfaceDescriptor> pluginDescs;
    QList<QServiceInterfaceDescriptor> ipcDescs;
    foreach (const QServiceInterfaceDescriptor &desc, descriptors) {
        int serviceType = desc.attribute(QServiceInterfaceDescriptor::ServiceType).toInt();
        if (serviceType == QService::Plugin)
            pluginDescs.append(desc);
        else
            ipcDescs.append(desc);
    }

    if (pluginDescs.size() > 0) {
        *stdoutStream << service;
        if (ipcDescs.size() > 0)
            *stdoutStream << " (Plugin):\n";
        else
            *stdoutStream << ":\n";

        QString description = pluginDescs[0].attribute(
                QServiceInterfaceDescriptor::ServiceDescription).toString();
        if (!description.isEmpty())
            *stdoutStream << '\t' << description << '\n';

        *stdoutStream << "\tPlugin Library: ";
        showInterfaceInfo(pluginDescs);
    }

    if (ipcDescs.size() > 0) {
        *stdoutStream << service;
        if (pluginDescs.size() > 0)
            *stdoutStream << " (IPC):\n";
        else
            *stdoutStream << ":\n";

        QString description = ipcDescs[0].attribute(
                QServiceInterfaceDescriptor::ServiceDescription).toString();
        if (!description.isEmpty())
            *stdoutStream << '\t' << description << '\n';

        *stdoutStream << "\tIPC Address: ";
        showInterfaceInfo(ipcDescs);
    }
}

void CommandProcessor::showInterfaceInfo(QList<QServiceInterfaceDescriptor> descriptors)
{
    QList<QServiceInterfaceDescriptor> systemList;
    QList<QServiceInterfaceDescriptor> userList;

    foreach (const QServiceInterfaceDescriptor &desc, descriptors) {
        if (desc.scope() == QService::SystemScope)
            systemList.append(desc);
        else
            userList.append(desc);
    }

    if (userList.size() > 0) {
        *stdoutStream << userList[0].attribute(QServiceInterfaceDescriptor::Location).toString() << '\n'
                      << "\tUser Implements:\n";

        foreach (const QServiceInterfaceDescriptor &desc, userList) {
            QStringList capabilities = desc.attribute(
                    QServiceInterfaceDescriptor::Capabilities).toStringList();

            *stdoutStream << "\t    " << desc.interfaceName() << ' '
                << desc.majorVersion() << '.' << desc.minorVersion()
                << (capabilities.isEmpty() ? "" : "\t{" + capabilities.join(", ") + "}") << "\n";
        }
    }

    if (systemList.size() > 0) {
        if (userList.size() < 1)
            *stdoutStream << systemList[0].attribute(QServiceInterfaceDescriptor::Location).toString() << '\n';

        *stdoutStream << "\tSystem Implements:\n";

        foreach (const QServiceInterfaceDescriptor &desc, systemList) {
            QStringList capabilities = desc.attribute(
                    QServiceInterfaceDescriptor::Capabilities).toStringList();

            *stdoutStream << "\t    " << desc.interfaceName() << ' '
                << desc.majorVersion() << '.' << desc.minorVersion()
                << (capabilities.isEmpty() ? "" : "\t{" + capabilities.join(", ") + "}") << "\n";
        }
    }
}

int CommandProcessor::errorCode()
{
    return m_error;
}

void CommandProcessor::setErrorCode(int error)
{
    m_error = error;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args = QCoreApplication::arguments();

    // check for at least one non-option argument
    bool exec = false;

    QStringList options;
    for (int i=1; i<args.count(); i++) {
        if (args[i].startsWith("--"))
            options += args[i];
        else
            exec = true;
    }

    if (!exec || args.count() == 1 || args.value(1) == "--help" || args.value(1) == "-h") {
        QTextStream stream(stdout);
        CommandProcessor::showUsage(&stream);
        return 0;
    }

    CommandProcessor processor;
    processor.execute(options, args.value(options.count() + 1), args.mid(options.count() + 2));
    return processor.errorCode();
}

#include "servicefw.moc"
