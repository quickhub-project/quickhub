/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "UserListWrapper.h"
#include <QDebug>


UserListWrapper::UserListWrapper(QObject *parent) : IList(parent)
{
    connect(&_mapper, SIGNAL(mapped(QString)), this, SLOT(dataChanged(QString)));
    connect(DefaultAuthenticator::instance(), SIGNAL(userAdded(userPtr)),      this, SLOT(userAdded(userPtr)));
    connect(DefaultAuthenticator::instance(), SIGNAL(userDeleted(userPtr)),    this, SLOT(userDeleted(userPtr)));
    auto tmpUsers = DefaultAuthenticator::instance()->getUsers();
    QVectorIterator<userPtr> it(tmpUsers);
    while(it.hasNext())
    {
        userPtr user = it.next();
        _mapper.setMapping(user.data(), user->identityID());
        connect(user.data(), SIGNAL(dataChanged()), &_mapper, SLOT(map()));
        _userVariants.insert(user->identityID(), userToVariant(user));
    }
}

QVariantList UserListWrapper::getListData() const
{
    return _userVariants.values();
}

bool UserListWrapper::isPermitted(QString token) const
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull())
        return false;

    return user->isAuthorizedTo(MONITOR_USERS);
}

void UserListWrapper::dataChanged(QString userID)
{
    userPtr user = DefaultAuthenticator::instance()->getUserForID(userID);
    QVariantMap userVariant = userToVariant(user);
    _userVariants[userID] = userVariant;
    Q_EMIT itemChanged(userVariant, getIndex(userID));
}

void UserListWrapper::userAdded(userPtr user)
{
    QVariant userVariant = userToVariant(user);
    QString userID = user->identityID();
    _userVariants[userID] = userVariant;
    _mapper.setMapping(user.data(), user->identityID());
    connect(user.data(), SIGNAL(dataChanged()), &_mapper, SLOT(map()));
    Q_EMIT itemAdded(userVariant, getIndex(userID));
}

void UserListWrapper::userDeleted(userPtr user)
{
    QString userID = user->identityID();
    int idx = getIndex(userID);
    _userVariants.remove(userID);
    Q_EMIT itemRemoved(idx);
}

int UserListWrapper::getIndex(QString uuid) const
{
    if(!_userVariants.contains(uuid))
        return -1;

    const auto keys = _userVariants.keys();
    return std::lower_bound(keys.begin(), keys.end(), uuid) - keys.begin();
}


QVariantMap UserListWrapper::userToVariant(userPtr user) const
{
    QVariantMap userVariant;
    QSharedPointer<User> castedUser = qSharedPointerCast<User>(user);
    userVariant["userID"] = castedUser->identityID();
    userVariant["userData"] = castedUser->getUserData();
    userVariant["sessionCount"] = castedUser->sessionCount();
    userVariant["userName"] = castedUser->getUserName();
    userVariant["userPermissions"] = castedUser->userPermissions();
    userVariant["eMail"] = castedUser->getEMail();
    return userVariant;
}
