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

    Type getType() const;
    const QString& getName() const;
    const QString& getTitle() const;
    const QDateTime& getTitleDate() const;
    const QString& getTopic() const;
    const QDateTime& getTopicDate() const;
    QList<dAmnPrivClass*> getPrivclasses() const;
    dAmnChatroomIdentifier getId() const;

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

private:
    Type type;
    QString name, title, topic;
    QDateTime titledate, topicdate;
    QHash<QString, dAmnPrivClass*> privclasses;
    QHash<QString, dAmnPrivClass*> membersToPc;

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
    dAmnSession* session;
};

#endif // DAMNCHATROOM_H
