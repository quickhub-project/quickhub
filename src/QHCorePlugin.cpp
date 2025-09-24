/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "QHCorePlugin.h"
#include "SocketApi/SocketServer.h"
#include "QStandardPaths"
#include "Server/Logging/Logger.h"
#include "Server/Devices/DeviceService.h"
#include "Server/Devices/DeviceService.h"
#include "Server/Services/ServiceManager.h"

#include "Server/Resources/ListResource/ListResourceFactory.h"
#include "Server/Resources/ObjectResource/ObjectResourceFactory.h"
#include "Server/Resources/ResourceManager/ResourceManager.h"

#include "PluginManager.h"

QHCorePlugin::QHCorePlugin(QObject* parent) : IPlugin(parent)
{
}

bool QHCorePlugin::init(QVariantMap parameters)
{
    int port = parameters.value("p", 4711).toInt();
    QString path
        = parameters
              .value("f",
                     QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation).at(0)
                         + "/v1.3/")
              .toString();
    ServiceManager::instance()->registerService(new DeviceService(this));
    SocketServer::instance()->start(path, static_cast<quint16>(port));
    QList<IListResourceStorageFactory*> listStoragePlugins = PluginManager::getInstance()->getObjects<IListResourceStorageFactory>();
    if(listStoragePlugins.count() > 0)
    {
        SocketServer::instance()->setListResourceStorageFactory(listStoragePlugins.at(0));
    }

    QList<IObjectResourceStorageFactory*> objectStoragePlugins = PluginManager::getInstance()->getObjects<IObjectResourceStorageFactory>();
    if(objectStoragePlugins.count() > 0)
    {
        SocketServer::instance()->setObjectResourceStorageFactory(objectStoragePlugins.at(0));
    }

    qInstallMessageHandler(Logger::handleMessage);
    return true;
}

bool QHCorePlugin::shutdown()
{
    return false;
}

QString QHCorePlugin::getPluginName()
{
    return "QHCore";
}
