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

dAmnSession::dAmnSession(const QString& username, const QByteArray& token)
    : _state(offline), _packetdevice(this, this->_socket), _socket(this),
      _username(username), _authtoken(token)
{
    _useragent = tr("mnlib/").append(MNLIB_VERSION);

    connect(&this->_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SIGNAL(socketError(QAbstractSocket::SocketError)));
    connect(&this->_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(socketStateChange(QAbstractSocket::SocketState)));
    connect(&this->_packetdevice, SIGNAL(packetReady(dAmnPacket&)),
            this, SLOT(handlePacket(dAmnPacket&)));
}

dAmnSession::~dAmnSession()
{
}

const QString& dAmnSession::userName() const
{
    return this->_username;
}

dAmnSession::State dAmnSession::state() const
{
    return this->_state;
}

QHash<QString, dAmnUser*>& dAmnSession::users()
{
    return this->_users;
}

dAmnUser* dAmnSession::addUser(const QString& name,
                               int usericon,
                               const QChar& symbol,
                               const QString &realname,
                               const QString &type_name)
{
    dAmnUser* user = this->_users.value(name);

    if(!user)
    {
        user = new dAmnUser(this, name, symbol, usericon, realname, type_name);
        this->_users.insert(name, user);
    }

    return user;
}

void dAmnSession::cleanupUser(const QString& name)
{
    dAmnUser* user = this->_users.value(name);
    if(!user)
    {
        MNLIB_WARN("Can't cleanup user %s we don't know about.", qPrintable(name));
        return;
    }

    if(user->chatrooms().isEmpty())
    {
        this->_users.remove(name);
        delete user;
    }
}

bool dAmnSession::isMe(const QString& name)
{
    if(name == this->_username)
        return true;

    return false;
}

void dAmnSession::connectToHost()
{
    if(this->_state == offline)
    {
        //connect(&this->socket, SIGNAL(connected()),
        //        this, SLOT(login()));
        this->_socket.connectToHost("chat.deviantart.com", 3900, QIODevice::ReadWrite);
        //this->setState(connecting);
    }
    else
    {
        MNLIB_WARN("Attempted to connect an already connected session.");
    }
}

void dAmnSession::handlePacket(dAmnPacket& packet)
{
    switch(packet.command())
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
    case dAmnPacket::recv:


    default:
        MNLIB_DEBUG("Unhandled packet: %s", packet.toByteArray().data());
        qt_noop();
    }
}

void dAmnSession::socketStateChange(QAbstractSocket::SocketState socketState)
{
    switch(socketState)
    {
    case QAbstractSocket::ConnectingState:
        this->setState(connecting);
        break;
    case QAbstractSocket::ConnectedState:
        this->setState(connected);
        this->login();
        break;
    case QAbstractSocket::UnconnectedState:
        this->setState(offline);
    }
}

void dAmnSession::send(dAmnPacket& packet)
{
    QByteArray data = packet.toByteArray();
    //MNLIB_DEBUG("%s", data.data());
    this->_socket.write(data);
}

