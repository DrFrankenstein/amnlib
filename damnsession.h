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

    QTcpSocket socket;
    dAmnPacketDevice packetdevice;

    QString user_agent, user_name, real_name, type_name, gpc;
    QByteArray auth_token;
    QChar symbol;

    QHash<QString, dAmnChatroom*> chatrooms;

public:
    enum State
    {
        unauthenticated, offline, logging_in, online
    };

private:
    State state;

private slots:
    void gotAuthToken(QNetworkReply*);
    void handleSslErrors(QNetworkReply* reply, QList<QSslError> errors);

    void handlePacket(dAmnPacket* packet);

public:
    dAmnSession();
    virtual ~dAmnSession();

    void authenticate(const QString& username, const QString& password, bool reusetoken = true);

    const QString& getUserName() const;
    State getState() const;

    bool isMe(const QString& name);

    void connectToHost();
    void send(dAmnPacket& packet);

    void join(const QString& name, dAmnChatroom::Type type = dAmnChatroom::chat);
    void join(const dAmnChatroomIdentifier& id);
    void part(const QString& name, dAmnChatroom::Type type = dAmnChatroom::chat);
    void part(const dAmnChatroomIdentifier& id);

    void kill(const QString& username, const QString& reason = QString());

    void pong();

    void quit();

signals:
    // Incoming packet signals
    void handshake(const HandshakeEvent* event);
    void loggedIn(const LoginEvent* event);

    void joined(const JoinedEvent* event);
    void parted(const PartedEvent* event);

    void ping();

    void gotProperty(const PropertyEvent* event);
    void gotWhois(const WhoisEvent* event);

    void gotMsg(const MsgEvent* event);
    void gotAction(const ActionEvent* event);

    void peerJoined(const JoinEvent* event);
    void peerParted(const PartEvent* event);
    void peerKicked(const KickEvent* event);

    void privchged(const PrivchgEvent* event);
    void privUpdated(const PrivUpdateEvent* event);
    void privMoved(const PrivMoveEvent* event);
    void privRemoved(const PrivRemoveEvent* event);

    void gotPrivShow(const PrivShowEvent* event);
    void gotPrivUsers(const PrivUsersEvent* event);

    void kicked(const KickedEvent* event);
    void disconnected(const DisconnectEvent* event);

    void sendError(const SendError* error);
    void kickError(const KickError* error);
    void getError(const GetError* error);
    void setError(const SetError* error);
    void killError(const KillError* error);

private:
    void login();
    void sendCredentials();

    void handleHandshake(dAmnPacket* packet);
    void handleLogin(dAmnPacket* packet);

    void handleJoin(dAmnPacket* packet);
    void handlePart(dAmnPacket* packet);

    void handlePing();

    void handleProperty(dAmnPacket* packet);
    void handleWhois(dAmnPacket* packet);

    void handleMsg(dAmnPacket* packet);
    void handleAction(dAmnPacket* packet);

    void handlePeerJoin(dAmnPacket* packet);
    void handlePeerPart(dAmnPacket* packet);
    void handlePeerKick(dAmnPacket* packet);

    void handlePrivchg(dAmnPacket* packet);
    void handlePrivUpdate(dAmnPacket* packet);
    void handlePrivMove(dAmnPacket* packet);
    void handlePrivRemove(dAmnPacket* packet);

    void handlePrivShow(dAmnPacket* packet);
    void handlePrivUsers(dAmnPacket* packet);

    void handkeKick(dAmnPacket* packet);
    void handleDisconnect(dAmnPacket* packet);

    void handleSendError(dAmnPacket* packet);
    void handleKickError(dAmnPacket* packet);
    void handleGetError(dAmnPacket* packet);
    void handleSetError(dAmnPacket* packetr);
    void handleKillError(dAmnPacket* packet);
};

#endif // DAMNSESSION_H
