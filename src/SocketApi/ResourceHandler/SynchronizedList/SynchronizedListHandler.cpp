/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "SynchronizedListHandler.h"
#include <QDateTime>
#include <QDir>
#include <QUuid>
#include <QJsonDocument>
#include "Server/Resources/ResourceManager/ResourceManager.h"
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Authentication/User.h"
#include "Server/Authentication/AuthentificationService.h"

SynchronizedListHandler::SynchronizedListHandler(QSharedPointer<ListResource> resource) : IResourceHandler(resource->getResourceType(), resource.data()),
    _resource(resource)
{
    connect(_resource.data(), &ListResource::itemAppended, this, &SynchronizedListHandler::itemAppended);
    connect(_resource.data(), &ListResource::itemInserted, this, &SynchronizedListHandler::itemInserted);
    connect(_resource.data(), &ListResource::listAppended, this, &SynchronizedListHandler::listAppended);
    connect(_resource.data(), &ListResource::listCleared,  this, &SynchronizedListHandler::listCleared);
    connect(_resource.data(), &ListResource::listDeleted,  this, &SynchronizedListHandler::listDeleted);
    connect(_resource.data(), &ListResource::metadataChanged,  this, &SynchronizedListHandler::metadataChanged);
    connect(_resource.data(), &ListResource::itemSet,      this, &SynchronizedListHandler::itemSet);
    connect(_resource.data(), &ListResource::itemRemoved,  this, &SynchronizedListHandler::itemRemoved);
    connect(_resource.data(), &ListResource::propertySet,  this, &SynchronizedListHandler::propertySet);
     connect(_resource.data(), &ListResource::reset,  this, &SynchronizedListHandler::listResetted);
}

SynchronizedListHandler::~SynchronizedListHandler()
{
}

void SynchronizedListHandler::initHandle(ISocket* handle)
{
    QVariantMap msg;
    QVariantMap parameters;

    if(_resource->getCount() > 0)
    {
        msg["command"] = "synclist:init";
        parameters["count"] = _resource->getCount();
    }
    else
    {
        msg["command"] = "synclist:dump";
        parameters["data"] = _resource.data()->getListData();
    }

    parameters["metadata"] = _resource.data()->getMetadata();
    msg["parameters"] = parameters;
    handle->sendVariant(msg);
}

bool SynchronizedListHandler::dynamicContent() const
{
    return _resource->dynamicContent();
}

void SynchronizedListHandler::handleMessage(QVariant message, ISocket *handle)
{
    QVariantMap msg     = message.toMap();

    QString     command     = msg["command"].toString();
    QString     token       = msg["token"].toString();
    QVariantMap parameters  = msg["parameters"].toMap();
    QVariant    data        = parameters["data"];
    msg.remove("token");

    if(command == "synclist:dump")
    {
        parameters["data"] = _resource.data()->getListData();
        parameters["metadata"] = _resource.data()->getMetadata();
        msg["parameters"] = parameters;
        handle->sendVariant(msg);
        return;
    }

    if(command == "synclist:get")
    {
        int from = parameters["from"].toInt();
        int count = parameters["count"].toInt();
        if(from < 0 || count <= 0 || from+count-1  >= _resource->getCount())
        {
            return;
        }

        QVariantList data;

        for(int i = from; i < from+count; i++)
        {
            data << _resource->getItem(i);
        }

        parameters["data"] = data;

        msg["parameters"] = parameters;
        handle->sendVariant(msg);
        return;
    }

    if(command == "synclist:filter")
    {
        if(_resource->dynamicContent())
            _resource->setFilter(data.toMap());
    }

    // ############### MODIFIER

    if(command == "synclist:append")
    {
        disconnect(_resource.data(), &ListResource::itemAppended, this, &SynchronizedListHandler::itemAppended);
        ListResource::ModificationResult result = _resource.data()->appendItem(data, token);
        connect(_resource.data(), &ListResource::itemAppended, this, &SynchronizedListHandler::itemAppended);
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            parameters["data"] = result.data;
            msg["parameters"] = parameters;
            deployToAll(msg, handle);
            return;
        }
    }

    if(command == "synclist:insertat")
    {
        bool ok;
        int index = parameters["index"].toInt(&ok);
        if(!ok)
        {
            qWarning()<<"SocketListHandler: Invalid data (index not an integer?)";
            return;
        }

        disconnect(_resource.data(), &ListResource::itemInserted, this, &SynchronizedListHandler::itemInserted);
        ListResource::ModificationResult result = _resource.data()->insertAt(data, index, token);
        connect(_resource.data(), &ListResource::itemInserted, this, &SynchronizedListHandler::itemInserted);
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            parameters["data"] = result.data;
            msg["parameters"] = parameters;
            deployToAll(msg, handle);
            return;
        }
    }


    if(command == "synclist:appendlist")
    {
        QVariantList dataList = data.toList();

        disconnect(_resource.data(), &ListResource::listAppended, this, &SynchronizedListHandler::listAppended);
        ListResource::ModificationResult result = _resource->appendList(dataList, token);
        connect(_resource.data(), &ListResource::listAppended, this, &SynchronizedListHandler::listAppended);
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            parameters["data"] = result.data;
            msg["parameters"] = parameters;
            deployToAll(msg, handle);
            return;
        }
    }

    if(command == "synclist:clear")
    {
        disconnect(_resource.data(), &ListResource::listCleared, this, &SynchronizedListHandler::listCleared);
        ListResource::ModificationResult result = _resource->clearList(token);
        connect(_resource.data(), &ListResource::listCleared, this, &SynchronizedListHandler::listCleared);
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            deployToAll(msg, handle);
            return;
        }
    }


    if(command == "synclist:delete")
    {
        disconnect(_resource.data(), &ListResource::listDeleted, this, &SynchronizedListHandler::listDeleted);
        ListResource::ModificationResult result = _resource->deleteList(token);
        connect(_resource.data(), &ListResource::listDeleted, this, &SynchronizedListHandler::listDeleted);
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            deployToAll(msg, handle);
            return;
        }
    }

    if(command == "synclist:property:set")
    {
        int index = parameters["index"].toInt();
        QString uuid = parameters["uuid"].toString();
        QString property = parameters["property"].toString();

        disconnect(_resource.data(), &ListResource::propertySet, this, &SynchronizedListHandler::propertySet);
        ListResource::ModificationResult result = _resource->setProperty(property, data, index, uuid, token);
        connect(_resource.data(), &ListResource::propertySet, this, &SynchronizedListHandler::propertySet);

        parameters["lastupdate"] = result.data.toMap()["lastupdate"];
        parameters["userid"] = result.data.toMap()["userid"];
        parameters["username"] = result.data.toMap()["username"];

        msg["parameters"] = parameters;
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            deployToAll(msg, handle);
            return;
        }
    }

    if(command == "synclist:set")
    {
        int index = parameters["index"].toInt();
        QString uuid = parameters["uuid"].toString();

        disconnect(_resource.data(), &ListResource::itemSet, this, &SynchronizedListHandler::itemSet);
        ListResource::ModificationResult result = _resource->set(data, index, uuid, token);
        connect(_resource.data(), &ListResource::itemSet, this, &SynchronizedListHandler::itemSet);

        parameters["data"] = result.data;
        msg["parameters"] = parameters;
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            deployToAll(msg, handle);
            return;
        }
    }

    if(command == "synclist:metadata:set")
    {
         disconnect(_resource.data(), &ListResource::metadataChanged,  this, &SynchronizedListHandler::metadataChanged);
        _resource.data()->setMetadata(parameters["metadata"]);
        connect(_resource.data(), &ListResource::metadataChanged,  this, &SynchronizedListHandler::metadataChanged);
        deployToAll(msg, handle);
    }

    if(command == "synclist:remove")
    {
        int index = parameters["index"].toInt();
        QString uuid = parameters["uuid"].toString();

        disconnect(_resource.data(), &ListResource::itemRemoved, this, &SynchronizedListHandler::itemRemoved);
        ListResource::ModificationResult result = _resource->removeItem(uuid, token, index);
        connect(_resource.data(), &ListResource::itemRemoved, this, &SynchronizedListHandler::itemRemoved);
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            deployToAll(msg, handle);
            return;
        }
    }
}



