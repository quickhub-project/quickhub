/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "DeviceHandleListWrapper.h"

DeviceHandleListWrapper::DeviceHandleListWrapper(QObject* parent) : IList(parent)
{
    connect(DeviceManager::instance(), &DeviceManager::newDeviceHandle, this, &DeviceHandleListWrapper::newHandle);
    connect(DeviceManager::instance(), &DeviceManager::deviceHandleRemoved, this, &DeviceHandleListWrapper::handleRemoved);
    connect(DeviceManager::instance(), &DeviceManager::newDeviceMapping, this, &DeviceHandleListWrapper::newMapping);

    QList<deviceHandlePtr> handles = DeviceManager::instance()->getHandles();
    QListIterator<deviceHandlePtr> it(handles);
    while (it.hasNext())
    {
        addHandle(it.next());
    }
}

QVariantList DeviceHandleListWrapper::getListData() const
{
    return _devices.values();
}

void DeviceHandleListWrapper::newHandle(QString uuid)
{
    addHandle(DeviceManager::instance()->getHandle(uuid));
}

void DeviceHandleListWrapper::handleRemoved(QString uuid)
{
    int idx = getIndex(uuid);
    if(idx < 0)
        return;

    _devices.remove(uuid);
    Q_EMIT itemRemoved(idx);
}

void DeviceHandleListWrapper::deviceStateChangedSlot(QString uuid, IDevice::DeviceState state)
{
    if(!_devices.contains(uuid))
        return;

    bool online = (state == IDevice::ONLINE);
    QVariantMap item = _devices[uuid].toMap();
    item["online"] = online;
    _devices.insert(uuid, item);
    int idx = getIndex(uuid);
    Q_EMIT IList::propertyChanged("online", online, idx);
}

void DeviceHandleListWrapper::deviceDescriptionChangedSlot(QString uuid, QString description)
{
    if(!_devices.contains(uuid))
        return;

    QVariantMap item = _devices[uuid].toMap();
    item["description"] = description;
    _devices.insert(uuid, item);
    int idx = getIndex(uuid);
    Q_EMIT IList::propertyChanged("description", description, idx);
}

void DeviceHandleListWrapper::newMapping(QString uuid, QString mapping)
{
    Q_UNUSED(mapping)
    if(!_devices.contains(uuid))
        return;

    QVariantMap item = _devices[uuid].toMap();
    QStringList mappings = DeviceManager::instance()->getMappings().keys(uuid);
    item["mappings"] = mappings;
    _devices.insert(uuid, item);
    int idx = getIndex(uuid);
    Q_EMIT propertyChanged("mappings", mappings, idx);
}

void DeviceHandleListWrapper::addHandle(deviceHandlePtr handle)
{
    if(handle.isNull())
        return;

    QString uuid = handle->uuid();
    QVariant handleData = toVariant(handle);
    _devices.insert(uuid, handleData);
    int idx = getIndex(uuid);

    connect(handle.data(), &DeviceHandle::deviceStateChanged, this, &DeviceHandleListWrapper::deviceStateChangedSlot);
    connect(handle.data(), &DeviceHandle::descriptionChanged, this, &DeviceHandleListWrapper::deviceDescriptionChangedSlot);
    Q_EMIT itemAdded(handleData, idx);
}

QVariant DeviceHandleListWrapper::toVariant(deviceHandlePtr handle)
{
    QVariantMap data;
    QStringList deviceMappings = DeviceManager::instance()->getMappings().keys(handle->uuid());
    data["mappings"] = deviceMappings;
    data["online"] = handle->getDeviceState() == IDevice::ONLINE;
    data["uuid"] = handle->uuid();
    data["type"] = handle->type();
    data["description"] = handle->getDescription();
    data["shortID"] = handle->shortUid();
    return data;
}

int DeviceHandleListWrapper::getIndex(QString uuid) const
{
    if(!_devices.contains(uuid))
        return -1;

    QList<QString> keys = _devices.keys();
    //auto i = qBinaryFind(keys.begin(), keys.end(), uuid);
    const auto i = std::lower_bound(keys.begin(), keys.end(), uuid);
    return i - keys.begin();
}
