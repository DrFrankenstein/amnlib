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
         _subpacket(NULL)
{
}

dAmnPacket::dAmnPacket(dAmnSession* parent, const QString& cmd, const QString& param, const QString& data)
    : dAmnObject(parent),
        _cmd(cmd), _param(param), _data(data), _args(QHash<QString, QString>()), _subpacket(NULL)
{
    this->setKCmd();
}

dAmnPacket::dAmnPacket(const dAmnPacket& packet)
    : dAmnObject(packet.session())
{
    if(packet._subpacket != NULL)
    {
        this->_subpacket = new dAmnPacket(*packet._subpacket);
    }
}

dAmnPacket::~dAmnPacket()
{
    delete _subpacket;
}

QHash<QString, dAmnPacket::KnownCmd> dAmnPacket::_kcmd_map;

void dAmnPacket::initKCmd()
{
#   define KCMD(name) _kcmd_map[#name] = name;
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
    if(this->_kcmd_map.isEmpty())
        initKCmd();

    this->_kcmd = _kcmd_map[this->_cmd];
}

QByteArray dAmnPacket::toByteArray() const
{
    QString rawpacket;
    QTextStream builder (&rawpacket, QIODevice::WriteOnly);

    builder << this->_cmd;
    if(!this->_param.isEmpty())
    {
        builder << ' ' << this->_param;
    }

    builder << '\n';

    QList<QString> keys = this->_args.keys();
    foreach(QString argn, keys)
    {
        builder << argn << '=' << this->_args[argn] << '\n';
    }

    if(!this->_data.isEmpty())
    {
        builder << this->_data;
    }

    builder << '\0';

    return rawpacket.toUtf8();
}

const QHash<QString, QString>& dAmnPacket::args() const
{
    return this->_args;
}

QHash<QString, QString>& dAmnPacket::args()
{
    return this->_args;
}

QString dAmnPacket::arg(const QString& name) const
{
    return this->_args.value(name);
}

void dAmnPacket::setArg(const QString& name, const QString& value)
{
    this->_args.insert(name, value);
}

void dAmnPacket::setArgs(const QHash<QString, QString> &args)
{
    this->_args = args;
}

dAmnPacket::KnownCmd dAmnPacket::command() const
{
    return this->_kcmd;
}

const QString& dAmnPacket::param() const
{
    return this->_param;
}

const QString& dAmnPacket::data() const
{
    return this->_data;
}

dAmnPacket& dAmnPacket::subPacket()
{
    if(!this->_subpacket)
    {
        dAmnPacketParser parser (this->session());
        this->_subpacket = parser.parsePacket(&this->_data.toUtf8());
    }

    return *this->_subpacket;
}