void SynchronizedListHandler::metadataChanged()
{
    QVariantMap msg;
    msg["command"] = "synclist:metadata:set";
    QVariantMap parameters;
    parameters["metadata"] =  _resource.data()->getMetadata();
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void SynchronizedListHandler::itemAppended(QVariant data, iIdentityPtr user)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:append";
    QVariantMap parameters;
    parameters["data"] =  data;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void SynchronizedListHandler::itemInserted(QVariant data, int index, iIdentityPtr user)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:insertat";
    QVariantMap parameters;
    parameters["data"] =  data;
    parameters["index"] =  index;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void SynchronizedListHandler::listAppended(QVariantList data, iIdentityPtr user)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:appendlist";
    QVariantMap parameters;
    parameters["data"] =  data;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void SynchronizedListHandler::itemRemoved(int index, QString uuid, iIdentityPtr user)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:remove";
    QVariantMap parameters;
    parameters["uuid"] =  uuid;
    parameters["index"] =  index;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void SynchronizedListHandler::listDeleted(iIdentityPtr user)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:delete";
    deployToAll(msg);
}

void SynchronizedListHandler::listCleared(iIdentityPtr user)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:clear";
    deployToAll(msg);
}

void SynchronizedListHandler::itemSet(QVariant data, int index, QString uuid, iIdentityPtr user)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:set";
    QVariantMap parameters;
    parameters["uuid"] =  uuid;
    parameters["index"] =  index;
    parameters["data"] =  data;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void SynchronizedListHandler::propertySet(QString property, QVariant data, int index, QString uuid, iIdentityPtr user, qint64 timestamp)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:property:set";
    QVariantMap parameters;
    parameters["uuid"] =  uuid;
    parameters["index"] =  index;
    parameters["property"] =  property;
    if (data.typeId() == QVariant::Map)
        parameters["data"] =  data.toMap()["data"].toMap()[property];
    else
        parameters["data"] = data;

    parameters["lastupdate"] = timestamp;
    if(!user.isNull())
    {
        parameters["userid"] = user->identityID();
        //parameters["username"] = user->userName();
    }
    msg["parameters"] = parameters;

    deployToAll(msg);
}

void SynchronizedListHandler::listResetted()
{
    QVariantMap msg;
    QVariantMap parameters;
    msg["command"] = "synclist:init";
    parameters["count"] =  _resource->getCount();
    msg["parameters"] = parameters;
    deployToAll(msg);
}

bool SynchronizedListHandler::isPermitted(QString token) const
{
    return  _resource->isPermittedToRead(AuthenticationService::instance()->validateToken(token));
}
