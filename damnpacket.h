/*
    This file is part of
    amnlib - A C++ library for deviantART Message Network
    Copyright � 2010 Carl Tessier <http://drfrankenstein90.deviantart.com/>

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

#ifndef DAMNPACKET_H
#define DAMNPACKET_H

#include <QHash>

#include "mnlib_global.h"
#include "damnobject.h"

template <class T1, class T2> struct QPair;

class dAmnSession;

class MNLIBSHARED_EXPORT dAmnPacket : public dAmnObject
{
    QString cmd, param, data;
    QHash<QString, QString> args;
    dAmnPacket* subpacket;

    void setKCmd();
    static void initKCmd();

public:
    enum KnownCmd
    {
        unknown,
        dAmnClient, dAmnServer, login,
        join, part,
        ping, pong,
        send, recv,
        promote, demote,
        kick, kicked, ban, unban,
        get, set,
        admin, disconnect, kill,
        property,

        msg, action, npmsg,
        userinfo,
        whois,
        privchg
    };
protected:
    KnownCmd kcmd;
    static QHash<QString, KnownCmd> kcmd_map;
public:

    // Creates a null packet.
    dAmnPacket(dAmnSession* parent);
    // Copies a packet.
    dAmnPacket(const dAmnPacket& packet);
    // Builds a packet.
    dAmnPacket(dAmnSession* parent, const QString& cmd, const QString& param = QString(), const QString& data = QString());
    // Destroys a packet.
    virtual ~dAmnPacket();

    // Retrieves the argument list.
    const QHash<QString, QString>& getArgs() const;
    QHash<QString, QString>& getArgs();
    void setArgs(const QHash<QString, QString>& args);
    //QString operator[](const QString& arg);

    KnownCmd command() const;

    const QString& getParam() const;
    const QString& getData() const;

    QByteArray toByteArray() const;

    dAmnPacket& getSubPacket();
};

#endif // DAMNPACKET_H
