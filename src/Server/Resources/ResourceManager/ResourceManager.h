/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QObject>
#include <QMap>
#include <QReadWriteLock>
#include "../../Authentication/User.h"
#include "../../Defines/ErrDef.h"
#include "qhcore_global.h"
#include <QSharedPointer>


class IResource;
class IResourceFactory;
class User;

typedef  QSharedPointer<IResource> resourcePtr;

class COREPLUGINSHARED_EXPORT ResourceManager : public QObject
{
    Q_OBJECT

public:
    explicit ResourceManager(QObject *parent = nullptr);
    static ResourceManager* instance();
    void                    init();
    void                    addResourceFactory(IResourceFactory* factory);
    resourcePtr             getOrCreateResource(QString type, QString descriptor, QString token, Err::CloudError* error = nullptr);
    QString                 getResourceID(QString type, QString descriptor, QString token) const;

private:
    void addResource(resourcePtr resource, QString qualifiedResourceName);

    IResourceFactory*                       getResourceFactory(QString type, QString descriptor) const;
    QMap<QString, QWeakPointer<IResource>>  _resources;
    QMultiMap<QString, IResourceFactory *> _resourceFactorys;
    mutable QReadWriteLock                  _resourceMutex;

private slots:
    void resourceDestroyed(QString descriptor);

};

#endif // RESOURCEMANAGER_H
