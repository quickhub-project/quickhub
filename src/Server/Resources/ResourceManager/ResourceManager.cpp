/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "ResourceManager.h"
#include <QCoreApplication>
#include "../../Authentication/AuthentificationService.h"
#include "IResourceFactory.h"
#include "../ListResource/ListResource.h"
#include "../ObjectResource/ObjectResource.h"

Q_GLOBAL_STATIC(ResourceManager, resourceManager);

ResourceManager *ResourceManager::instance()
{
    return resourceManager;
}

ResourceManager::ResourceManager(QObject *parent) : QObject(parent)
{
}

void ResourceManager::init()
{
   // _storagePath = storagePath;
}

void ResourceManager::addResourceFactory(IResourceFactory *factory)
{
    qInfo()<<"Added Resource factory for: "+ factory->getResourceType()+ (factory->getDescriptorPrefix().isEmpty() ? " (default)" : "; descriptor: " + factory->getDescriptorPrefix());
    factory->setParent(this);
    _resourceFactorys.insert(factory->getResourceType(), factory);
}

resourcePtr ResourceManager::getOrCreateResource(QString type, QString descriptor, QString token, Err::CloudError *error)
{
    if(error)
        *error = Err::NO_ERROR;

    if(descriptor.isEmpty())
    {
        qWarning()<<Q_FUNC_INFO <<"- Invalid descriptor!";
        if(error)
            *error = Err::INVALID_DESCRIPTOR;
        return resourcePtr(nullptr);
    }

    iIdentityPtr  user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull())
    {
        qWarning()<<Q_FUNC_INFO <<"- Invalid token!";
        if(error)
            *error = Err::INVALID_TOKEN;
        return resourcePtr(nullptr);
    }

    // TODO: Check if user is allowed to access resources - > groups?

    IResourceFactory* factory = getResourceFactory(type, descriptor);
    if(!factory)
    {
        qWarning()<<Q_FUNC_INFO <<"- Unknown resource type: "<<type;
        if(error)
            *error = Err::UNKNOWN_TYPE;
        return resourcePtr(nullptr);
    }

    QString resourceId = factory->getResourceID(descriptor, token);
    if(resourceId.isEmpty())
        return resourcePtr(nullptr);

    QWriteLocker locker(&_resourceMutex);
    if(_resources.contains(resourceId))
    {
        return _resources.value(resourceId);
    }

    resourcePtr resource = factory->createResource(token, descriptor);

    if(!resource.isNull())
    {
      //  resource->setParent(this);
        if(!resource->dynamicContent())
            addResource(resource, resourceId);
    }

    return resource;
}


QString ResourceManager::getResourceID(QString type, QString descriptor, QString token) const
{
    IResourceFactory* factory = getResourceFactory(type, descriptor);
    if(!factory)
        return "";

    return factory->getResourceID(descriptor, token);
}


void ResourceManager::addResource(resourcePtr resource, QString qualifiedResourceName)
{
    resource->setProperty("descriptor", qualifiedResourceName);
    resource->_descriptor = qualifiedResourceName;
    connect(resource.data(), &IResource::resourceDestroyed, this, &ResourceManager::resourceDestroyed);
    _resources.insert(qualifiedResourceName, resource);
}

IResourceFactory *ResourceManager::getResourceFactory(QString type, QString descriptor) const
{
   QList<IResourceFactory*> factories = _resourceFactorys.values(type);
   std::sort(factories.begin(), factories.end(), [](const IResourceFactory* a, const IResourceFactory* b) -> bool
   {
       return a->getDescriptorPrefix().length() > b->getDescriptorPrefix().length();
   });
   QListIterator<IResourceFactory*> it(factories);

   if(factories.count() == 1)
       return factories.first();

   IResourceFactory* defaultFactory = nullptr;
   while(it.hasNext())
   {
       IResourceFactory* factory = it.next();
       if(factory->getDescriptorPrefix().isEmpty())
       {
            defaultFactory = factory;
            continue;
       }

       int lastSlash = descriptor.lastIndexOf("/");
       QString preparedDescriptor2 = descriptor.left(lastSlash);
       QString preparedDescriptor = descriptor.split(":").first();
       preparedDescriptor = preparedDescriptor.split("?").first();
       QString currentPrefix = factory->getDescriptorPrefix();
       if(preparedDescriptor.startsWith(currentPrefix))
           return factory;

//       This was the first version - hasn't worked with prefixes like "foo" when the requested
//       descriptor was like foo/bar/ID471123 - The current version above is more pragmatic
//       if(preparedDescriptor == currentPrefix || preparedDescriptor2 == currentPrefix )
//           return factory;
   }
   return defaultFactory;
}

void ResourceManager::resourceDestroyed(QString descriptor)
{
    _resourceMutex.lockForWrite();
    _resources.remove(descriptor);
    _resourceMutex.unlock();

}


