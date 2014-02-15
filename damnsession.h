/*
    This file is part of
    amnlib - A C++ library for deviantART Message Network
    Copyright © 2011 Carl Tessier <http://drfrankenstein90.deviantart.com/>

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

#ifndef DAMNSESSION_H
#define DAMNSESSION_H

#include "mnlib_global.h"

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QByteArray>
#include <QSslError>
#include <QHash>

#include "damnchatroom.h"
#include "evtfwd.h"
#include "damnuser.h"
#include "damnpacketdevice.h"

class QNetworkReply;
template <typename T> class QList;

class dAmnPacket;

class MNLIBSHARED_EXPORT dAmnSession : public QObject
{
    Q_OBJECT

    QTcpSocket _socket;
    dAmnPacketDevice _packetdevice;

    QString _useragent, _username, _realname, _typename, _gpc;
    QByteArray _authtoken;
    QChar _symbol;

    QHash<QString, dAmnChatroom*> _chatrooms;
    QHash<QString, dAmnUser*> _users;

public:
    enum State
    {
        offline, connecting, connected, logging_in, online
    };

private:
    State _state;

private slots:
    void handlePacket(dAmnPacket& packet);
    void socketStateChange(QAbstractSocket::SocketState socketState);

public:
    dAmnSession(const QString& username, const QByteArray &token, QObject* parent);
    virtual ~dAmnSession();

    const QString& userName() const;
    State state() const;
    QHash<QString, dAmnUser*>& users();
    dAmnUser* addUser(const QString& name,
                      int usericon,
                      const QChar& symbol,
                      const QString& realname,
                      const QString& type_name,
                      const QString& gpc);
    dAmnUser* addUser(const QString& name, const QString& props);
    void cleanupUser(const QString& name);

    bool isMe(const QString& name);

    void connectToHost();
    void send(dAmnPacket& packet);

    void login();

    void join(const QString& name, dAmnChatroom::Type type = dAmnChatroom::chat);
    void join(const dAmnChatroomIdentifier& id);
    void part(const QString& name, dAmnChatroom::Type type = dAmnChatroom::chat);
    void part(const dAmnChatroomIdentifier& id);

    void kill(const QString& username, const QString& reason = QString());

    void pong();

    QString errorString() const;

    void quit();

signals:
    // Incoming packet signals
    void handshake(const HandshakeEvent& event);
    void loggedIn(const LoginEvent& event);

    void joined(const JoinedEvent& event);
    void parted(const PartedEvent& event);

    void ping();

    void gotProperty(const PropertyEvent& event);
    void gotWhois(const WhoisEvent& event);

    void message(const MsgEvent& event);
    void action(const ActionEvent& event);
    void kicked(const KickedEvent& event);
    void disconnected(const DisconnectEvent& event);

    void join(const JoinEvent& event);
    void part(const PartEvent& event);
    void kick(const KickEvent& event);

    void privChg(const PrivchgEvent& event);
    void privUpdate(const PrivUpdateEvent& event);
    void privMove(const PrivMoveEvent& event);
    void privRemove(const PrivRemoveEvent& event);

    void privShow(const PrivShowEvent& event);
    void privUsers(const PrivUsersEvent& event);

    void sendError(const SendError& error);
    void kickError(const KickError& error);
    void getError(const GetError& error);
    void setError(const SetError& error);
    void killError(const KillError& error);

    void socketError(QAbstractSocket::SocketError socketError);

    void stateChange(dAmnSession::State state);

private:
    void sendCredentials();

    void setState(State state);

    void handleHandshake(dAmnPacket& packet);
    void handleLogin(dAmnPacket& packet);

    void handleJoin(dAmnPacket& packet);
    void handlePart(dAmnPacket& packet);
    void handleKick(dAmnPacket& packet);
    void handleDisconnect(dAmnPacket& packet);

    void handlePing();

    void handleProperty(dAmnPacket& packet);
    void handleWhois(dAmnPacket& packet);

    void handleRecv(dAmnPacket& packet);

    void handleMsg(dAmnPacket& packet, dAmnChatroom* room);
    void handleAction(dAmnPacket& packet, dAmnChatroom* room);

    void handlePeerJoin(dAmnPacket& packet, dAmnChatroom* room);
    void handlePeerPart(dAmnPacket& packet, dAmnChatroom* room);
    void handlePeerKick(dAmnPacket& packet, dAmnChatroom* room);

    void handlePrivchg(dAmnPacket& packet, dAmnChatroom* room);
    void handlePrivUpdate(dAmnPacket& packet, dAmnChatroom* room);
    void handlePrivMove(dAmnPacket& packet, dAmnChatroom* room);
    void handlePrivRemove(dAmnPacket& packet, dAmnChatroom* room);

    void handlePrivShow(dAmnPacket& packet, dAmnChatroom* room);
    void handlePrivUsers(dAmnPacket& packet, dAmnChatroom* room);

    void handleSendError(dAmnPacket& packet);
    void handleKickError(dAmnPacket& packet);
    void handleGetError(dAmnPacket& packet);
    void handleSetError(dAmnPacket& packet);
    void handleKillError(dAmnPacket& packet);
};

#endif // DAMNSESSION_H
