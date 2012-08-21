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

void dAmnChatroom::addPrivclass(dAmnPrivClass* pc)
{
    this->privclasses[pc->getName()] = pc;
}
void dAmnChatroom::removePrivclass(const QString& name)
{
    dAmnPrivClass* pc = this->privclasses[name];
    delete pc;

    this->privclasses.remove(name);
}

void dAmnChatroom::updatePrivclasses(const QString& data)
{
    this->privclasses.clear();

    QString pclasses = data;
    QTextStream parser (&pclasses);

    while(!parser.atEnd())
    {
        bool ok;

        QString line = parser.readLine();

        QStringList split = line.split(':');
        if(split.size() < 2)
        {
            MNLIB_WARN("Invalid privclass property \"%s\" ignored.",
                       qPrintable(line));
            continue;
        }

        int idx = split[0].toInt(&ok);
        if(!ok)
        {
            MNLIB_WARN("Could not parse privclass order: %s",
                       qPrintable(split[0]));
            continue;
        }

        this->addPrivclass(
                new dAmnPrivClass(this, split[1], idx)
                );
    }
}

void dAmnChatroom::processMembers(const QString& data)
{
    QString members = data;
    QTextStream reader (&members);
    QString name, pc, realname, type_name;
    int usericon = 0;
    QChar symbol = 0;

    while(!reader.atEnd())
    {
        QString line = reader.readLine();

        if(line.startsWith("member "))
        {
            if(!name.isEmpty())
            {
                this->addMember(name, pc, usericon, symbol, realname, type_name);
            }

            pc = realname = type_name = QString();
            symbol = usericon = 0;

            name = line.mid(7); // indexOf(' ')
        }
        else if(!line.isEmpty())
        {
            QPair<QString,QString> pair = dAmnPacketParser::splitPair(line);
            if(pair.first == "pc") pc = pair.second;
            else if(pair.first == "usericon")
            {
                bool ok;
                usericon = pair.second.toInt(&ok);
                if(!ok) MNLIB_WARN("Invalid usericon value for user %s: %s",
                                   qPrintable(name), qPrintable(pair.second));
            }
            else if(pair.first == "symbol") symbol = pair.second.at(0);
            else if(pair.first == "realname") realname = pair.second;
            else if(pair.first == "typename") type_name = pair.second;
            else
            {
                MNLIB_WARN("Unknown user property %s = %s for %s",
                           qPrintable(pair.first), qPrintable(pair.second), qPrintable(name));
            }
        }
    }
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
    packet.getArgs().insert("u", user.getName());
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
    packet.getArgs().insert("p", property);

    this->session()->send(packet);
}
void dAmnChatroom::setRoomProperty(const QString& property, const QString& value)
{
    dAmnPacket packet (this->session(), "set", this->getId().toIdString(), value);
    packet.getArgs().insert("p", property);

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

void dAmnChatroom::addMember(const QString &name,
                             const QString &pcname,
                             int usericon,
                             const QChar &symbol,
                             const QString &realname,
                             const QString &type_name)
{
    dAmnUser* user = session()->addUser(name, usericon, symbol, realname, type_name);

    dAmnPrivClass* pc = this->privclasses[pcname];
    if (!pc)
    {
        MNLIB_FAIL("Chatroom %s member %s belonging to unknown privclass %s",
                   qPrintable(this->name), qPrintable(user->getName()), qPrintable(pcname));
        return; // Handle better, like... create pc?
    }

    pc->addUser(user);
    this->membersToPc[name] = pc;
}

void dAmnChatroom::removeMember(const QString& name)
{
    dAmnPrivClass* pc = this->membersToPc[name];
    if(!pc)
    {
        MNLIB_WARN("Attempt to remove unknown member %s from chatroom %s",
                   qPrintable(name), this->name);
        return;
    }

    pc->removeUser(session()->getUsers()[name]);
    session()->cleanupUser(name);
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
