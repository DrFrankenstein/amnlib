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

#include "damnsession.h"
#include "damnpacket.h"
#include "events.h"

#include <QHostAddress>
#include <QRegExp>

dAmnSession::dAmnSession(const QByteArray& token)
    : state(offline), packetdevice(this, this->socket), auth_token(token)
{
    user_agent = tr("mnlib/").append(MNLIB_VERSION);
}

dAmnSession::~dAmnSession()
{
}

const QString& dAmnSession::getUserName() const
{
    return this->user_name;
}

dAmnSession::State dAmnSession::getState() const
{
    return this->state;
}

QHash<QString, dAmnUser*>& dAmnSession::getUsers()
{
    return this->users;
}

dAmnUser* dAmnSession::addUser(const QString& name,
                               int usericon,
                               const QChar& symbol,
                               const QString &realname,
                               const QString &type_name)
{
    dAmnUser* user = this->users[name];

    if(!user)
    {
        user = new dAmnUser(this, name, symbol, usericon, realname, type_name);
        this->users[name] = user;
    }

    return user;
}

void dAmnSession::cleanupUser(const QString& name)
{
    dAmnUser* user = this->users[name];
    if(!user)
    {
        MNLIB_WARN("Can't cleanup user %s we don't know about.", qPrintable(name));
        return;
    }

    if(user->getChatrooms().isEmpty())
    {
        this->users.remove(name);
        delete user;
    }
}

bool dAmnSession::isMe(const QString& name)
{
    if(name == this->user_name)
        return true;

    return false;
}

void dAmnSession::connectToHost()
{
    if(this->state == offline)
    {
        connect(&this->socket, SIGNAL(connected()),
                this, SLOT(client()));
        this->socket.connectToHost(QHostAddress("chat.deviantart.com"), 3900, QIODevice::ReadWrite);

        this->login();
    }
    else
    {
        MNLIB_WARN("Attempted to connect an already connected session.");
    }
}

void dAmnSession::handlePacket(dAmnPacket* packet)
{
    switch(packet->command())
    {
    case dAmnPacket::dAmnServer:
        this->handleHandshake(packet);
        break;
    case dAmnPacket::login:
        this->handleLogin(packet);
        break;
    case dAmnPacket::join:
        this->handleJoin(packet);
        break;
    case dAmnPacket::part:
        this->handlePart(packet);
        break;
    case dAmnPacket::ping:
        this->handlePing();
        break;
    case dAmnPacket::property:
        this->handleProperty(packet);
        break;
    case dAmnPacket::whois:
        this->handleWhois(packet);

    default: qt_noop();
    }
}

void dAmnSession::send(dAmnPacket& packet)
{
    this->socket.write(packet.toByteArray());
}

void dAmnSession::login()
{
    dAmnPacket handshakePacket (this, "dAmnClient", DAMN_VERSION);
    handshakePacket["agent"] = this->user_agent;

    this->send(handshakePacket);

    this->state = logging_in;
}

void dAmnSession::join(const dAmnChatroomIdentifier& id)
{
    this->join(id.toString(), id.type);
}
void dAmnSession::join(const QString& name, dAmnChatroom::Type type)
{
    QString parsedname = name;

    if(type == dAmnChatroom::chat && parsedname[0] == '#')
        parsedname.remove(0, 1);

    switch(type)
    {
    case dAmnChatroom::chat:
        parsedname = QString("chat:%1").arg(parsedname);
        break;
    case dAmnChatroom::pchat:
        parsedname = QString("pchat:%1").arg(parsedname);
        break;
    }

    dAmnPacket packet (this, "join", parsedname);

    this->send(packet);
}

void dAmnSession::part(const dAmnChatroomIdentifier& id)
{
    this->part(id.toString(), id.type);
}
void dAmnSession::part(const QString& name, dAmnChatroom::Type type)
{
    QString parsedname = name;

    if(type == dAmnChatroom::chat && parsedname[0] == '#')
        parsedname.remove(0, 1);

    switch(type)
    {
    case dAmnChatroom::chat:
        parsedname = QString("chat:%1").arg(parsedname);
        break;
    case dAmnChatroom::pchat:
        parsedname = QString("pchat:%1").arg(parsedname);
        break;
    }

    dAmnPacket packet (this, "part", parsedname);

    this->send(packet);
}

