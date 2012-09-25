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
#include "damnpacketparser.h"

#include <QString>
#include <QChar>
#include <QString>
#include <QDateTime>
#include <QList>
#include <QTextStream>
#include <QPair>

dAmnEvent::dAmnEvent(dAmnSession* parent, dAmnPacket* packet)
          : dAmnObject(parent),
            _packet(packet)
{
}
dAmnEvent::~dAmnEvent()
{
    delete _packet;
}

dAmnPacket& dAmnEvent::packet() const
{
    return *this->_packet;
}
///////////////////////////////////////////////////////////////////////////////
HandshakeEvent::HandshakeEvent(dAmnSession* parent, dAmnPacket* packet)
      : dAmnEvent(parent, packet),
        _version(packet->param())
{
}
const QString& HandshakeEvent::version() const
{
    return this->_version;
}
bool HandshakeEvent::matches() const
{
    return QString(DAMN_VERSION) == this->_version;
}
///////////////////////////////////////////////////////////////////////////////
LoginEvent::LoginEvent(dAmnSession* parent, dAmnPacket* packet)
      : dAmnEvent(parent, packet),
        _username(packet->param()),
        _event(unknown),
        _eventstr(packet->arg("e"))
{
    if(this->_eventstr == "ok")
        this->_event = ok;
    else if(this->_eventstr == "authentication failed")
        this->_event = authentication_failed;
    else if(this->_eventstr == "not privileged")
        this->_event = not_privileged;
    else if(this->_eventstr == "too many connections")
        this->_event = too_many_connections;

    foreach(QString line, packet->data().split('\n'))
    {
        QStringList parts = line.split('=');
        if(parts[0] == "symbol")
            this->_symbol = parts[1][0];
        if(parts[0] == "realname")
            this->_realname = parts[1];
        if(parts[0] == "typename")
              this->_type == parts[1];
        if(parts[0] == "gpc")
            this->_gpc == parts[1];
    }
}

