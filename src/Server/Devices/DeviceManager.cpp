/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "DeviceManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QRandomGenerator>
#include "../Authentication/AuthentificationService.h"
#include "../Authentication/User.h"

Q_GLOBAL_STATIC(DeviceManager, deviceManager);

bool DeviceManager::registerDevice(iDevicePtr device)
{
    QString uuid = device->uuid();
    _shortIDtoUid.insert(device->shortId().toUpper(), uuid);
    connect(device.data(), &IDevice::deregistered, this, &DeviceManager::deregisterDevice);
    qInfo()<<"Device registered: "<<uuid;
    _deviceMap.insert(uuid, device);
    // Handles are listening to that signal.
    // The handle to which that device belongs will grab it via getDeviceByUuid()
    Q_EMIT deviceRegistered(uuid);
    if(_preparedHooks.contains(uuid))
        if(hook(_preparedHooks.value(uuid), uuid) == Err::NO_ERROR)
            _preparedHooks.remove(uuid);

    return true;
}

bool DeviceManager::exists(QString uuid)
{
    return _deviceMap.contains(uuid);
}

Err::CloudError DeviceManager::setDeviceMapping(QString token, QString mapping, QString uuid, bool force)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull() || !user->isAuthorizedTo(MANAGE_DEVICES))
        return Err::PERMISSION_DENIED;

    if(uuid.isEmpty()) // remove existing mapping
    {
        return unhook(mapping);
    }
    else
    {
        return hook(mapping, uuid , force);
    }
}

Err::CloudError DeviceManager::setDeviceMappingByShortId(QString token, QString mapping, QString shortID, bool force)
{
    QString uuid = _shortIDtoUid.value(shortID.toUpper());
    if(uuid.isEmpty())
        return Err::INVALID_DATA;

    return setDeviceMapping(token, mapping, uuid, force);
}

Err::CloudError DeviceManager::prepareDeviceMapping(QString token, QString mapping, QString uuid)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull() || !user->isAuthorizedTo(MANAGE_DEVICES))
        return Err::PERMISSION_DENIED;

    if(hook(mapping, uuid) != Err::NO_ERROR)
    {
        _preparedHooks.insert(uuid, mapping);
    }

    return Err::NO_ERROR;
}

void DeviceManager::init(QString storagePath)
{
    _storagePath = storagePath;
    loadMappings();
    loadHandles();
}

QMap<QString, QString> DeviceManager::getMappings() const
{
    return _deviceMappings;
}

deviceHandlePtr DeviceManager::getHandle(QString uuid) const
{
    return _handles.value(uuid, deviceHandlePtr());
}

deviceHandlePtr DeviceManager::getHandleByMapping(QString mapping)
{
    deviceHandlePtr handle;
    QString uuid = _deviceMappings.value(mapping);
    if(!uuid.isEmpty())
    {
         handle = _handles.value(uuid);
        if(!handle.isNull())
            return handle;
    }

    if(_handleByMappings.contains(mapping))
    {
        weakDeviceHandlePtr weakhandle = _handleByMappings.value(mapping);
        if(!weakhandle.isNull())
            return weakhandle.toStrongRef();
    }

    handle.reset(new DeviceHandle(""));
    _handleByMappings.insert(mapping, handle);

    return handle;

}

QList<deviceHandlePtr> DeviceManager::getHandles()
{
    return _handles.values();
}

QStringList DeviceManager::getDevices() const
{
    return _deviceMap.keys();
}

iDevicePtr DeviceManager::getDeviceByUuid(QString uuid) const
{
    return _deviceMap.value(uuid, nullptr);
}

QString DeviceManager::getDeviceByMapping(QString mapping) const
{
    QString uuid = _deviceMappings.value(mapping,"");
    return uuid;
}

QString DeviceManager::getTypeForUuid(QString uuid) const
{
    iDevicePtr device = _deviceMap.value(uuid);
    if(device)
    {
        return device->type();
    }
    return "";
}

QString DeviceManager::getUuidForShortId(QString shortID) const
{
    return _shortIDtoUid.value(shortID.toUpper());
}

DeviceManager::DeviceManager(QObject *parent) : QObject(parent){}

DeviceManager::~DeviceManager()
{
}

deviceHandlePtr DeviceManager::addDeviceHandle(QString uuid)
{
    if(_handles.contains(uuid))
        return _handles.value(uuid);

    QString path = _storagePath +"/handles/"+uuid;
    deviceHandlePtr deviceHandle(new DeviceHandle(uuid, path));
    _handles.insert(uuid, deviceHandle);
    Q_EMIT newDeviceHandle(uuid);
    return deviceHandle;
}

DeviceManager *DeviceManager::instance()
{
    return deviceManager;
}

