# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
# It is part of the QuickHub framework - www.quickhub.org
# Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de

TEMPLATE = app
INCLUDEPATH += ../qh_plugin_system/src
LIBS += -lQHPluginSystem -L../bin/lib -ldl
LIBS += -lQHCore -L../bin/plugins
QT += core websockets qml
DESTDIR = ../bin
SOURCES += \
    main.cpp
TARGET = run
INSTALLS += target
