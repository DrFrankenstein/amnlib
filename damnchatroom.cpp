﻿/*
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
#include "damnrichtext.h"
#include "damnpacket.h"
#include "damnuser.h"
#include "events.h"

#include <QString>
#include <QStringList>
#include <QHash>
#include <QRegExp>
#include <algorithm>

dAmnChatroom::dAmnChatroom(dAmnSession* parent, const QString& roomstring)
    : dAmnObject(parent)
{
    if(roomstring.startsWith('#'))
    {
        this->_type = dAmnChatroom::chat;
        this->_name = roomstring.mid(1);
    }
    else
    {
        QStringList parts = roomstring.split(':');

        if(parts[0] == "pchat")
        {

            this->_type = dAmnChatroom::pchat;
            if(this->parent() != NULL)
            {
                if(this->session()->isMe(parts[1]))
                    this->_name = parts[2];
                else
                    this->_name = parts[1];
            }
            else
            {
                this->_name = QString("%1:%2").arg(parts[1], parts[2]);
            }
        }
        else if(parts[0] == "chat")
        {
            this->_type = dAmnChatroom::chat;
            this->_name = parts[1];
        }
        else
        {
            this->_type = dAmnChatroom::pchat;
            this->_name = parts[0];
        }
    }

    this->setObjectName(this->_name);
}

dAmnChatroom::dAmnChatroom(dAmnSession* parent, const dAmnChatroomIdentifier& id)
    : dAmnObject(parent),
      _type(id.type), _name(id.name)
{
    this->setObjectName(this->_name);
}

dAmnChatroom::Type dAmnChatroom::type() const
{
    return this->_type;
}
const QString& dAmnChatroom::name() const
{
    return this->_name;
}
const dAmnRichText& dAmnChatroom::title() const
{
    return this->_title;
}
const QDateTime& dAmnChatroom::titleDate() const
{
    return this->_titledate;
}
const dAmnRichText& dAmnChatroom::topic() const
{
    return this->_topic;
}
const QDateTime& dAmnChatroom::topicDate() const
{
    return this->_topicdate;
}
QList<dAmnPrivClass*> dAmnChatroom::privclasses() const
{
    return this->_privclasses.values();
}
dAmnChatroomIdentifier dAmnChatroom::id() const
{
    return dAmnChatroomIdentifier(this->session(), this->_type, this->_name);
}

void dAmnChatroom::updateTopic(const QString& newtopic)
{
    this->_topic = dAmnRichText(newtopic);
    MNLIB_DEBUG("Topic updated for %s: %s", qPrintable(this->id().toIdString()), qPrintable(this->_topic.toPlain()));
}
void dAmnChatroom::updateTitle(const QString& newtitle)
{
    this->_title = dAmnRichText(newtitle);
    MNLIB_DEBUG("Title updated for %s: %s", qPrintable(this->id().toIdString()), qPrintable(this->_title.toPlain()));
}

void dAmnChatroom::addPrivclass(dAmnPrivClass* pc)
{
    this->_privclasses[pc->name()] = pc;
}
void dAmnChatroom::removePrivclass(const QString& name)
{
    dAmnPrivClass* pc = this->_privclasses.value(name);
    delete pc;

    this->_privclasses.remove(name);
}

void dAmnChatroom::updatePrivclasses(const QString& data)
{
    this->_privclasses.clear();

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

        uint idx = split[0].toUInt(&ok);
        if(!ok)
        {
            MNLIB_WARN("Could not parse privclass order: %s",
                       qPrintable(split[0]));
            continue;
        }

        QString pcname = split[1];
        if(this->_privclasses.contains(pcname))
        {
            dAmnPrivClass* pc = this->_privclasses[pcname];
            pc->setOrderValue(idx);
        }
        else
        {
            this->addPrivclass(
                new dAmnPrivClass(this, pcname, idx)
                );
        }
    }

    MNLIB_DEBUG("Update: %s has %d privclasses.", qPrintable(this->name()), this->_privclasses.count());
}

void dAmnChatroom::processMembers(const QString& data)
{
    QString members = data;
    QTextStream reader (&members);
    QString name, pc, props;

    while(!reader.atEnd())
    {
        QString line = reader.readLine();

        if(line.startsWith("member "))
        {
            if(!name.isEmpty())
            {
                this->addMember(name, pc, props);
            }

            pc = props = QString();

            name = line.mid(7); // indexOf(' ') + 1
        }
        else if(!line.isEmpty())
        {
            if(line.startsWith("pc=")) pc = line.mid(3); // indexOf('=') + 1
            else props.append(line + "\n");
        }
    }
}

void dAmnChatroom::part()
{
    this->session()->part(this->id());
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
    this->promote(user.name());
}
void dAmnChatroom::promote(const QString &username)
{
    dAmnPacket packet (this->session(), "promote", username);
    this->send(packet);
}

void dAmnChatroom::demote(const dAmnUser &user)
{
    this->demote(user.name());
}
void dAmnChatroom::demote(const QString &username)
{
    dAmnPacket packet (this->session(), "demote", username);
    this->send(packet);
}

void dAmnChatroom::chgPrivclass(const dAmnUser &user, const QString &privclass)
{
    dAmnPacket packet (this->session(), "promote", user.name(), privclass);
    this->send(packet);
}

void dAmnChatroom::kick(const dAmnUser &user, const QString& reason)
{
    dAmnPacket packet (this->session(), "kick", this->id().toIdString(),
                       reason);
    packet.args().insert("u", user.name());
    this->session()->send(packet);
}

void dAmnChatroom::ban(const dAmnUser &user)
{
    dAmnPacket packet (this->session(), "ban", user.name());
    this->send(packet);
}
void dAmnChatroom::unban(const dAmnUser &user)
{
    dAmnPacket packet (this->session(), "unban", user.name());
    this->send(packet);
}

void dAmnChatroom::getRoomProperty(const QString& property)
{
    dAmnPacket packet (this->session(), "get", this->id().toIdString());
    packet.args().insert("p", property);

    this->session()->send(packet);
}
void dAmnChatroom::setRoomProperty(const QString& property, const QString& value)
{
    dAmnPacket packet (this->session(), "set", this->id().toIdString(), value);
    packet.args().insert("p", property);

    this->session()->send(packet);
}

void dAmnChatroom::sendAdminCommand(const QString& command)
{
    dAmnPacket packet (this->session(), "admin", QString(), command);
    this->send(packet);
}

void dAmnChatroom::send(const dAmnPacket& packet)
{
    dAmnPacket sendpacket (this->session(), "send", this->id().toIdString(),
                           packet.toByteArray());

    this->session()->send(sendpacket);
}

void dAmnChatroom::addMember(const QString& name,
                             const QString& pcname,
                             int usericon,
                             const QChar& symbol,
                             const QString& realname,
                             const QString& type_name,
                             const QString& gpc)
{
    dAmnUser* user = session()->addUser(name, usericon, symbol, realname, type_name, gpc);

    dAmnPrivClass* pc = this->_privclasses.value(pcname);
    if (!pc)
    {
        MNLIB_WARN("Chatroom %s member %s belonging to unknown privclass %s",
                   qPrintable(this->_name), qPrintable(user->name()), qPrintable(pcname));
        pc = new dAmnPrivClass(this, pcname, 0);
        this->addPrivclass(pc);
    }

    pc->addUser(user);
    this->_membersToPc.insert(name, pc);
}

void dAmnChatroom::addMember(const QString& name, const QString& pcname, const QString& props)
{
    dAmnUser* user = session()->addUser(name, props);

    dAmnPrivClass* pc = this->_privclasses.value(pcname);
    if(!pc)
    {
        MNLIB_WARN("Chatroom %s member %s belonging to unknown privclass %s",
                   qPrintable(this->_name), qPrintable(user->name()), qPrintable(pcname));
        pc = new dAmnPrivClass(this, pcname, 0);
        this->addPrivclass(pc);
    }

    pc->addUser(user);
    user->chatrooms().insert(this);
    this->_membersToPc.insert(name, pc);
}

void dAmnChatroom::removeMember(const QString& name)
{
    dAmnPrivClass* pc = this->_membersToPc.value(name);
    if(!pc)
    {
        MNLIB_WARN("Attempt to remove unknown member %s from chatroom %s",
                   qPrintable(name), this->_name);
        return;
    }

    dAmnUser* user = session()->users().value(name);
    if(!user)
    {
        MNLIB_WARN("Attempt to remove unknown member %s from chatroom %s",
                   qPrintable(name), this->_name);
        return;
    }
    pc->removeUser(user);
    user->chatrooms().remove(this);
    session()->cleanupUser(name);
}

void dAmnChatroom::moveMember(dAmnUser* user, dAmnPrivClass* src, dAmnPrivClass* dst)
{
    src->removeUser(user);
    dst->addUser(user);

    this->_membersToPc.insert(user->name(), dst);
}

uint dAmnChatroom::moveAll(dAmnPrivClass* src, dAmnPrivClass* dst)
{
    dAmnUser* user;
    uint count = 0;
    foreach(user, src->users())
    {
        this->moveMember(user, src, dst);
        count++;
    }

    return count;
}

dAmnPrivClass* dAmnChatroom::defaultPrivClass()
{   // Apparently, there are exceptions to this, though I can't find a straight answer in the Botdom Docs wiki.
    // In fact, I'm sure it's wrong.
    auto pcit = std::find_if(this->_privclasses.begin(), this->_privclasses.end(),
                             [](dAmnPrivClass* pc) { return pc->orderValue() == 25; } );

    if(pcit == this->_privclasses.end())
    {
        MNLIB_CRIT("No default privclass found in #%s!", qPrintable(this->_name));
        return NULL;
    }

    return *pcit;
}

void dAmnChatroom::notifyMessage(const MsgEvent& event)
{
    emit message(event);
    emit message(event.userName(), event.message());
}

void dAmnChatroom::notifyAction(const ActionEvent& event)
{
    emit action(event);
    emit action(event.userName(), event.action());
}

void dAmnChatroom::notifyJoin(const JoinEvent& event)
{
    QRegExp rx ("pc=(\\w+)");
    (void) rx.indexIn(event.properties());
    QString pcname = rx.cap(1);

    this->addMember(event.userName(), pcname, event.properties());

    emit joined(event);
    emit joined(event.userName());
}

void dAmnChatroom::notifyPart(const PartEvent& event)
{
    this->removeMember(event.userName());
    emit parted(event);
    emit parted(event.userName(), event.reason());
}

void dAmnChatroom::notifyPrivchg(const PrivchgEvent& event)
{
    QString userName = event.userName();
    dAmnPrivClass* oldpc = this->_membersToPc.value(userName);
    dAmnUser* user = this->session()->users().value(userName);
    oldpc->removeUser(user);
    this->_membersToPc.remove(userName);

    dAmnPrivClass* newpc = this->_privclasses.value(event.privClass());
    newpc->addUser(user);
    this->_membersToPc.insert(userName, newpc);

    emit privchg(event);
    emit privchg(userName, event.adminName(), event.privClass());
}

void dAmnChatroom::notifyKick(const KickEvent& event)
{
    this->removeMember(event.userName());
    emit kicked(event);
    emit kicked(event.userName(), event.kickerName(), event.reason());
}

void dAmnChatroom::notifyPrivUpdate(const PrivUpdateEvent& event)
{
    dAmnPrivClass* pc;

    switch(event.action())
    {
    case PrivUpdateEvent::update:
        pc = this->_privclasses.value(event.privClass());

        if(pc) break;
        else
        {
			MNLIB_WARN("Recieved update on unknown privclass %s in #%s. Creating it.", event.privClass(), this->_name);
        }	// pc missing; fall through to create

    case PrivUpdateEvent::create:
        pc = new dAmnPrivClass(this, event.privClass(), 0);
        break;

	default:
	case PrivUpdateEvent::unknown: qt_noop();
    }

    pc->apply(event.privString());

    emit privUpdate(event);
}

void dAmnChatroom::notifyPrivMove(const PrivMoveEvent& event)
{
    dAmnPrivClass* pc = this->_privclasses.value(event.oldName());

    switch(event.action())
    {
    case PrivMoveEvent::rename:
        pc->setName(event.newName());

    case PrivMoveEvent::move:
    {
        dAmnPrivClass* dest = this->_privclasses.value(event.newName());
        uint count = this->moveAll(pc, dest);
        if(count != event.usersAffected())
            MNLIB_CRIT("Users affected mismatch while moving from privclass %s to %s.", qPrintable(pc->name()), qPrintable(dest->name()));
    }
    }

    emit privMove(event);
}

void dAmnChatroom::notifyPrivRemove(const PrivRemoveEvent& event)
{
    dAmnPrivClass* deleted = this->_privclasses.value(event.privClass()),
                 * def = this->defaultPrivClass();
    // TODO: Handle def not found. Really.
    uint count = this->moveAll(deleted, def);
    if(count != event.usersAffected())
        MNLIB_CRIT("Users affected mismatch while deleting privclass %s.", qPrintable(deleted->name()));

    deleted->deleteLater();

    emit privRemove(event);
}

void dAmnChatroom::notifyPrivShow(const PrivShowEvent& event)
{
    emit privShow(event);
}

void dAmnChatroom::notifyPrivUsers(const PrivUsersEvent& event)
{
    emit privUsers(event);
}

////////////////////////////////////////////////////////////////////////////////

dAmnChatroomIdentifier::dAmnChatroomIdentifier(dAmnSession* parent, const QString& roomstring)
    : _session(parent)
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
            if(this->_session != NULL)
            {
                if(_session->isMe(parts[1]))
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
    _session(parent)
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
        const QString& username = _session->userName();
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
