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

dAmnSession::dAmnSession()
    : state(offline)
{
    user_agent = tr("mnlib/").append(MNLIB_VERSION);
    auth_token.reserve(32);
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
            this, SLOT(gotAuthToken(QNetworkReply*)));
    connect(http, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
            this, SLOT(handleSslErrors(QNetworkReply*,QList<QSslError>)));

    QString ident_str = tr("ref=https://www.deviantart.com/users/login&username=%1&password=%2&reusetoken=%3")
                .arg(username, password, QString().setNum(reusetoken));

    QNetworkCookie cookie (qba("skipintro"), qba("1"));
    QList<QNetworkCookie> cookies;
    cookies.push_back(cookie);

    QNetworkRequest request (QUrl("https://www.deviantart.com/users/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    request.setHeader(QNetworkRequest::ContentLengthHeader, ident_str.size());
    request.setHeader(QNetworkRequest::CookieHeader, QVariant::fromValue(cookies));
    request.setRawHeader(qba("User-Agent"), this->user_agent.toAscii());
    request.setRawHeader(qba("Accept"), qba("text/html"));

    http->post(request, ident_str.toAscii());

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

void dAmnSession::gotAuthToken(QNetworkReply* reply)
{
    QNetworkAccessManager* http = reply->manager();

    // The expected form of the cookie is a serialized PHP array, URL-encoded, that contains an item with
    // the key "authtoken" and a value that has 32 characters.

    QString cookiestr;
    QList<QNetworkCookie> cookies = http->cookieJar()->cookiesForUrl(QUrl("https://www.deviantart.com/users/login"));
    foreach(QNetworkCookie cookie, cookies)
    {
        if(cookie.name() == "userinfo")
        {   // URL-decode the recieved cookie.
            cookiestr = QUrl::fromPercentEncoding(cookie.value());
            break;
        }
    }

    int found = false;
    if(!cookiestr.isEmpty())
    {   // HACK: This may break if the cookie changes more than we expect it to.
        // The ideal solution would be a decent parser, but is tedious to implement.
        this->auth_token = cookiestr.mid((found = cookiestr.indexOf("authtoken\";s:32:")) + 17, 32).toAscii();

        MNLIB_DEBUG("Token accepted: %s", this->auth_token.data());
    }
    else
    {   // Got no cookie with the name "userinfo".
        // TODO: Handle error
        MNLIB_WARN("NO COOKIE :(");
    }

    if(!found)
    {   // Cookie contains no authtoken?
        // TODO: Handle error
        MNLIB_WARN("CAN'T NOM COOKIE :(");
    }

    //delete http; // HACK: cause of a segfault further in the execution.
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

        connect(&this->socket, SIGNAL(readyRead()),
                this, SLOT(readPacket()));

        this->login();
    }
    else
    {
        MNLIB_WARN("Attempted to connect an already connected session.");
    }
}

void dAmnSession::send(dAmnPacket& packet)
{
    this->socket.write(packet.toByteArray());
}

void dAmnSession::readPacket()
{
    dAmnPacket packet (this, this->socket.readAll());
    switch(packet.command())
    {
    case dAmnPacket::dAmnServer:
        this->handleHandshake(packet);
        break;
    default: qt_noop();
    }
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
    dAmnPacket packet (this, "pong", QString());
    this->send(packet);
}

void dAmnSession::quit()
{
    dAmnPacket packet (this, "quit");
    this->send(packet);
}

void dAmnSession::handleHandshake(const dAmnPacket& packet)
{
    HandshakeEvent event (this, packet);

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
    dAmnPacket loginPacket (this, "login", this->user_name);
    loginPacket["pk"] = this->auth_token;

    this->send(loginPacket);
}

void dAmnSession::handleLogin(const dAmnPacket& packet)
{
    LoginEvent event (this, packet);

    if(event.getEvent() == LoginEvent::ok)
    {
        this->user_name = event.getUserName();
        this->symbol    = event.getSymbol();
        this->real_name = event.getRealName();
        this->type_name = event.getTypeName();
        this->gpc       = event.getGpc();

        this->state     = online;
    }
    else
    {
        this->state     = offline;
        this->socket.disconnectFromHost();
    }

    emit loggedIn(event);
}

void dAmnSession::handleJoin(const dAmnPacket& packet)
{
    JoinedEvent event (this, packet);

    if(event.getEvent() == JoinedEvent::ok)
    {
        if(this->chatrooms.contains(event.getChatroom().toIdString()))
        {
            MNLIB_WARN("Joined a chatroom which we already joined. Aborting.");
        }
        else
        {
            this->chatrooms[event.getChatroom().toIdString()]
                    = new dAmnChatroom(this, event.getChatroom());
        }
    }

    emit joined(event);
}

void dAmnSession::handlePart(const dAmnPacket& packet)
{
    PartedEvent event (this, packet);

    if(event.getEvent() == PartedEvent::ok)
    {
        this->chatrooms.remove(event.getChatroom().toIdString());
    }

    emit parted(event);
}

void dAmnSession::handlePing()
{
    this->pong();

    emit ping();
}

void dAmnSession::handleProperty(const dAmnPacket& packet)
{
    PropertyEvent event (this, packet);
    QString idstring = event.getChatroom().toIdString();

    if(!this->chatrooms.contains(idstring))
    {
        MNLIB_WARN("Got a property of chatroom %s that haven't joined.",
                   qPrintable(event.getChatroom().toString()));
        return;
    }

    switch(event.getProperty())
    {
    case PropertyEvent::topic:
        this->chatrooms[idstring]->setTopic(event.getValue());
    case PropertyEvent::title:
        this->chatrooms[idstring]->setTitle(event.getValue());
    case PropertyEvent::privclasses:
        ; // TODO
    }
}
