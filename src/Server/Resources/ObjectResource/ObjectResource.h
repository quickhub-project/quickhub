/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef OBJECTRESOURCE_H
#define OBJECTRESOURCE_H

#include <QObject>
#include <QFile>
#include <QVariant>
#include <QReadWriteLock>

#include "../ResourceManager/IResource.h"
#include "ObjectResourceFactory.h"

/*!
    \class ObjectResource
    \brief This class is the one and only ObjectResource.

    The ObjectResource implements the interface and logic for the
    synchronization of object-based (key/value pairs) data. From this class can be derived to
    add own object implementations via plugin. For a custom ObjectResources you
    need an additional IObjectResourceFactory implementation.
*/

class IObjectResourceStorage;
class ResourceManager;
class ObjectResource : public IResource
{
    Q_OBJECT

    friend class ObjectResourceFactory;

public:
    explicit ObjectResource(IObjectResourceStorage* storage, QObject *parent = 0);
    ~ObjectResource() override;

    /*!
        \fn qint64 ObjectResource::lastAccess() const override
        Returns a unix timestamp of the last user access.
    */
    qint64                      lastAccess() const override;

    /*!
        \fn QString ObjectResource::getResourceType() const override
        Returns the resource type as string.
    */
    QString const               getResourceType() const override;

    /*!
        \fn virtual QVariantMap ObjectResource::getObjectData() const;
        Returns the complete object as QVariantMap.
        \note QVariantMap -> QMap<QVariant>
    */
    virtual QVariantMap         getObjectData() const;

    /*!
        \fn QVariantMap ObjectResource::getMetadata() const;
        Returns the metadata-blob of the object. This can contain additional
        information about the list data
    */
    virtual QVariantMap         getMetaData() const;

    /*!
        \fn bool ObjectResource::isPermittedToRead(QString token) const
        Overwrite this function and return false when the appropriate user is not permitted to read the data
        in this resource. If false is returned
    */
    virtual bool               isPermittedToRead(QString token) const;

    /*!
        \fn bool ObjectResource::isPermittedToWrite(QString token) const
        Overwrite this function and return false when the appropriate user is not permitted to modify / write the data
        in this resource.
        \note This function is checked within ObjectResource::setProperty. If you overwrite this function with
        your own implementation, there is no internal check anymore.
    */
    virtual bool               isPermittedToWrite(QString token) const;

    /*!
        Sets the value of a given property.
    */
    virtual ModificationResult setProperty(QString name, const QVariant &value, QString token);

    /*!
      \fn bool ObjectResource::setFilter(QVariantMap query)
      Applies a filter to the list. This function is not implemented by this base class. It can be overwritten to provide
      filtering query/features. The idea is to make it possible to query data that meet certain properties

      \note Don't forget to call IRresource::setDynamicContent(true) in the constructor of your derived list class when you
      provide filtering.
      \sa IResource::setDynamicContent(bool enabled)
    */
    virtual bool                setFilter(QVariantMap query);

protected:
    qint64                  _lastAccess;
    IObjectResourceStorage* _storage;
    mutable QReadWriteLock  _mutex;

signals:
    void propertyChanged(QString property, QVariant data, iIdentityPtr  user);

public slots:
};

#endif // OBJECTRESOURCE_H
