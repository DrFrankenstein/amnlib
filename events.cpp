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

#include "events.h"
#include "damnpacket.h"
#include "damnchatroom.h"
#include "damnsession.h"
#include "timespan.h"

#include <QString>
#include <QChar>
#include <QString>
#include <QDateTime>
#include <QList>
#include <QTextStream>
#include <QPair>

dAmnEvent::dAmnEvent(dAmnSession* parent, const dAmnPacket& packet)
          : dAmnObject(parent),
            packet(packet)
{
}
const dAmnPacket& dAmnEvent::getPacket() const
{
    return this->packet;
}
///////////////////////////////////////////////////////////////////////////////
HandshakeEvent::HandshakeEvent(dAmnSession* parent, const dAmnPacket& packet)
      : dAmnEvent(parent, packet),
        version(packet.getParam())
{
}
const QString& HandshakeEvent::getVersion() const
{
    return this->version;
}
bool HandshakeEvent::matches() const
{
    return QString(DAMN_VERSION) == this->version;
}
///////////////////////////////////////////////////////////////////////////////
LoginEvent::LoginEvent(dAmnSession* parent, const dAmnPacket& packet)
      : dAmnEvent(parent, packet),
        username(packet.getParam()),
        event(unknown),
        eventstr(packet["e"])
{
    if(this->eventstr == "ok")
        this->event = ok;
    else if(this->eventstr == "authentication failed")
        this->event = authentication_failed;
    else if(this->eventstr == "not privileged")
        this->event = not_privileged;
    else if(this->eventstr == "too many connections")
        this->event = too_many_connections;

    foreach(QString line, packet.getData().split('\n'))
    {
        QStringList parts = line.split('=');
        if(parts[0] == "symbol")
            this->symbol = parts[1][0];
        if(parts[0] == "realname")
            this->realname = parts[1];
        if(parts[0] == "typename")
              this->type == parts[1];
        if(parts[0] == "gpc")
            this->gpc == parts[1];
    }
}

