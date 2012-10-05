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

#ifndef EVENTS_H
#define EVENTS_H

#include "mnlib_global.h"
#include "damnchatroom.h"
#include "timespan.h"

#include <QObject>
#include <QChar>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QList>
#include <QHash>

class dAmnSession;
class dAmnPacket;

class MNLIBSHARED_EXPORT dAmnEvent : public dAmnObject
{
protected:
    dAmnPacket& _packet;
public:
    dAmnEvent(dAmnSession* parent, dAmnPacket& packet);
    virtual ~dAmnEvent();

    dAmnPacket& packet() const;
};

class MNLIBSHARED_EXPORT HandshakeEvent : public dAmnEvent
{
    QString _version;
public:
    HandshakeEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& version() const;
    bool matches() const;
};

class MNLIBSHARED_EXPORT LoginEvent : public dAmnEvent
{
    QString _username;
public:
    enum EventCode
    {
        unknown,
        ok, authentication_failed, not_privileged, too_many_connections
    };
private:
    EventCode _event;
    QString _eventstr;
    QChar _symbol;
    QString _realname, _type, _gpc;
public:
    LoginEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& userName() const;
    EventCode eventCode() const;
    const QString& eventString() const;
    const QChar& symbol() const;
    const QString& realName() const;
    const QString& typeName() const;
    const QString& gpc() const;
};

class MNLIBSHARED_EXPORT ChatroomEvent : public dAmnEvent
{
    dAmnChatroomIdentifier _chatroom;
public:
    ChatroomEvent(dAmnSession* parent, dAmnPacket& packet);
    const dAmnChatroomIdentifier& chatroom() const;
};

class MNLIBSHARED_EXPORT JoinedEvent : public ChatroomEvent
{
public:
    enum EventCode
    {
        unknown,
        ok, not_privileged, inexistant, bad_namespace
    };
private:
    EventCode _event;
    QString _eventstr;
public:
    JoinedEvent(dAmnSession* parent, dAmnPacket& packet);
    EventCode eventCode() const;
    const QString& eventString() const;
};

class MNLIBSHARED_EXPORT PartedEvent : public ChatroomEvent
{
public:
    enum EventCode
    {
        unknown,
        ok, not_joined, bad_namespace
    };
private:
    EventCode _event;
    QString _eventstr;
    QString _reason;
public:
    PartedEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& reason() const;
    EventCode eventCode() const;
    const QString& eventString() const;
};

class MNLIBSHARED_EXPORT PropertyEvent : public ChatroomEvent
{
public:
    enum PropertyCode
    {   // whois is handled by another event type.
        unknown,
        topic, title, privclasses, members
    };
private:
    PropertyCode _property;
    QString _propertystr, _author, _value;
    QDateTime _timestamp;
public:
    PropertyEvent(dAmnSession* parent, dAmnPacket& packet);
    PropertyCode propertyCode() const;
    const QString& propertyString() const;
    const QString& author() const;
    const QDateTime& timeStamp() const;
    const QString& value() const;
};

class MNLIBSHARED_EXPORT WhoisEvent : public dAmnEvent
{
public:
    struct Connection
    {
        TimeSpan online, idle;
        QList<dAmnChatroomIdentifier> chatrooms;
    };
private:
    QString _username, _realname, _type, _gpc;
    uint _usericon;
    QChar _symbol;
    QList<Connection> _connections;
public:
    WhoisEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& userName() const;
    uint userIcon() const;
    const QChar& symbol() const;
    const QString& realName() const;
    const QString& typeName() const;
    const QString& gpc() const;
    const QList<Connection>& connections() const;

private:
    bool parseData();
};

// There's no "RecvEvent" class as the recv packet contains no useful information except a subpacket.

class MNLIBSHARED_EXPORT MsgEvent : public ChatroomEvent
{
    QString _username, _message;
public:
    MsgEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& userName() const;
    const QString& message() const;
};

class MNLIBSHARED_EXPORT ActionEvent : public ChatroomEvent
{
    QString _username, _action;
public:
    ActionEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& userName() const;
    const QString& action() const;
};

class MNLIBSHARED_EXPORT JoinEvent : public ChatroomEvent
{
    QString _username, _props;
public:
    JoinEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& userName() const;
    const QString& properties() const;
};

class MNLIBSHARED_EXPORT PartEvent : public ChatroomEvent
{
    QString _username, _reason;
public:
    PartEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& userName() const;
    const QString& reason() const;
};

class MNLIBSHARED_EXPORT PrivchgEvent : public ChatroomEvent
{
    QString _username, _admin, _privclass;
public:
    PrivchgEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& userName() const;
    const QString& adminName() const;
    const QString& privClass() const;
};

