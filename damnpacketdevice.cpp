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

#include "damnpacketdevice.h"

#include <QByteArray>
#include "damnobject.h"
#include "damnpacket.h"

class dAmnSession;

dAmnPacketDevice::dAmnPacketDevice(dAmnSession *session, QIODevice& device):
    dAmnObject(session), _device(device), _parser(session)
{
    connect(&this->_device, SIGNAL(readyRead()),
            SLOT(readPacket()));
}

void dAmnPacketDevice::readPacket()
{
    char c;

    do
    {
        while(this->_device.getChar(&c) && c)
        {
            this->_packetBuffer.append(c);
        }

        if(!c)
        {   // '\0'
            dAmnPacket* packet = this->_parser.parsePacket(&this->_packetBuffer);

            if(packet)
                emit packetReady(*packet);

            delete packet;
            this->_packetBuffer.clear();
        }
    }
    while(this->_device.bytesAvailable());
}
