/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ServiceRequestHandler.h"
#include "../Server/Services/ServiceManager.h"
#include <QVariantMap>
#include <QDebug>


ServiceRequestHandler::ServiceRequestHandler(QObject *parent) : IRequestHandler(parent)
{
    connect(ServiceManager::instance(), &ServiceManager::serviceAdded, [=](QString serviceName)
    {
        connect(ServiceManager::instance()->service(serviceName), &IService::response, this, &ServiceRequestHandler::sendAnswer);
    });

    auto tmpServices = ServiceManager::instance()->getServices();
    QMapIterator<QString, IService*> it(tmpServices);
    while(it.hasNext())
    {
        connect(it.next().value(), &IService::response, this, &ServiceRequestHandler::sendAnswer);
    }
}

bool ServiceRequestHandler::handleRequest(QVariantMap message, ISocket *socket)
{
    QString command         = message["command"].toString();
    if(!command.startsWith("call"))
        return false;

    //     |
    // call:<service>/<callName>

    QStringList tokens = command.split(":");
    if(tokens.count() < 2)
        return false;

    command = tokens.at(0);
    QString selector = tokens.at(1);
    QStringList selectorTokens = selector.split("/");

    QString serviceName;
    serviceName = selectorTokens.first();
    QString call;
    call = selector.right(selector.size() - serviceName.size() - 1);

    QString token           = message["token"].toString();
    QVariantMap payload     = message["payload"].toMap();

    QVariant arg            = payload["arg"];
    QString uid             = payload["uid"].toString();

    if(serviceName.isEmpty() || call.isEmpty())
    {
        qCritical()<<"ServiceRequestHandler: Incomplete parameters.";
        return false;
    }


    if(command == "call" )
    {
        IService* service = ServiceManager::instance()->service(serviceName);
        if(service == nullptr)
        {
            qWarning() << "Unavailable servcie: "+serviceName;
            return false;
        }

        _socketMap.insert(uid, socket);
        if(!service->call(call, token, uid, arg))
        {
            _socketMap.remove(uid);
        }
    }

    return false;
}

QStringList ServiceRequestHandler::getSupportedCommands()
{
    return QStringList();
}

void ServiceRequestHandler::sendAnswer(QString uid, QVariant result)
{
    ISocket* socket = _socketMap.value(uid, nullptr);
    if(socket)
    {
        QVariantMap response;
        response["uid"] = uid;
        response["data"] = result;
        socket->sendVariant(response);
    }

    _socketMap.remove(uid);
}