void dAmnSession::kill(const QString& username, const QString& reason)
{
    dAmnPacket packet (this, "kill", QString("login:%1").arg(username), reason);
    this->send(packet);
}

void dAmnSession::pong()
{
    dAmnPacket packet (this, "pong");
    this->send(packet);
}

void dAmnSession::quit()
{
    dAmnPacket packet (this, "quit");
    this->send(packet);
}

void dAmnSession::handleHandshake(dAmnPacket* packet)
{
    HandshakeEvent* event = new HandshakeEvent(this, packet);

    if(event->matches())
    {
        this->sendCredentials();
    }
    else
    {
        MNLIB_CRIT("Protocol version mismatch. Aborting connection.");
    }

    emit handshake(event);
}

void dAmnSession::sendCredentials()
{
    dAmnPacket loginPacket (this, "login", this->user_name);
    loginPacket["pk"] = this->auth_token;

    this->send(loginPacket);
}

void dAmnSession::handleLogin(dAmnPacket* packet)
{
    LoginEvent* event = new LoginEvent(this, packet);

    if(event->getEvent() == LoginEvent::ok)
    {
        this->user_name = event->getUserName();
        this->symbol    = event->getSymbol();
        this->real_name = event->getRealName();
        this->type_name = event->getTypeName();
        this->gpc       = event->getGpc();

        this->state     = online;
    }
    else
    {
        this->state     = offline;
        this->socket.disconnectFromHost();
    }

    emit loggedIn(event);
}

void dAmnSession::handleJoin(dAmnPacket* packet)
{
    JoinedEvent* event = new JoinedEvent(this, packet);

    if(event->getEvent() == JoinedEvent::ok)
    {
        if(this->chatrooms.contains(event->getChatroom().toIdString()))
        {
            MNLIB_WARN("Joined a chatroom which we already joined. Aborting.");
        }
        else
        {
            this->chatrooms[event->getChatroom().toIdString()]
                    = new dAmnChatroom(this, event->getChatroom());
        }
    }

    emit joined(event);
}

void dAmnSession::handlePart(dAmnPacket* packet)
{
    PartedEvent* event = new PartedEvent(this, packet);

    if(event->getEvent() == PartedEvent::ok)
    {
        QString id = event->getChatroom().toIdString();
        delete this->chatrooms[id];
        this->chatrooms.remove(id);
    }

    emit parted(event);
}

void dAmnSession::handlePing()
{
    this->pong();

    emit ping();
}

void dAmnSession::handleProperty(dAmnPacket* packet)
{
    PropertyEvent* event = new PropertyEvent(this, packet);
    QString idstring = event->getChatroom().toIdString();

    if(!this->chatrooms.contains(idstring))
    {
        MNLIB_WARN("Got property %s of chatroom %s that we haven't joined.",
                   qPrintable(event->getPropertyString()),
                   qPrintable(event->getChatroom().toString()));
        return;
    }

    dAmnChatroom* chatroom = this->chatrooms[idstring];

    switch(event->getProperty())
    {
    case PropertyEvent::topic:
        chatroom->setTopic(event->getValue());
        break;
    case PropertyEvent::title:
        chatroom->setTitle(event->getValue());
        break;
    case PropertyEvent::privclasses:
        chatroom->updatePrivclasses(event->getValue());
        break;
    case PropertyEvent::members:
        chatroom->processMembers(event->getValue());
        break;

    case PropertyEvent::unknown:
    default:
        MNLIB_WARN("Got unknown property %s for chatroom %s.",
                   qPrintable(event->getPropertyString()),
                   qPrintable(event->getChatroom().toString()));
    }
}

void dAmnSession::handleWhois(dAmnPacket* packet)
{
    WhoisEvent* event = new WhoisEvent(this, packet);

    emit gotWhois(event);
}
