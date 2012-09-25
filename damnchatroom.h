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

#ifndef DAMNCHATROOM_H
#define DAMNCHATROOM_H

#include "mnlib_global.h"
#include "damnobject.h"

#include <QString>
#include <QDateTime>
#include <QHash>

template <typename T> class QList;
class QByteArray;

class dAmnChatroom;
struct dAmnChatroomIdentifier;
class dAmnSession;
class dAmnPrivClass;
class dAmnUser;
class dAmnPacket;

class MNLIBSHARED_EXPORT dAmnChatroom : public dAmnObject
{
public:
    enum Type
    {
        chat, pchat
    };

    dAmnChatroom(dAmnSession* parent, const QString& roomstring);
    dAmnChatroom(dAmnSession* parent, const dAmnChatroomIdentifier& id);

    Type type() const;
    const QString& name() const;
    const QString& title() const;
    const QDateTime& titleDate() const;
    const QString& topic() const;
    const QDateTime& topicDate() const;
    QList<dAmnPrivClass*> privclasses() const;
    dAmnChatroomIdentifier id() const;

    void setTopic(const QString& newtopic);
    void setTitle(const QString& newtitle);

    void addPrivclass(dAmnPrivClass* pc);
    void removePrivclass(const QString& name);

    void updatePrivclasses(const QString& data);

    void processMembers(const QString& data);

    void part();

    void say(const QString& message);
    void act(const QString& action);
    void npmsg(const QString& message);

    void promote(const dAmnUser& user);
    void promote(const QString& username);
    void demote(const dAmnUser& user);
    void demote(const QString& username);
    void chgPrivclass(const dAmnUser& user, const QString& privclass);

    void kick(const dAmnUser& user, const QString& reason = QString());
    void ban(const dAmnUser& user);
    void unban(const dAmnUser& user);

    void getRoomProperty(const QString& property);
    void setRoomProperty(const QString& property, const QString& value);

    void sendAdminCommand(const QString& command);

signals:
    void message(const QString& user, const QString& content);
    void action(const QString& user, const QString& content);

    void joined(const QString& user);
    void parted(const QString& user, const QString& reason);

    void privchg(const QString& user, const QString& by, const QString& pc);
    void kicked(const QString& user, const QString& by, const QString& reason);

private:
    Type _type;
    QString _name, _title, _topic;
    QDateTime _titledate, _topicdate;
    QHash<QString, dAmnPrivClass*> _privclasses;
    QHash<QString, dAmnPrivClass*> _membersToPc;

    void send(const dAmnPacket& packet);

    void addMember(const QString& name, const QString& pcname, int usericon, const QChar& symbol, const QString& realname, const QString& type_name);
    void removeMember(const QString& name);
};

struct MNLIBSHARED_EXPORT dAmnChatroomIdentifier // NOT a QObject.
{
    dAmnChatroom::Type type;
    QString name;

    dAmnChatroomIdentifier(dAmnSession* parent, const QString& roomstring);
    dAmnChatroomIdentifier(dAmnSession* parent, dAmnChatroom::Type type, const QString& name);

    bool operator ==(const dAmnChatroomIdentifier& rhs) const;

    QString toString() const;
    QString toIdString() const;

private:
    dAmnSession* _session;
};

#endif // DAMNCHATROOM_H
