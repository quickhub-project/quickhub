/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef SERVICEHANDLERFACTORY_H
#define SERVICEHANDLERFACTORY_H

#include <QObject>

class ServiceHandlerFactory : public QObject
{
    Q_OBJECT
public:
    explicit ServiceHandlerFactory(QObject *parent = nullptr)
        : QObject(parent) {};

signals:

public slots:
};

#endif // SERVICEHANDLERFACTORY_H
