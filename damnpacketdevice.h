/*
    This file is part of
    amnlib - A C++ library for deviantART Message Network
    Copyright © 2011 Carl Tessier <http://drfrankenstein90.deviantart.com/>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DAMNPACKETDEVICE_H
#define DAMNPACKETDEVICE_H

#include <QIODevice>
#include "mnlib_global.h"
#include "damnobject.h"
#include "damnpacketparser.h"

class dAmnSession;

class MNLIBSHARED_EXPORT dAmnPacketDevice : public dAmnObject
{
    Q_OBJECT

    QIODevice& device;
    dAmnPacketParser parser;

public:
    explicit dAmnPacketDevice(dAmnSession* session, QIODevice& device);

signals:
    void packetReady(dAmnPacket* packet);

private slots:
    void readPacket();
};

#endif // DAMNPACKETDEVICE_H
