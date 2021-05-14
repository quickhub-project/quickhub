/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QStandardPaths>
#include <QDebug>
#include "PluginManager.h"
#include <QDir>
#include <initializer_list>
#include <signal.h>
#include <unistd.h>



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString path = a.applicationDirPath()+"/plugins";
    a.addLibraryPath(path);
    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption portOption(QStringList()<<"p"<<"port", "Sets the port where the server will listen for websocket connections.", "port", "4711");
    QCommandLineOption folderOption(QStringList()<<"f"<<"folder", "Sets the folder where the server will store the cloud data.", "folder", QStandardPaths::standardLocations(QStandardPaths::DataLocation).at(0)+"/v1.3/");
    parser.addOption(portOption);
    parser.addOption(folderOption);
    parser.process(a);

    QStringList optionNames = parser.optionNames() ;
    QVariantMap parameters;
    foreach (QString optionName, optionNames)
    {
        parameters.insert(optionName, parser.value(optionName));
    }

    PluginManager::getInstance()->loadPlugins(parameters);
    return a.exec();
}