const QString& LoginEvent::getUserName() const
{
    return this->username;
}
LoginEvent::EventCode LoginEvent::getEvent() const
{
    return this->event;
}
const QString& LoginEvent::getEventString() const
{
    return this->eventstr;
}
const QChar& LoginEvent::getSymbol() const
{
    return this->symbol;
}
const QString& LoginEvent::getRealName() const
{
    return this->realname;
}
const QString& LoginEvent::getTypeName() const
{
    return this->type;
}
const QString& LoginEvent::getGpc() const
{
    return this->gpc;
}
///////////////////////////////////////////////////////////////////////////////
ChatroomEvent::ChatroomEvent(dAmnSession* parent, const dAmnPacket& packet)
      : dAmnEvent(parent, packet),
        chatroom(parent, packet.getParam())
{
}
const dAmnChatroomIdentifier& ChatroomEvent::getChatroom() const
{
    return this->chatroom;
}
///////////////////////////////////////////////////////////////////////////////
JoinedEvent::JoinedEvent(dAmnSession* parent, const dAmnPacket& packet)
      : ChatroomEvent(parent, packet),
        event(unknown),
        eventstr(packet["e"])
{
    if(this->eventstr == "ok")
        this->event = ok;
    else if(this->eventstr == "not privileged")
        this->event = not_privileged;
    else if(this->eventstr == "chatroom doesn't exist")
        this->event = inexistant;
    else if(this->eventstr == "bad namespace")
        this->event = bad_namespace;
}
JoinedEvent::EventCode JoinedEvent::getEvent() const
{
    return this->event;
}
const QString& JoinedEvent::getEventString() const
{
    return this->eventstr;
}
///////////////////////////////////////////////////////////////////////////////
PartedEvent::PartedEvent(dAmnSession* parent, const dAmnPacket& packet)
      : ChatroomEvent(parent, packet),
        event(unknown),
        eventstr(packet["e"])
{
    if(this->eventstr == "ok")
        this->event = ok;
    else if(this->eventstr == "not joined")
        this->event = not_joined;
    else if(this->eventstr == "bad namespace")
        this->event = bad_namespace;

    if(packet["r"] != NULL)
    {
        this->reason = packet["r"];
    }
}
PartedEvent::EventCode PartedEvent::getEvent() const
{
    return this->event;
}
const QString& PartedEvent::getEventString() const
{
    return this->eventstr;
}
const QString& PartedEvent::getReason() const
{
    return this->reason;
}
///////////////////////////////////////////////////////////////////////////////
PropertyEvent::PropertyEvent(dAmnSession* parent, const dAmnPacket& packet)
      : ChatroomEvent(parent, packet),
        property(unknown),
        propertystr(packet["e"]),
        author(packet["by"])
{
    if(this->propertystr == "topic")
        this->property = topic;
    else if(this->propertystr == "title")
        this->property = title;
    else if(this->propertystr == "privclass")
        this->property = privclasses;
    else if(this->propertystr == "members")
        this->property = members;

    bool ok;
    this->timestamp = QDateTime::fromTime_t(packet["ts"].toUInt(&ok, 10));
}
PropertyEvent::PropertyCode PropertyEvent::getProperty() const
{
    return this->property;
}
const QString& PropertyEvent::getPropertyString() const
{
    return this->propertystr;
}
const QString& PropertyEvent::getAuthor() const
{
    return this->author;
}
const QDateTime& PropertyEvent::getTimeStamp() const
{
    return this->timestamp;
}
const QString& PropertyEvent::getValue() const
{
    return this->value;
}
///////////////////////////////////////////////////////////////////////////////
WhoisEvent::WhoisEvent(dAmnSession* parent, const dAmnPacket& packet)
    : dAmnEvent(parent, packet)
{
    Q_ASSERT(packet["p"] == "info");

    this->parseData();
}
bool WhoisEvent::parseData()
{
    QString data (this->packet.getData());
    QTextStream parser (&data);

    bool ok;
    /* This assumes:
        usericon=<int>
        symbol=<string[1]>
        realname=<string>
        typename=<string>
        gpc=<string>

        (conn
        online=<int>
        idle=<int>

        (ns <string>

        )*

        )*

        and fails as soon as the packet is not too well-formed.
        Although this should not be too much of a problem if the protocol doesn't change,
        it _can_ change. We should mull on a better wat to parse this. */

#   ifdef MNLIB_DEBUG_BUILD
#      define FAIL()    qCritical("%s: Parse error in whois packet. Line: '%s'", __FUNCTION__, line.toLatin1().data());\
                        return false
#   else
#      define FAIL()    return false
#   endif

    QString line;
    QPair<QString, QString> param;

    while(!(line = parser.readLine()).isEmpty())
    {
        param = dAmnPacket::parsePair(line);
        if(param.first == "usericon")
        {
            this->usericon = param.second.toUInt(&ok, 10);
            if(!ok) FAIL();
        }
        else if(param.first == "symbol")
        {
            if(param.second.size() > 1)
            {
                FAIL();
            }
            else
            {
                this->symbol = param.second[0];
            }
        }
        else if(param.first == "realname")
        {
            this->realname = param.second;
        }
        else if(param.first == "typename")
        {
            this->type = param.second;
        }
        else if(param.first == "gpc")
        {
            this->gpc = param.second;
        }
        else
        {
            FAIL();
        }
    }

    line = parser.readLine();
    do
    {
        if(line != "conn") FAIL();

        Connection conn;

        while(!(line = parser.readLine()).isEmpty())
        {
            param = dAmnPacket::parsePair(line);

            if(param.first == "online")
                conn.online = TimeSpan(param.second.toUInt(&ok, 10));
            else if (param.first == "idle")
                conn.idle = TimeSpan(param.second.toUInt(&ok, 10));

            if(!ok) FAIL();
        }

        line = parser.readLine();
        do
        {
            conn.chatrooms << dAmnChatroomIdentifier(this->session(), line.split(' ')[1]);
            parser.readLine();
            line = parser.readLine();
        } while(line.startsWith("ns"));

        this->connections << conn;

    } while(!parser.atEnd());

#   undef FAIL

    return true;
}
const QString& WhoisEvent::getUserName() const
{
    return this->username;
}
uint WhoisEvent::getUserIcon() const
{
    return this->usericon;
}
const QChar& WhoisEvent::getSymbol() const
{
    return this->symbol;
}
const QString& WhoisEvent::getRealName() const
{
    return this->realname;
}
const QString& WhoisEvent::getTypeName() const
{
    return this->type;
}
const QString& WhoisEvent::getGpc() const
{
    return this->gpc;
}
const QList<WhoisEvent::Connection>& WhoisEvent::getConnections() const
{
    return this->connections;
}
///////////////////////////////////////////////////////////////////////////////
MsgEvent::MsgEvent(dAmnSession* parent, const dAmnPacket& packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket* data = packet.getSubPacket();

    Q_ASSERT(data->getParam() == "main");

    this->username = (*data)["from"];
    this->message  = data->getData();

    delete data;
}
const QString& MsgEvent::getUserName() const
{
    return this->username;
}
const QString& MsgEvent::getMessage() const
{
    return this->message;
}
///////////////////////////////////////////////////////////////////////////////
ActionEvent::ActionEvent(dAmnSession* parent, const dAmnPacket& packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket* data = packet.getSubPacket();

    Q_ASSERT(data->getParam() == "main");

    this->username = (*data)["from"];
    this->action   = data->getData();

    delete data;
}
const QString& ActionEvent::getUserName() const
{
    return this->username;
}
const QString& ActionEvent::getAction() const
{
    return this->action;
}
///////////////////////////////////////////////////////////////////////////////
JoinEvent::JoinEvent(dAmnSession* parent, const dAmnPacket& packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket* data = packet.getSubPacket();

    this->username = data->getParam();

    delete data;
}
const QString& JoinEvent::getUserName() const
{
    return this->username;
}
///////////////////////////////////////////////////////////////////////////////
PartEvent::PartEvent(dAmnSession* parent, const dAmnPacket& packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket* data = packet.getSubPacket();

    this->username = data->getParam();
    this->reason = (*data)["r"];

    delete data;
}
const QString& PartEvent::getUserName() const
{
    return this->username;
}
const QString& PartEvent::getReason() const
{
    return this->reason;
}
///////////////////////////////////////////////////////////////////////////////
PrivchgEvent::PrivchgEvent(dAmnSession* parent, const dAmnPacket& packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket* data = packet.getSubPacket();

    this->username = data->getParam();
    this->admin = (*data)["by"];
    this->privclass = (*data)["pc"];

    delete data;
}
const QString& PrivchgEvent::getUserName() const
{
    return this->username;
}
const QString& PrivchgEvent::getAdminName() const
{
    return this->admin;
}
const QString& PrivchgEvent::getPrivClass() const
{
    return this->privclass;
}
///////////////////////////////////////////////////////////////////////////////
KickEvent::KickEvent(dAmnSession* parent, const dAmnPacket& packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket* data = packet.getSubPacket();

    this->username = data->getParam();
    this->kicker = (*data)["by"];
    this->reason = data->getData();

    delete data;
}
const QString& KickEvent::getUserName() const
{
    return this->username;
}
const QString& KickEvent::getKickerName() const
{
    return this->kicker;
}
const QString& KickEvent::getReason() const
{
    return this->reason;
}
///////////////////////////////////////////////////////////////////////////////
PrivUpdateEvent::PrivUpdateEvent(dAmnSession* parent, const dAmnPacket& packet)
    : ChatroomEvent(parent, packet),
      action(unknown)
{
    dAmnPacket* data = packet.getSubPacket();

    this->actionstr = data->getParam();

    if(this->actionstr == "create")
        this->action = create;
    else if(this->actionstr == "update")
        this->action = update;

    this->username = (*data)["by"];
    this->privclass = (*data)["name"];
    this->privstring = (*data)["privs"];

    delete data;
}
PrivUpdateEvent::ActionCode PrivUpdateEvent::getAction() const
{
    return this->action;
}
const QString& PrivUpdateEvent::getActionString() const
{
    return this->actionstr;
}
const QString& PrivUpdateEvent::getUserName() const
{
    return this->username;
}
const QString& PrivUpdateEvent::getPrivClass() const
{
    return this->privclass;
}
const QString& PrivUpdateEvent::getPrivString() const
{
    return this->privstring;
}
///////////////////////////////////////////////////////////////////////////////
PrivMoveEvent::PrivMoveEvent(dAmnSession* parent, const dAmnPacket& packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket* data = packet.getSubPacket();

    this->actionstr = data->getParam();

    if(this->actionstr == "rename")
        this->action = rename;
    else if(this->actionstr == "move")
        this->action = move;

    this->username = (*data)["by"];
    this->oldname = (*data)["prev"];
    this->newname = (*data)["name"];

    bool ok;
    if(this->action == move)
        this->usersaffected = (*data)["n"].toInt(&ok, 10);
    if(this->action != move || !ok)
        this->usersaffected = -1;

    delete data;
}
PrivMoveEvent::ActionCode PrivMoveEvent::getAction() const
{
    return this->action;
}
const QString& PrivMoveEvent::getActionString() const
{
    return this->actionstr;
}
const QString& PrivMoveEvent::getUserName() const
{
    return this->username;
}
const QString& PrivMoveEvent::getOldName() const
{
    return this->oldname;
}
const QString& PrivMoveEvent::getNewName() const
{
    return this->newname;
}
int PrivMoveEvent::getUsersAffected() const
{
    return this->usersaffected;
}
///////////////////////////////////////////////////////////////////////////////
PrivRemoveEvent::PrivRemoveEvent(dAmnSession* parent, const dAmnPacket &packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket* data = packet.getSubPacket();

    this->username = (*data)["by"];
    this->privclass = (*data)["name"];
    bool ok;
    this->usersaffected = (*data)["n"].toInt(&ok, 10);
    if(!ok) this->usersaffected = -1;

    delete data;
}
const QString& PrivRemoveEvent::getUserName() const
{
    return this->username;
}
const QString& PrivRemoveEvent::getPrivClass() const
{
    return this->privclass;
}
int PrivRemoveEvent::getUsersAffected() const
{
    return this->usersaffected;
}
///////////////////////////////////////////////////////////////////////////////
PrivShowEvent::PrivShowEvent(dAmnSession* parent, const dAmnPacket& packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket* data = packet.getSubPacket();

    int split = data->getData().indexOf(' ');
    this->privclass = data->getData().mid(0, split);
    this->privs = data->getData().mid(split + 1);

    delete data;
}
const QString& PrivShowEvent::getPrivClass() const
{
    return this->privclass;
}
const QString& PrivShowEvent::getPrivString() const
{
    return this->privs;
}
///////////////////////////////////////////////////////////////////////////////
PrivUsersEvent::PrivUsersEvent(dAmnSession* parent, const dAmnPacket& packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket* dataPacket = packet.getSubPacket();
    QString data = dataPacket->getData();

    QTextStream parser (&data);
    while(!parser.atEnd())
    {
        QString name;
        parser >> name;
        name.truncate(name.indexOf(':'));

        this->data[name] = parser.readLine().split(' ');
    }

    delete dataPacket;
}
const QHash<QString, QStringList>& PrivUsersEvent::getData() const
{
    return this->data;
}
QStringList PrivUsersEvent::getPrivClasses() const
{
    return this->data.keys();
}
QStringList PrivUsersEvent::getUsersInPrivClass(const QString& privclass) const
{
    return this->data[privclass];
}
///////////////////////////////////////////////////////////////////////////////
KickedEvent::KickedEvent(dAmnSession* parent, const dAmnPacket& packet)
    : ChatroomEvent(parent, packet),
      kicker(packet["by"]),
      reason(packet.getData())
{
}
const QString& KickedEvent::getKicker() const
{
    return this->kicker;
}
const QString& KickedEvent::getReason() const
{
    return this->reason;
}
///////////////////////////////////////////////////////////////////////////////
DisconnectEvent::DisconnectEvent(dAmnSession* parent, const dAmnPacket& packet)
    : dAmnEvent(parent, packet),
      event(unknown),
      eventstr(packet["e"])
{
    if(this->eventstr == "ok")
        this->event = ok;
    else if(this->eventstr == "killed")
        this->event = killed;
    else if(this->eventstr == "no login")
        this->event = no_login;
    else if(this->eventstr == "shutdown")
        this->event = shutdown;
}
DisconnectEvent::EventCode DisconnectEvent::getEvent() const
{
    return this->event;
}
const QString& DisconnectEvent::getEventString() const
{
    return this->eventstr;
}
///////////////////////////////////////////////////////////////////////////////
SendError::SendError(dAmnSession* parent, const dAmnPacket& packet)
    : ChatroomEvent(parent, packet),
      error(unknown),
      errormsg(packet["e"])
{
    if(this->errormsg == "nothing to send")
        this->error = nothing_to_send;
    else if(this->errormsg == "not privileged")
        this->error = not_privileged;
    else if(this->errormsg == "not open")
        this->error = not_open;
    else if(this->errormsg == "format error")
        this->error = format_error;
    else if(this->errormsg == "bad command")
        this->error = bad_command;
}
SendError::ErrorCode SendError::getError() const
{
    return this->error;
}
const QString& SendError::getErrorMessage() const
{
    return this->errormsg;
}
///////////////////////////////////////////////////////////////////////////////
KickError::KickError(dAmnSession* parent, const dAmnPacket& packet)
    : ChatroomEvent(parent, packet),
      username(packet["u"]),
      error(unknown),
      errormsg(packet["e"])
{
    if(this->errormsg == "no such member")
        this->error = no_such_member;
    else if(this->errormsg == "not privileged")
        this->error = not_privileged;
}
KickError::ErrorCode KickError::getError() const
{
    return this->error;
}
const QString& KickError::getErrorMessage() const
{
    return this->errormsg;
}
const QString& KickError::getUserName() const
{
    return this->username;
}
///////////////////////////////////////////////////////////////////////////////
GetError::GetError(dAmnSession* parent, const dAmnPacket& packet)
    : ChatroomEvent(parent, packet),
      property(packet["p"]),
      error(unknown),
      errormsg(packet["e"])
{
    if(this->errormsg == "not joined")
        this->error = not_joined;
    else if(this->errormsg == "unknown property")
        this->error = unknown_property;
}
GetError::ErrorCode GetError::getError() const
{
    return this->error;
}
const QString& GetError::getErrorMessage() const
{
    return this->errormsg;
}
const QString& GetError::getProperty() const
{
    return this->property;
}
///////////////////////////////////////////////////////////////////////////////
SetError::SetError(dAmnSession* parent, const dAmnPacket& packet)
    : ChatroomEvent(parent, packet),
      property(packet["p"]),
      error(unknown),
      errormsg(packet["e"])
{
    if(this->errormsg == "not joined")
        this->error = not_joined;
    else if(this->errormsg == "unknown property")
        this->error = unknown_property;
    else if(this->errormsg == "not privileged")
        this->error = not_privileged;
}
SetError::ErrorCode SetError::getError() const
{
    return this->error;
}
const QString& SetError::getErrorMessage() const
{
    return this->errormsg;
}
const QString& SetError::getProperty() const
{
    return this->property;
}
///////////////////////////////////////////////////////////////////////////////
KillError::KillError(dAmnSession* parent, const dAmnPacket& packet)
    : dAmnEvent(parent, packet),
      error(unknown),
      errormsg(packet["e"])
{
    this->username = packet.getParam().split(':')[1];

    if(this->errormsg == "bad namespace")
        this->error = bad_namespace;
    else if(this->errormsg == "not privileged")
        this->error = not_privileged;
}
KillError::ErrorCode KillError::getError() const
{
    return this->error;
}
const QString& KillError::getErrorMessage() const
{
    return this->errormsg;
}
const QString& KillError::getUserName() const
{
    return this->username;
}
