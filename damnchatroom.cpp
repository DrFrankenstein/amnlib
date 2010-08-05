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

#include "damnobject.h"
#include "damnsession.h"
#include "damnchatroom.h"
#include "damnprivclass.h"
#include "damnpacket.h"
#include "damnuser.h"

#include <QString>
#include <QStringList>
#include <QHash>

dAmnChatroom::dAmnChatroom(dAmnSession* parent, const QString& roomstring)
    : dAmnObject(parent)
{
    if(roomstring.startsWith('#'))
    {
        this->type = dAmnChatroom::chat;
        this->name = roomstring.mid(1);
    }
    else
    {
        QStringList parts = roomstring.split(':');

        if(parts[0] == "pchat")
        {

            this->type = dAmnChatroom::pchat;
            if(this->parent() != NULL)
            {
                if(this->session()->isMe(parts[1]))
                    this->name = parts[2];
                else
                    this->name = parts[1];
            }
            else
            {
                this->name = QString("%1:%2").arg(parts[1], parts[2]);
            }
        }
        else if(parts[0] == "chat")
        {
            this->type = dAmnChatroom::chat;
            this->name = parts[1];
        }
        else
        {
            this->type = dAmnChatroom::pchat;
            this->name = parts[0];
        }
    }
}

dAmnChatroom::dAmnChatroom(dAmnSession* parent, const dAmnChatroomIdentifier& id)
    : dAmnObject(parent),
      type(id.type), name(id.name)
{
}

dAmnChatroom::Type dAmnChatroom::getType() const
{
    return this->type;
}
const QString& dAmnChatroom::getName() const
{
    return this->name;
}
const QString& dAmnChatroom::getTitle() const
{
    return this->title;
}
const QDateTime& dAmnChatroom::getTitleDate() const
{
    return this->titledate;
}
const QString& dAmnChatroom::getTopic() const
{
    return this->topic;
}
const QDateTime& dAmnChatroom::getTopicDate() const
{
    return this->topicdate;
}
QList<dAmnPrivClass*> dAmnChatroom::getPrivclasses() const
{
    return this->privclasses.values();
}
dAmnChatroomIdentifier dAmnChatroom::getId() const
{
    return dAmnChatroomIdentifier(this->session(), this->type, this->name);
}

void dAmnChatroom::setTopic(const QString& newtopic)
{
    this->topic = newtopic;
}
void dAmnChatroom::setTitle(const QString& newtitle)
{
    this->title = newtitle;
}

void dAmnChatroom::addPrivclass(dAmnPrivClass& pc)
{
    this->privclasses[pc.getName()] = &pc;
}
void dAmnChatroom::removePrivclass(const QString& name)
{
    this->privclasses.remove(name);
}

void dAmnChatroom::part()
{
    this->session()->part(this->getId());
}

void dAmnChatroom::say(const QString& message)
{
    dAmnPacket packet (this->session(), "msg", "main", message);
    this->send(packet);
}

void dAmnChatroom::act(const QString& action)
{
    dAmnPacket packet (this->session(), "action", "main", action);
    this->send(packet);
}

void dAmnChatroom::npmsg(const QString& message)
{
    dAmnPacket packet (this->session(), "npmsg", "main", message);
    this->send(packet);
}

void dAmnChatroom::promote(const dAmnUser &user)
{
    this->promote(user.getName());
}
void dAmnChatroom::promote(const QString &username)
{
    dAmnPacket packet (this->session(), "promote", username);
    this->send(packet);
}

void dAmnChatroom::demote(const dAmnUser &user)
{
    this->demote(user.getName());
}
void dAmnChatroom::demote(const QString &username)
{
    dAmnPacket packet (this->session(), "demote", username);
    this->send(packet);
}

void dAmnChatroom::chgPrivclass(const dAmnUser &user, const QString &privclass)
{
    dAmnPacket packet (this->session(), "promote", user.getName(), privclass);
    this->send(packet);
}

void dAmnChatroom::kick(const dAmnUser &user, const QString& reason)
{
    dAmnPacket packet (this->session(), "kick", this->getId().toIdString(),
                       reason);
    packet["u"] = user.getName();
    this->session()->send(packet);
}

void dAmnChatroom::ban(const dAmnUser &user)
{
    dAmnPacket packet (this->session(), "ban", user.getName());
    this->send(packet);
}
void dAmnChatroom::unban(const dAmnUser &user)
{
    dAmnPacket packet (this->session(), "unban", user.getName());
    this->send(packet);
}

void dAmnChatroom::getRoomProperty(const QString& property)
{
    dAmnPacket packet (this->session(), "get", this->getId().toIdString());
    packet["p"] = property;

    this->session()->send(packet);
}
void dAmnChatroom::setRoomProperty(const QString& property, const QString& value)
{
    dAmnPacket packet (this->session(), "set", this->getId().toIdString(), value);
    packet["p"] = property;

    this->session()->send(packet);
}

void dAmnChatroom::sendAdminCommand(const QString& command)
{
    dAmnPacket packet (this->session(), "admin", QString(), command);
    this->send(packet);
}

void dAmnChatroom::send(const dAmnPacket& packet)
{
    dAmnPacket sendpacket (this->session(), "send", this->getId().toIdString(),
                           packet.toByteArray());

    this->session()->send(sendpacket);
}

////////////////////////////////////////////////////////////////////////////////

dAmnChatroomIdentifier::dAmnChatroomIdentifier(dAmnSession* parent, const QString& roomstring)
    : session(parent)
{
    if(roomstring.startsWith('#'))
    {
        this->type = dAmnChatroom::chat;
        this->name = roomstring.mid(1);
    }
    else
    {
        QStringList parts = roomstring.split(':');

        if(parts[0] == "pchat")
        {
            this->type = dAmnChatroom::pchat;
            if(this->session != NULL)
            {
                if(session->isMe(parts[1]))
                    this->name = parts[2];
                else
                    this->name = parts[1];
            }
            else
            {
                this->name = QString("%1:%2").arg(parts[1], parts[2]);
            }
        }
        else if(parts[0] == "chat")
        {
            this->type = dAmnChatroom::chat;
            this->name = parts[1];
        }
        else
        {
            this->type = dAmnChatroom::pchat;
            this->name = parts[0];
        }
    }
}

dAmnChatroomIdentifier::dAmnChatroomIdentifier(dAmnSession* parent, dAmnChatroom::Type type, const QString& name)
    : type(type),
    name(name),
    session(parent)
{
}

bool dAmnChatroomIdentifier::operator ==(const dAmnChatroomIdentifier& rhs) const
{
    return this->type == rhs.type && this->name == rhs.name;
}

QString dAmnChatroomIdentifier::toString() const
{
    switch(this->type)
    {
    case dAmnChatroom::chat:    return QString('#').append(this->name);
    case dAmnChatroom::pchat:   return this->name;
    }

    return QString();   // keep compiler happy.
}

QString dAmnChatroomIdentifier::toIdString() const
{
    switch(this->type)
    {
    case dAmnChatroom::chat:    return QString("chat:").append(this->name);
    case dAmnChatroom::pchat:
        const QString& username = session->getUserName();
        if(this->name < username)
        {
            return QString("pchat:%1:%2").arg(this->name, username);
        }
        else
        {
            return QString("pchat:%1:%2").arg(username, this->name);
        }
    }

    return QString();
}