const QString& LoginEvent::userName() const
{
    return this->_username;
}
LoginEvent::EventCode LoginEvent::eventCode() const
{
    return this->_event;
}
const QString& LoginEvent::eventString() const
{
    return this->_eventstr;
}
const QChar& LoginEvent::symbol() const
{
    return this->_symbol;
}
const QString& LoginEvent::realName() const
{
    return this->_realname;
}
const QString& LoginEvent::typeName() const
{
    return this->_type;
}
const QString& LoginEvent::gpc() const
{
    return this->_gpc;
}
///////////////////////////////////////////////////////////////////////////////
ChatroomEvent::ChatroomEvent(dAmnSession* parent, dAmnPacket* packet)
      : dAmnEvent(parent, packet),
        _chatroom(parent, packet->param())
{
}
const dAmnChatroomIdentifier& ChatroomEvent::chatroom() const
{
    return this->_chatroom;
}
///////////////////////////////////////////////////////////////////////////////
JoinedEvent::JoinedEvent(dAmnSession* parent, dAmnPacket* packet)
      : ChatroomEvent(parent, packet),
        _event(unknown),
        _eventstr(packet->arg("e"))
{
    if(this->_eventstr == "ok")
        this->_event = ok;
    else if(this->_eventstr == "not privileged")
        this->_event = not_privileged;
    else if(this->_eventstr == "chatroom doesn't exist")
        this->_event = inexistant;
    else if(this->_eventstr == "bad namespace")
        this->_event = bad_namespace;
}
JoinedEvent::EventCode JoinedEvent::eventCode() const
{
    return this->_event;
}
const QString& JoinedEvent::eventString() const
{
    return this->_eventstr;
}
///////////////////////////////////////////////////////////////////////////////
PartedEvent::PartedEvent(dAmnSession* parent, dAmnPacket* packet)
      : ChatroomEvent(parent, packet),
        _event(unknown),
        _eventstr(packet->arg("e"))
{
    if(this->_eventstr == "ok")
        this->_event = ok;
    else if(this->_eventstr == "not joined")
        this->_event = not_joined;
    else if(this->_eventstr == "bad namespace")
        this->_event = bad_namespace;

    if(packet->arg("r") != NULL)
    {
        this->_reason = packet->arg("r");
    }
}
PartedEvent::EventCode PartedEvent::eventCode() const
{
    return this->_event;
}
const QString& PartedEvent::eventString() const
{
    return this->_eventstr;
}
const QString& PartedEvent::reason() const
{
    return this->_reason;
}
///////////////////////////////////////////////////////////////////////////////
PropertyEvent::PropertyEvent(dAmnSession* parent, dAmnPacket* packet)
      : ChatroomEvent(parent, packet),
        _property(unknown),
        _propertystr(packet->arg("p")),
        _author(packet->arg("by")),
        _value(packet->data())
{
    if(this->_propertystr == "topic")
        this->_property = topic;
    else if(this->_propertystr == "title")
        this->_property = title;
    else if(this->_propertystr == "privclass")
        this->_property = privclasses;
    else if(this->_propertystr == "members")
        this->_property = members;

    bool ok;
    this->_timestamp = QDateTime::fromTime_t(packet->arg("ts").toUInt(&ok, 10));
}
PropertyEvent::PropertyCode PropertyEvent::propertyCode() const
{
    return this->_property;
}
const QString& PropertyEvent::propertyString() const
{
    return this->_propertystr;
}
const QString& PropertyEvent::author() const
{
    return this->_author;
}
const QDateTime& PropertyEvent::timeStamp() const
{
    return this->_timestamp;
}
const QString& PropertyEvent::value() const
{
    return this->_value;
}
///////////////////////////////////////////////////////////////////////////////
WhoisEvent::WhoisEvent(dAmnSession* parent, dAmnPacket* packet)
    : dAmnEvent(parent, packet)
{
    Q_ASSERT(packet->arg("p") == "info");

    this->parseData();
}
bool WhoisEvent::parseData()
{
    QString data (this->_packet->data());
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
        it _can_ change. We should mull on a better way to parse this. */

#   ifdef MNLIB_DEBUG_BUILD
#      define FAIL()    qCritical("%s: Parse error in whois packet. Line: '%s'", __FUNCTION__, qPrintable(line));\
                        return false
#   else
#      define FAIL()    return false
#   endif

    QString line;
    QPair<QString, QString> param;

    while(!(line = parser.readLine()).isEmpty())
    {
        param = dAmnPacketParser::splitPair(line);
        if(param.first == "usericon")
        {
            this->_usericon = param.second.toUInt(&ok, 10);
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
                this->_symbol = param.second[0];
            }
        }
        else if(param.first == "realname")
        {
            this->_realname = param.second;
        }
        else if(param.first == "typename")
        {
            this->_type = param.second;
        }
        else if(param.first == "gpc")
        {
            this->_gpc = param.second;
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
            param = dAmnPacketParser::splitPair(line);

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

        this->_connections << conn;

    } while(!parser.atEnd());

#   undef FAIL

    return true;
}
const QString& WhoisEvent::userName() const
{
    return this->_username;
}
uint WhoisEvent::userIcon() const
{
    return this->_usericon;
}
const QChar& WhoisEvent::symbol() const
{
    return this->_symbol;
}
const QString& WhoisEvent::realName() const
{
    return this->_realname;
}
const QString& WhoisEvent::typeName() const
{
    return this->_type;
}
const QString& WhoisEvent::gpc() const
{
    return this->_gpc;
}
const QList<WhoisEvent::Connection>& WhoisEvent::connections() const
{
    return this->_connections;
}
///////////////////////////////////////////////////////////////////////////////
MsgEvent::MsgEvent(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket& data = packet->subPacket();

    Q_ASSERT(data.param() == "main");

    this->_username = data.arg("from");
    this->_message  = data.data();
}
const QString& MsgEvent::userName() const
{
    return this->_username;
}
const QString& MsgEvent::message() const
{
    return this->_message;
}
///////////////////////////////////////////////////////////////////////////////
ActionEvent::ActionEvent(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket& data = packet->subPacket();

    Q_ASSERT(data.param() == "main");

    this->_username = data.arg("from");
    this->_action   = data.data();
}
const QString& ActionEvent::userName() const
{
    return this->_username;
}
const QString& ActionEvent::action() const
{
    return this->_action;
}
///////////////////////////////////////////////////////////////////////////////
JoinEvent::JoinEvent(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket& data = packet->subPacket();

    this->_username = data.param();
}
const QString& JoinEvent::userName() const
{
    return this->_username;
}
///////////////////////////////////////////////////////////////////////////////
PartEvent::PartEvent(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket& data = packet->subPacket();

    this->_username = data.param();
    this->_reason = data.arg("r");
}
const QString& PartEvent::userName() const
{
    return this->_username;
}
const QString& PartEvent::reason() const
{
    return this->_reason;
}
///////////////////////////////////////////////////////////////////////////////
PrivchgEvent::PrivchgEvent(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket& data = packet->subPacket();

    this->_username = data.param();
    this->_admin = data.arg("by");
    this->_privclass = data.arg("pc");
}
const QString& PrivchgEvent::userName() const
{
    return this->_username;
}
const QString& PrivchgEvent::adminName() const
{
    return this->_admin;
}
const QString& PrivchgEvent::privClass() const
{
    return this->_privclass;
}
///////////////////////////////////////////////////////////////////////////////
KickEvent::KickEvent(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket& data = packet->subPacket();

    this->_username = data.param();
    this->_kicker = data.arg("by");
    this->_reason = data.data();
}
const QString& KickEvent::userName() const
{
    return this->_username;
}
const QString& KickEvent::kickerName() const
{
    return this->_kicker;
}
const QString& KickEvent::reason() const
{
    return this->_reason;
}
///////////////////////////////////////////////////////////////////////////////
PrivUpdateEvent::PrivUpdateEvent(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet),
      _action(unknown)
{
    dAmnPacket& data = packet->subPacket();

    this->_actionstr = data.param();

    if(this->_actionstr == "create")
        this->_action = create;
    else if(this->_actionstr == "update")
        this->_action = update;

    this->_username = data.arg("by");
    this->_privclass = data.arg("name");
    this->_privstring = data.arg("privs");
}
PrivUpdateEvent::ActionCode PrivUpdateEvent::action() const
{
    return this->_action;
}
const QString& PrivUpdateEvent::actionString() const
{
    return this->_actionstr;
}
const QString& PrivUpdateEvent::userName() const
{
    return this->_username;
}
const QString& PrivUpdateEvent::privClass() const
{
    return this->_privclass;
}
const QString& PrivUpdateEvent::privString() const
{
    return this->_privstring;
}
///////////////////////////////////////////////////////////////////////////////
PrivMoveEvent::PrivMoveEvent(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket& data = packet->subPacket();

    this->_actionstr = data.param();

    if(this->_actionstr == "rename")
        this->_action = rename;
    else if(this->_actionstr == "move")
        this->_action = move;

    this->_username = data.arg("by");
    this->_oldname = data.arg("prev");
    this->_newname = data.arg("name");

    bool ok;
    if(this->_action == move)
        this->_usersaffected = data.arg("n").toInt(&ok, 10);
    if(this->_action != move || !ok)
        this->_usersaffected = -1;
}
PrivMoveEvent::ActionCode PrivMoveEvent::action() const
{
    return this->_action;
}
const QString& PrivMoveEvent::actionString() const
{
    return this->_actionstr;
}
const QString& PrivMoveEvent::userName() const
{
    return this->_username;
}
const QString& PrivMoveEvent::oldName() const
{
    return this->_oldname;
}
const QString& PrivMoveEvent::newName() const
{
    return this->_newname;
}
int PrivMoveEvent::usersAffected() const
{
    return this->_usersaffected;
}
///////////////////////////////////////////////////////////////////////////////
PrivRemoveEvent::PrivRemoveEvent(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket& data = packet->subPacket();

    this->_username = data.arg("by");
    this->_privclass = data.arg("name");
    bool ok;
    this->_usersaffected = data.arg("n").toInt(&ok, 10);
    if(!ok) this->_usersaffected = -1;
}
const QString& PrivRemoveEvent::userName() const
{
    return this->_username;
}
const QString& PrivRemoveEvent::privClass() const
{
    return this->_privclass;
}
int PrivRemoveEvent::usersAffected() const
{
    return this->_usersaffected;
}
///////////////////////////////////////////////////////////////////////////////
PrivShowEvent::PrivShowEvent(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket& data = packet->subPacket();

    int split = data.data().indexOf(' ');
    this->_privclass = data.data().mid(0, split);
    this->_privs = data.data().mid(split + 1);
}
const QString& PrivShowEvent::privClass() const
{
    return this->_privclass;
}
const QString& PrivShowEvent::privString() const
{
    return this->_privs;
}
///////////////////////////////////////////////////////////////////////////////
PrivUsersEvent::PrivUsersEvent(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet)
{
    dAmnPacket& dataPacket = packet->subPacket();
    QString data = dataPacket.data();

    QTextStream parser (&data);
    while(!parser.atEnd())
    {
        QString name;
        parser >> name;
        name.truncate(name.indexOf(':'));

        this->_data[name] = parser.readLine().split(' ');
    }
}
const QHash<QString, QStringList>& PrivUsersEvent::data() const
{
    return this->_data;
}
QStringList PrivUsersEvent::privClasses() const
{
    return this->_data.keys();
}
QStringList PrivUsersEvent::usersInPrivClass(const QString& privclass) const
{
    return this->_data[privclass];
}
///////////////////////////////////////////////////////////////////////////////
KickedEvent::KickedEvent(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet),
      _kicker(packet->arg("by")),
      _reason(packet->data())
{
}
const QString& KickedEvent::kicker() const
{
    return this->_kicker;
}
const QString& KickedEvent::reason() const
{
    return this->_reason;
}
///////////////////////////////////////////////////////////////////////////////
DisconnectEvent::DisconnectEvent(dAmnSession* parent, dAmnPacket* packet)
    : dAmnEvent(parent, packet),
      _event(unknown),
      _eventstr(packet->arg("e"))
{
    if(this->_eventstr == "ok")
        this->_event = ok;
    else if(this->_eventstr == "killed")
        this->_event = killed;
    else if(this->_eventstr == "no login")
        this->_event = no_login;
    else if(this->_eventstr == "shutdown")
        this->_event = shutdown;
}
DisconnectEvent::EventCode DisconnectEvent::eventCode() const
{
    return this->_event;
}
const QString& DisconnectEvent::eventString() const
{
    return this->_eventstr;
}
///////////////////////////////////////////////////////////////////////////////
SendError::SendError(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet),
      _error(unknown),
      _errormsg(packet->arg("e"))
{
    if(this->_errormsg == "nothing to send")
        this->_error = nothing_to_send;
    else if(this->_errormsg == "not privileged")
        this->_error = not_privileged;
    else if(this->_errormsg == "not open")
        this->_error = not_open;
    else if(this->_errormsg == "format error")
        this->_error = format_error;
    else if(this->_errormsg == "bad command")
        this->_error = bad_command;
}
SendError::ErrorCode SendError::error() const
{
    return this->_error;
}
const QString& SendError::errorMessage() const
{
    return this->_errormsg;
}
///////////////////////////////////////////////////////////////////////////////
KickError::KickError(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet),
      _username(packet->args()["u"]),
      _error(unknown),
      _errormsg(packet->args()["e"])
{
    if(this->_errormsg == "no such member")
        this->_error = no_such_member;
    else if(this->_errormsg == "not privileged")
        this->_error = not_privileged;
}
KickError::ErrorCode KickError::error() const
{
    return this->_error;
}
const QString& KickError::errorMessage() const
{
    return this->_errormsg;
}
const QString& KickError::userName() const
{
    return this->_username;
}
///////////////////////////////////////////////////////////////////////////////
GetError::GetError(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet),
      _property(packet->arg("p")),
      _error(unknown),
      _errormsg(packet->arg("e"))
{
    if(this->_errormsg == "not joined")
        this->_error = not_joined;
    else if(this->_errormsg == "unknown property")
        this->_error = unknown_property;
}
GetError::ErrorCode GetError::error() const
{
    return this->_error;
}
const QString& GetError::errorMessage() const
{
    return this->_errormsg;
}
const QString& GetError::propertyName() const
{
    return this->_property;
}
///////////////////////////////////////////////////////////////////////////////
SetError::SetError(dAmnSession* parent, dAmnPacket* packet)
    : ChatroomEvent(parent, packet),
      _property(packet->arg("p")),
      _error(unknown),
      _errormsg(packet->arg("e"))
{
    if(this->_errormsg == "not joined")
        this->_error = not_joined;
    else if(this->_errormsg == "unknown property")
        this->_error = unknown_property;
    else if(this->_errormsg == "not privileged")
        this->_error = not_privileged;
}
SetError::ErrorCode SetError::error() const
{
    return this->_error;
}
const QString& SetError::errorMessage() const
{
    return this->_errormsg;
}
const QString& SetError::propertyName() const
{
    return this->_property;
}
///////////////////////////////////////////////////////////////////////////////
KillError::KillError(dAmnSession* parent, dAmnPacket* packet)
    : dAmnEvent(parent, packet),
      _error(unknown),
      _errormsg(packet->arg("e"))
{
    this->_username = packet->param().split(':')[1];

    if(this->_errormsg == "bad namespace")
        this->_error = bad_namespace;
    else if(this->_errormsg == "not privileged")
        this->_error = not_privileged;
}
KillError::ErrorCode KillError::error() const
{
    return this->_error;
}
const QString& KillError::errorMessage() const
{
    return this->_errormsg;
}
const QString& KillError::userName() const
{
    return this->_username;
}
