/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "IResourceHandler.h"
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Authentication/User.h"
#include <QUuid>
int IResourceHandler::instanceCount = 0;
IResourceHandler::IResourceHandler(QString resourceType, QObject *parent) : QObject(parent)
{
    _resourceType = resourceType;
    connect(AuthenticationService::instance(), &AuthenticationService::sessionClosed, this, &IResourceHandler::sessionClosed);
}


bool IResourceHandler::attachHandle(QString token, ISocket *handle)
{
    if(!isPermitted(token))
        return false;

    handle->setParent(this);
    connect(handle, &ISocket::messageReceived, this, &IResourceHandler::messageReceived);
    connect(handle, &ISocket::disconnected,    this, &IResourceHandler::handleDisconnected);

    _handles.insert(handle);
    _tokenToHandleMap.insert(token, handle);

    QVariantMap msg;
    msg["command"] = _resourceType+":attach:success";
    handle->sendVariant(msg);
    initHandle(handle);
    return true;
}

QString IResourceHandler::getUUID()
{
    return "";
}

void IResourceHandler::initHandle(ISocket *handle)
{
    Q_UNUSED (handle)
}

bool IResourceHandler::isPermitted(QString token) const
{
    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);
    return !identity.isNull();
}

void IResourceHandler::detachHandle(ISocket *handle)
{
    if(_handles.contains(handle))
    {
        _handles.remove(handle);
        _tokenToHandleMap.remove(_tokenToHandleMap.key(handle));
        handle->setParent(nullptr);
        disconnect(handle, &ISocket::messageReceived, this, &IResourceHandler::messageReceived);
        disconnect(handle, &ISocket::disconnected,    this, &IResourceHandler::handleDisconnected);

        QVariantMap answer;
        answer["command"] = _resourceType+":detach:success";
        handle->sendVariant(answer);

        if(_handles.size() == 0)
        {
            //destroyed signal is catched in the handler. Object will be removed from the handler map.
            this->deleteLater();
        }
    }
}

bool IResourceHandler::dynamicContent() const
{
    return false;
}

void IResourceHandler::handleDisconnected()
{
    ISocket* handle = qobject_cast<ISocket *>(sender());
    if (handle)
    {
        detachHandle(handle);
    }
}

void IResourceHandler::messageReceived(QVariant message)
{
    ISocket* handle = qobject_cast<ISocket*>(sender());
    if(!handle)
        return;

    QVariantMap msgMap = message.toMap();

    QString command = msgMap["command"].toString();

    if(command == "ACK")
    {
        return;
    }

    QStringList commandTokens = command.split(":");
    if(commandTokens.size() > 1 && commandTokens[1] == "detach")
    {
        detachHandle(handle);
        return;
    }
    handleMessage(message, handle);
}

void IResourceHandler::sessionClosed(QString userID, QString token)
{
    Q_UNUSED(userID)
    QList<ISocket*> _sockets = _tokenToHandleMap.values(token);
    QListIterator<ISocket*> it(_sockets);
    while(it.hasNext())
    {
        detachHandle(it.next());
    }
}


void IResourceHandler::deployToAll(QVariantMap msg, ISocket *sender)
{
    QSetIterator<ISocket*> it(_handles);

    while(it.hasNext())
    {
        ISocket* receiver = it.next();
        msg["reply"] = receiver == sender;
        receiver->sendVariant(msg);
    }
}

void IResourceHandler::handleError(QString command, IResource::ResourceError error, ISocket *socket, QVariantMap parameters)
{
    QVariantMap answer;
    if (error >= 0)
    {
        answer["command"] = command += ":success";
        socket->sendVariant(answer);
        return;
    }

    answer["command"] = command + ":failed";
    answer["errorcode"] = error;
    QString errorString;
    switch(error)
    {
        case IResource::NO_ERROR: errorString = "No error";break;
        case IResource::PERMISSION_DENIED: errorString = "Permission Denied."; break;
        case IResource::UNKNOWN_ITEM :errorString = "Unknown Item"; break;
        case IResource::INVALID_PARAMETERS :errorString = "Invalid or missing parameters"; break;
        case IResource::STORAGE_ERROR :errorString = "Storage error"; break;
        case IResource::UNKNOWN_ERROR:
            errorString = "Unknown error";
            break;
        case IResource::NOT_SUPPORTED:
            errorString = "Not supported";
            break;
        }

    answer["errorstring"] = errorString;
    if(!parameters.isEmpty())
        answer["parameters"] = parameters;

    socket->sendVariant(answer);
}
