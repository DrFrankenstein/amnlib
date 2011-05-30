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

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QSslError>
#include <QHostAddress>
#include <QRegExp>

dAmnSession::dAmnSession()
    : state(unauthenticated), packetdevice(this, this->socket)
{
    user_agent = tr("mnlib/").append(MNLIB_VERSION);
    auth_token.reserve(32);

    connect(&this->packetdevice, SIGNAL(packetReady(dAmnPacket*)),
            this, SLOT(handlePacket(dAmnPacket*)));
}

dAmnSession::~dAmnSession()
{
}

void dAmnSession::authenticate(const QString& username, const QString& password, bool reusetoken)
{
    typedef QByteArray qba;

    this->user_name = username;

    QNetworkAccessManager* http = new QNetworkAccessManager(this);

    connect(http, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(gotAuthCookie(QNetworkReply*)));
    connect(http, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
            this, SLOT(handleSslErrors(QNetworkReply*,QList<QSslError>)));

    QUrl loginform;
    loginform.addQueryItem("username", username);
    loginform.addQueryItem("password", password);
    loginform.addQueryItem("remember_me", QString().setNum(reusetoken));
    QByteArray logindata = loginform.toEncoded();

    QNetworkRequest request (QUrl("https://www.deviantart.com/users/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    request.setHeader(QNetworkRequest::ContentLengthHeader, logindata.size());
    request.setRawHeader(qba("User-Agent"), this->user_agent.toAscii());
    request.setRawHeader(qba("Accept"), qba("text/html"));

    http->post(request, logindata);

    MNLIB_DEBUG("Requesting authentication for %s", qPrintable(username));
}

void dAmnSession::handleSslErrors(QNetworkReply* reply, QList<QSslError> errors)
{   MNLIB_STUB

#if MNLIB_DEBUG_BUILD
    foreach(QSslError error, errors)
    {
        MNLIB_DEBUG("%s", qPrintable(error.errorString()));
    }
#else
    Q_UNUSED(errors)
#endif

    reply->ignoreSslErrors();
}

void dAmnSession::gotAuthCookie(QNetworkReply* reply)
{
    QNetworkAccessManager* http = reply->manager();
    disconnect(http, SIGNAL(finished(QNetworkReply*)),
               this, SLOT(gotAuthCookie(QNetworkReply*)));

    bool authenticated = false;
    QList<QNetworkCookie> cookies = http->cookieJar()
            ->cookiesForUrl(QUrl("https://www.deviantart.com/"));
    foreach(QNetworkCookie cookie, cookies)
        if(cookie.name() == "auth")
            authenticated = true;

    if(authenticated)
    {
        connect(http, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(gotAuthData(QNetworkReply*)));

        QNetworkRequest request (QUrl("http://chat.deviantart.com/chat/devart"));
        http->get(request);

        MNLIB_DEBUG("Authentication received, getting dAmn login token.");
    }
    else
    {
        MNLIB_CRIT("Authentication failed for %s", qPrintable(this->user_name));
        this->state = offline;
        http->deleteLater();
    }

    reply->deleteLater();
}

void dAmnSession::gotAuthData(QNetworkReply* reply)
{
    QString data (reply->readAll());

    // Ugly page scraping ahead...
    QRegExp rx ("dAmn_Login\\s*\\(\\s*\".*\"\\s*,\\s*\"([0-9a-f]{32})\"");
    int pos = rx.indexIn(data);
    if(pos >= 0)
    {
        this->auth_token = rx.cap(1).toAscii();
        this->state = logging_in;
        MNLIB_DEBUG("Token accepted: %s", this->auth_token.data());
    }
    else
    {
        MNLIB_CRIT("Couldn't scrape login token!");
        this->state = offline;
    }

    reply->manager()->deleteLater();
    reply->deleteLater();
}

const QString& dAmnSession::getUserName() const
{
    return this->user_name;
}

dAmnSession::State dAmnSession::getState() const
{
    return this->state;
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
        this->chatrooms.remove(event->getChatroom().toIdString());
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
    case PropertyEvent::title:
        chatroom->setTitle(event->getValue());
    case PropertyEvent::privclasses:
        chatroom->updatePrivclasses(event->getValue());
    case PropertyEvent::members:
        // NYI

    default:
        MNLIB_WARN("Got unknown property %s for chatroom %s.",
                   qPrintable(event->getPropertyString()),
                   qPrintable(event->getChatroom().toString()));
    }
}
