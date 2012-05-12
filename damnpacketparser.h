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

#ifndef DAMNPACKETPARSER_H
#define DAMNPACKETPARSER_H

class QByteArray;
class dAmnSession;
class dAmnPacket;

#include "mnlib_global.h"
#include <QPair>
#include <QString>

class MNLIBSHARED_EXPORT dAmnPacketParser
{
    dAmnSession* session;

    enum ParserState
    {
        Cmd, Param, ArgName, ArgValue, Data
    };

public:
    explicit dAmnPacketParser(dAmnSession* session);

    dAmnPacket* parsePacket(QByteArray* raw);

    static QPair<QString, QString> splitPair(const QString& line);

};

#endif // DAMNPACKETPARSER_H