Err::CloudError DeviceManager::unhook(QString mapping)
{
    qDebug() << "Delete mapping: "+mapping;
    QString deviceUUID = getDeviceByMapping(mapping);


    if(deviceUUID.isEmpty())
        return Err::INVALID_DATA;

    _deviceMappings.remove(mapping);
    Q_EMIT deviceMappingRemoved(deviceUUID, mapping);

    weakDeviceHandlePtr weakPtr = _handles.value(deviceUUID).toWeakRef();

    _handles.remove(deviceUUID);
    if(!weakPtr.isNull())
    {
        /* Someone still has a smart pointer to this DeviceHandle instance.
         * The DeviceHandle instance is only deleted when there are no more
         * references to it. Until then the handle remains registered in the manager.
         * It may even happen that the handle is reused if a new mapping with
         * this address is created in the meantime. */

        weakPtr.toStrongRef()->removeDevice();
        _handleByMappings.insert(mapping, weakPtr);
    }
    Q_EMIT deviceHandleRemoved(deviceUUID);

    saveMappings();
    return Err::NO_ERROR;
}

Err::CloudError DeviceManager::hook(QString mapping, QString uuid, bool force)
{
    iDevicePtr device = _deviceMap.value(uuid, nullptr);
    if(!device)
    {
        return Err::PERMISSION_DENIED; // Device needs to be online
    }

    if(_deviceMappings.contains(mapping))
    {
        unhook(mapping);
    }

    if(_deviceMappings.values().contains(uuid))
    {
        if(force)
             unhook(_deviceMappings.key(uuid));
        else
            return Err::ALREADY_EXISTS;
    }

    quint32 value = 0;
    if(device->enableAuthentificationKey())
    {
        value = QRandomGenerator::global()->generate();
        device->setAuthentificationKey(value);
    }

    _deviceMappings.insert(mapping, uuid);

    // dummy handle already exists - This is the case when someone instanciates
    // a handler to an Device which hasn't already a hook
    deviceHandlePtr handle = _handleByMappings.value(mapping);
    if(!handle.isNull())
    {
        QString path = _storagePath +"/handles/"+uuid;
        handle->setResourcePath(path);
        handle->setDevice(_deviceMap[uuid]);

        if(value != 0)
            handle->setAuthentificationKey(value);

        _handles.insert(uuid, handle);
        Q_EMIT newDeviceHandle(uuid);

    }
    else
    {
        qDebug()<<"Create new handle";
        handle = addDeviceHandle(uuid);
        if(!handle.isNull() && value != 0)
                handle->setAuthentificationKey(value);
    }

    handle->setPermissions(device->getRequestedPermissions());
    Q_EMIT newDeviceMapping(uuid, mapping);

    _handleByMappings.remove(mapping);
    saveMappings();
    return Err::NO_ERROR;
}

void DeviceManager::saveMappings()
{
    QVariantMap mappings;
    qInfo()<<"Save mapping:"+_storagePath+"/mappings";
    QMapIterator<QString, QString> it(_deviceMappings);

    while(it.hasNext())
    {
        it.next();
        mappings.insert(it.key(), it.value());
    }

    QVariantMap data;
    data["mappings"] = mappings;


    QFile file(_storagePath+"/mappings");
    QFileInfo info(file);
    QDir dir(info.absolutePath());
    if(!dir.exists())
    {
        dir.mkpath(info.absolutePath());
    }

    if(file.open(QFile::WriteOnly))
    {
        file.write(QJsonDocument::fromVariant(data).toJson());
        file.close();
    }
    else
    {
        qDebug()<<"Warning: Could not open file -"<<file.errorString();
    }
}

void DeviceManager::loadMappings()
{
    QFile file(_storagePath+"/mappings");
    if( file.open(QFile::ReadOnly))
    {
        QVariantMap data =  QJsonDocument::fromJson(file.readAll()).toVariant().toMap();
        QVariantMap mappings = data["mappings"].toMap();
        file.close();
        _deviceMappings.clear();
        QMapIterator<QString, QVariant> it(mappings);
        while(it.hasNext())
        {
            it.next();
            QString mapping = it.key();
            QString uuid = it.value().toString();
            _deviceMappings.insert(mapping, uuid);
            Q_EMIT newDeviceMapping(uuid, mapping);
        }
    }
    else
    {
        qCritical()<<"Warning: Could not open File:  "<<_storagePath<<" - "<<file.errorString();
    }
}

void DeviceManager::loadHandles()
{
    // load only handles for Devices which have a mapping
    QSet<QString> registeredDeviceUuids(_deviceMappings.begin(), _deviceMappings.end());
    QSetIterator<QString> it(registeredDeviceUuids);
    while(it.hasNext())
    {
        QString uuid = it.next();
        qDebug()<<"load uuid: "+ uuid;
        addDeviceHandle(uuid);
    }
}

void DeviceManager::deregisterDevice(QString uuid)
{
    if(!_deviceMap.contains(uuid))
        return;

    disconnect(_deviceMap.value(uuid).data(), &IDevice::deregistered, this, &DeviceManager::deregisterDevice);
    _deviceMap.remove(uuid);
    qInfo()<<"Device deregistered: "<<uuid;
    Q_EMIT deviceDeregistered(uuid);
}
