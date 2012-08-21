/*
    This file is part of
    amnlib - A C++ library for deviantART Message Network
    Copyright © 2010 Carl Tessier <http://drfrankenstein90.deviantart.com/>

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

#include "damnpacket.h"
#include "damnsession.h"
#include "damnpacketparser.h"

#include <QByteArray>
#include <QTextStream>
#include <QBuffer>
#include <QMap>
#include <QString>

dAmnPacket::dAmnPacket(dAmnSession* parent)
        :dAmnObject(parent),
         subpacket(NULL)
{
}

dAmnPacket::dAmnPacket(dAmnSession* parent, const QString& cmd, const QString& param, const QString& data)
    : dAmnObject(parent),
        cmd(cmd), param(param), data(data), args(QHash<QString, QString>()), subpacket(NULL)
{
    this->setKCmd();
}

dAmnPacket::dAmnPacket(const dAmnPacket& packet)
    : dAmnObject(packet.session())
{
    if(packet.subpacket != NULL)
    {
        this->subpacket = new dAmnPacket(*packet.subpacket);
    }
}

dAmnPacket::~dAmnPacket()
{
    delete subpacket;
}

QHash<QString, dAmnPacket::KnownCmd> dAmnPacket::kcmd_map;

void dAmnPacket::initKCmd()
{
#   define KCMD(name) kcmd_map[#name] = name;
    KCMD(dAmnClient) KCMD(dAmnServer) KCMD(login)
    KCMD(join) KCMD(part)
    KCMD(ping) KCMD(pong)
    KCMD(send) KCMD(recv)
    KCMD(promote) KCMD(demote)
    KCMD(kick) KCMD(kicked) KCMD(ban) KCMD(unban)
    KCMD(get) KCMD(set)
    KCMD(admin) KCMD(disconnect) KCMD(kill)
    KCMD(property)

    KCMD(msg) KCMD(action) KCMD(npmsg)
    KCMD(userinfo)
    KCMD(whois)
    KCMD(privchg)
#   undef KCMD
}

void dAmnPacket::setKCmd()
{
    if(this->kcmd_map.isEmpty())
        initKCmd();

    this->kcmd = kcmd_map[this->cmd];
}

QByteArray dAmnPacket::toByteArray() const
{
    QString rawpacket;
    QTextStream builder (&rawpacket, QIODevice::WriteOnly);

    builder << this->cmd;
    if(!this->param.isEmpty())
    {
        builder << ' ' << this->param;
    }

    builder << '\n';

    QList<QString> keys = this->args.keys();
    foreach(QString argn, keys)
    {
        builder << argn << '=' << this->args[argn] << '\n';
    }

    if(!this->data.isEmpty())
    {
        builder << this->data;
    }

    builder << '\0';

    return rawpacket.toUtf8();
}

const QHash<QString, QString>& dAmnPacket::getArgs() const
{
    return this->args;
}

QHash<QString, QString>& dAmnPacket::getArgs()
{
    return this->args;
}

void dAmnPacket::setArgs(const QHash<QString, QString> &args)
{
    this->args = args;
}

dAmnPacket::KnownCmd dAmnPacket::command() const
{
    return this->kcmd;
}

const QString& dAmnPacket::getParam() const
{
    return this->param;
}

const QString& dAmnPacket::getData() const
{
    return this->data;
}

dAmnPacket& dAmnPacket::getSubPacket()
{
    if(!this->subpacket)
    {
        dAmnPacketParser parser (this->session());
        this->subpacket = parser.parsePacket(&this->data.toUtf8());
    }

    return *this->subpacket;
}