class MNLIBSHARED_EXPORT KickEvent : public ChatroomEvent
{
    QString _username, _kicker, _reason;
public:
    KickEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& userName() const;
    const QString& kickerName() const;
    const QString& reason() const;
};

class MNLIBSHARED_EXPORT PrivUpdateEvent : public ChatroomEvent
{
public:
    enum ActionCode
    {
        unknown,
        create, update
    };
private:
    ActionCode _action;
    QString _actionstr, _username, _privclass, _privstring;
public:
    PrivUpdateEvent(dAmnSession* parent, dAmnPacket& packet);
    ActionCode action() const;
    const QString& actionString() const;
    const QString& userName() const;
    const QString& privClass() const;
    const QString& privString() const;
};

class MNLIBSHARED_EXPORT PrivMoveEvent : public ChatroomEvent
{
public:
    enum ActionCode
    {
        unknown,
        rename, move
    };
private:
    ActionCode _action;
    QString _actionstr, _username, _oldname, _newname;
    int _usersaffected;
public:
    PrivMoveEvent(dAmnSession* parent, dAmnPacket& packet);
    ActionCode action() const;
    const QString& actionString() const;
    const QString& userName() const;
    const QString& oldName() const;
    const QString& newName() const;
    int usersAffected() const;
};

class MNLIBSHARED_EXPORT PrivRemoveEvent : public ChatroomEvent
{
    QString _username, _privclass;
    int _usersaffected;
public:
    PrivRemoveEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& userName() const;
    const QString& privClass() const;
    int usersAffected() const;
};

class MNLIBSHARED_EXPORT PrivShowEvent : public ChatroomEvent
{
    QString _privclass, _privs;
public:
    PrivShowEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& privClass() const;
    const QString& privString() const;
};

class MNLIBSHARED_EXPORT PrivUsersEvent : public ChatroomEvent
{
    QHash<QString, QStringList> _data;
public:
    PrivUsersEvent(dAmnSession* parent, dAmnPacket& packet);
    const QHash<QString, QStringList>& data() const;
    QStringList privClasses() const;
    QStringList usersInPrivClass(const QString& privclass) const;
};

class MNLIBSHARED_EXPORT KickedEvent : public ChatroomEvent
{
    QString _kicker, _reason;
public:
    KickedEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& kicker() const;
    const QString& reason() const;
};

class MNLIBSHARED_EXPORT DisconnectEvent : public dAmnEvent
{
public:
    enum EventCode
    {
        unknown,
        ok, killed, no_login, shutdown
    };
private:
    EventCode _event;
    QString _eventstr;
public:
    DisconnectEvent(dAmnSession* parent, dAmnPacket& packet);
    EventCode eventCode() const;
    const QString& eventString() const;
};

class MNLIBSHARED_EXPORT SendError : public ChatroomEvent
{
public:
    enum ErrorCode
    {
        unknown,
        nothing_to_send, not_privileged, not_open, format_error, bad_command
    };
private:
    ErrorCode _error;
    QString _errormsg;
public:
    SendError(dAmnSession* parent, dAmnPacket& packet);
    ErrorCode error() const;
    const QString& errorMessage() const;
};

class MNLIBSHARED_EXPORT KickError : public ChatroomEvent
{
public:
    enum ErrorCode
    {
        unknown,
        no_such_member, not_privileged
    };
private:
    QString _username;
    ErrorCode _error;
    QString _errormsg;
public:
    KickError(dAmnSession* parent, dAmnPacket& packet);
    ErrorCode error() const;
    const QString& errorMessage() const;
    const QString& userName() const;
};

class MNLIBSHARED_EXPORT GetError : public ChatroomEvent
{
public:
    enum ErrorCode
    {
        unknown,
        not_joined, unknown_property
    };
private:
    QString _property;
    ErrorCode _error;
    QString _errormsg;
public:
    GetError(dAmnSession* parent, dAmnPacket& packet);
    ErrorCode error() const;
    const QString& errorMessage() const;
    const QString& propertyName() const;
};

class MNLIBSHARED_EXPORT SetError : public ChatroomEvent
{
public:
    enum ErrorCode
    {
        unknown,
        not_joined, unknown_property, not_privileged
    };
private:
    QString _property;
    ErrorCode _error;
    QString _errormsg;
public:
    SetError(dAmnSession* parent, dAmnPacket& packet);
    ErrorCode error() const;
    const QString& errorMessage() const;
    const QString& propertyName() const;
};

class MNLIBSHARED_EXPORT KillError : public dAmnEvent
{
public:
    enum ErrorCode
    {
        unknown,
        bad_namespace, not_privileged
    };
private:
    QString _username;
    ErrorCode _error;
    QString _errormsg;
public:
    KillError(dAmnSession* parent, dAmnPacket& packet);
    ErrorCode error() const;
    const QString& errorMessage() const;
    const QString& userName() const;
};

#endif // EVENTS_H