void dAmnSession::login()
{
    MNLIB_DEBUG("Greeting server as %s", qPrintable(this->_useragent));
    dAmnPacket handshakePacket (this, "dAmnClient", DAMN_VERSION);
    handshakePacket.args().insert("agent", this->_useragent);

    this->send(handshakePacket);

    this->setState(logging_in);
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

void dAmnSession::handleHandshake(dAmnPacket& packet)
{
    HandshakeEvent event (this, packet);
    MNLIB_DEBUG("Handshake recieved, version %s", qPrintable(event.version()));

    if(event.matches())
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
    MNLIB_DEBUG("Logging in as %s", qPrintable(this->_username));
    dAmnPacket loginPacket (this, "login", this->_username);
    loginPacket.args().insert("pk", this->_authtoken);

    this->send(loginPacket);
}

void dAmnSession::setState(State state)
{
    this->_state = state;
    emit stateChange(state);
}

void dAmnSession::handleLogin(dAmnPacket& packet)
{
    LoginEvent event (this, packet);

    if(event.eventCode() == LoginEvent::ok)
    {
        MNLIB_DEBUG("Login OK.");
        this->_username = event.userName();
        this->_symbol    = event.symbol();
        this->_realname = event.realName();
        this->_typename = event.typeName();
        this->_gpc       = event.gpc();

        this->setState(online);
    }
    else
    {
        MNLIB_DEBUG("Login denied: %s", qPrintable(event.eventString()));
        this->setState(offline);
        this->_socket.disconnectFromHost();
    }

    emit loggedIn(event);
}

void dAmnSession::handleJoin(dAmnPacket& packet)
{
    JoinedEvent event (this, packet);

    if(event.eventCode() == JoinedEvent::ok)
    {
        MNLIB_DEBUG("Joined %s", qPrintable(event.chatroom().toIdString()));
        if(this->_chatrooms.contains(event.chatroom().toIdString()))
        {
            MNLIB_WARN("Joined a chatroom which we already joined. Aborting.");
        }
        else
        {
            this->_chatrooms[event.chatroom().toIdString()]
                    = new dAmnChatroom(this, event.chatroom());
        }
    }
    else
    {
        MNLIB_DEBUG("Could not join %s: %s",
                    qPrintable(event.chatroom().toIdString()),qPrintable(event.eventString()));
    }

    emit joined(event);
}

void dAmnSession::handlePart(dAmnPacket& packet)
{
    PartedEvent event (this, packet);

    if(event.eventCode() == PartedEvent::ok)
    {
        MNLIB_DEBUG("Parted from %s", qPrintable(event.chatroom().toIdString()));
        QString id = event.chatroom().toIdString();
        delete this->_chatrooms[id];
        this->_chatrooms.remove(id);
    }
    else
    {
        MNLIB_DEBUG("Could not part from %s: %s",
                    qPrintable(event.chatroom().toIdString()), qPrintable(event.eventString()));
    }

    emit parted(event);
}

void dAmnSession::handlePing()
{
    MNLIB_DEBUG("Ping? Pong!");
    this->pong();

    emit ping();
}

void dAmnSession::handleProperty(dAmnPacket& packet)
{
    PropertyEvent event (this, packet);
    QString idstring = event.chatroom().toIdString();

    if(!this->_chatrooms.contains(idstring))
    {
        MNLIB_WARN("Got property %s of chatroom %s that we haven't joined.",
                   qPrintable(event.propertyString()),
                   qPrintable(event.chatroom().toString()));
        return;
    }

    dAmnChatroom* chatroom = this->_chatrooms[idstring];

    switch(event.propertyCode())
    {
    case PropertyEvent::topic:
        MNLIB_DEBUG("Topic of %s is %s", qPrintable(event.chatroom().toIdString()), qPrintable(event.value()));
        chatroom->setTopic(event.value());
        break;
    case PropertyEvent::title:
        MNLIB_DEBUG("Title of %s is %s", qPrintable(event.chatroom().toIdString()), qPrintable(event.value()));
        chatroom->setTitle(event.value());
        break;
    case PropertyEvent::privclasses:
        MNLIB_DEBUG("Got privclasses for %s", qPrintable(event.chatroom().toIdString()));
        chatroom->updatePrivclasses(event.value());
        break;
    case PropertyEvent::members:
        MNLIB_DEBUG("Got members for %s", qPrintable(event.chatroom().toIdString()));
        chatroom->processMembers(event.value());
        break;

    case PropertyEvent::unknown:
    default:
        MNLIB_WARN("Got unknown property %s for chatroom %s.",
                   qPrintable(event.propertyString()),
                   qPrintable(event.chatroom().toString()));
    }
}

void dAmnSession::handleWhois(dAmnPacket& packet)
{
    WhoisEvent event (this, packet);

    emit gotWhois(event);
}

