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
    dAmnPacket& packet;
public:
    dAmnEvent(dAmnSession* parent, dAmnPacket& packet);
    dAmnPacket& getPacket() const;
};

class MNLIBSHARED_EXPORT HandshakeEvent : public dAmnEvent
{
    QString version;
public:
    HandshakeEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& getVersion() const;
    bool matches() const;
};

class MNLIBSHARED_EXPORT LoginEvent : public dAmnEvent
{
    QString username;
public:
    enum EventCode
    {
        unknown,
        ok, authentication_failed, not_privileged, too_many_connections
    };
private:
    EventCode event;
    QString eventstr;
    QChar symbol;
    QString realname, type, gpc;
public:
    LoginEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& getUserName() const;
    EventCode getEvent() const;
    const QString& getEventString() const;
    const QChar& getSymbol() const;
    const QString& getRealName() const;
    const QString& getTypeName() const;
    const QString& getGpc() const;
};

class MNLIBSHARED_EXPORT ChatroomEvent : public dAmnEvent
{
    dAmnChatroomIdentifier chatroom;
public:
    ChatroomEvent(dAmnSession* parent, dAmnPacket& packet);
    const dAmnChatroomIdentifier& getChatroom() const;
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
    EventCode event;
    QString eventstr;
public:
    JoinedEvent(dAmnSession* parent, dAmnPacket& packet);
    EventCode getEvent() const;
    const QString& getEventString() const;
};

class MNLIBSHARED_EXPORT PartedEvent : public ChatroomEvent
{
    QString reason;
public:
    enum EventCode
    {
        unknown,
        ok, not_joined, bad_namespace
    };
private:
    EventCode event;
    QString eventstr;
public:
    PartedEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& getReason() const;
    EventCode getEvent() const;
    const QString& getEventString() const;
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
    PropertyCode property;
    QString propertystr, author, value;
    QDateTime timestamp;
public:
    PropertyEvent(dAmnSession* parent, dAmnPacket& packet);
    PropertyCode getProperty() const;
    const QString& getPropertyString() const;
    const QString& getAuthor() const;
    const QDateTime& getTimeStamp() const;
    const QString& getValue() const;
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
    QString username, realname, type, gpc;
    uint usericon;
    QChar symbol;
    QList<Connection> connections;
public:
    WhoisEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& getUserName() const;
    uint getUserIcon() const;
    const QChar& getSymbol() const;
    const QString& getRealName() const;
    const QString& getTypeName() const;
    const QString& getGpc() const;
    const QList<Connection>& getConnections() const;

private:
    bool parseData();
};

// There's no "RecvEvent" class as the recv packet contains no useful information except a subpacket.

class MNLIBSHARED_EXPORT MsgEvent : public ChatroomEvent
{
    QString username, message;
public:
    MsgEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& getUserName() const;
    const QString& getMessage() const;
};

class MNLIBSHARED_EXPORT ActionEvent : public ChatroomEvent
{
    QString username, action;
public:
    ActionEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& getUserName() const;
    const QString& getAction() const;
};

class MNLIBSHARED_EXPORT JoinEvent : public ChatroomEvent
{
    QString username;
public:
    JoinEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& getUserName() const;
};

class MNLIBSHARED_EXPORT PartEvent : public ChatroomEvent
{
    QString username, reason;
public:
    PartEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& getUserName() const;
    const QString& getReason() const;
};

class MNLIBSHARED_EXPORT PrivchgEvent : public ChatroomEvent
{
    QString username, admin, privclass;
public:
    PrivchgEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& getUserName() const;
    const QString& getAdminName() const;
    const QString& getPrivClass() const;
};

class MNLIBSHARED_EXPORT KickEvent : public ChatroomEvent
{
    QString username, kicker, reason;
public:
    KickEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& getUserName() const;
    const QString& getKickerName() const;
    const QString& getReason() const;
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
    ActionCode action;
    QString actionstr, username, privclass, privstring;
public:
    PrivUpdateEvent(dAmnSession* parent, dAmnPacket& packet);
    ActionCode getAction() const;
    const QString& getActionString() const;
    const QString& getUserName() const;
    const QString& getPrivClass() const;
    const QString& getPrivString() const;
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
    ActionCode action;
    QString actionstr, username, oldname, newname;
    int usersaffected;
public:
    PrivMoveEvent(dAmnSession* parent, dAmnPacket& packet);
    ActionCode getAction() const;
    const QString& getActionString() const;
    const QString& getUserName() const;
    const QString& getOldName() const;
    const QString& getNewName() const;
    int getUsersAffected() const;
};

class MNLIBSHARED_EXPORT PrivRemoveEvent : public ChatroomEvent
{
    QString username, privclass;
    int usersaffected;
public:
    PrivRemoveEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& getUserName() const;
    const QString& getPrivClass() const;
    int getUsersAffected() const;
};

class MNLIBSHARED_EXPORT PrivShowEvent : public ChatroomEvent
{
    QString privclass, privs;
public:
    PrivShowEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& getPrivClass() const;
    const QString& getPrivString() const;
};

class MNLIBSHARED_EXPORT PrivUsersEvent : public ChatroomEvent
{
    QHash<QString, QStringList> data;
public:
    PrivUsersEvent(dAmnSession* parent, dAmnPacket& packet);
    const QHash<QString, QStringList>& getData() const;
    QStringList getPrivClasses() const;
    QStringList getUsersInPrivClass(const QString& privclass) const;
};

class MNLIBSHARED_EXPORT KickedEvent : public ChatroomEvent
{
    QString kicker, reason;
public:
    KickedEvent(dAmnSession* parent, dAmnPacket& packet);
    const QString& getKicker() const;
    const QString& getReason() const;
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
    EventCode event;
    QString eventstr;
public:
    DisconnectEvent(dAmnSession* parent, dAmnPacket& packet);
    EventCode getEvent() const;
    const QString& getEventString() const;
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
    ErrorCode error;
    QString errormsg;
public:
    SendError(dAmnSession* parent, dAmnPacket& packet);
    ErrorCode getError() const;
    const QString& getErrorMessage() const;
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
    QString username;
    ErrorCode error;
    QString errormsg;
public:
    KickError(dAmnSession* parent, dAmnPacket& packet);
    ErrorCode getError() const;
    const QString& getErrorMessage() const;
    const QString& getUserName() const;
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
    QString property;
    ErrorCode error;
    QString errormsg;
public:
    GetError(dAmnSession* parent, dAmnPacket& packet);
    ErrorCode getError() const;
    const QString& getErrorMessage() const;
    const QString& getProperty() const;
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
    QString property;
    ErrorCode error;
    QString errormsg;
public:
    SetError(dAmnSession* parent, dAmnPacket& packet);
    ErrorCode getError() const;
    const QString& getErrorMessage() const;
    const QString& getProperty() const;
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
    QString username;
    ErrorCode error;
    QString errormsg;
public:
    KillError(dAmnSession* parent, dAmnPacket& packet);
    ErrorCode getError() const;
    const QString& getErrorMessage() const;
    const QString& getUserName() const;
};

#endif // EVENTS_H
