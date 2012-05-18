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

#ifndef DAMNPRIVCLASS_H
#define DAMNPRIVCLASS_H

#include <QObject>
#include <QHash>
#include <QSet>

#include "mnlib_global.h"

class dAmnChatroom;
class dAmnUser;

class MNLIBSHARED_EXPORT dAmnPrivClass : public QObject
{
    Q_OBJECT

    QString name;

    uint order;

    bool joinpriv, titlepriv, topicpriv, kickpriv, msgpriv, shownoticepriv, adminpriv;
    int imagespriv, smiliespriv, emoticonspriv, thumbspriv, avatarspriv, websitespriv, objectspriv;

    QSet<dAmnUser*> users;

public:
    enum KnownPrivs
    {
        unknown,
        join, title, topic, kick, msg, shownotice, admin,
        images, smilies, emoticons, thumbs, avatars, websites, objects
    };

private:
    static QHash<QString, KnownPrivs> kpriv_map;
    static void initKPriv();
    static KnownPrivs getPriv(QString privname);

public:
    dAmnPrivClass(dAmnChatroom* parent);
    dAmnPrivClass(dAmnChatroom* parent, const QString& name, uint order);
    dAmnPrivClass(dAmnChatroom* parent, const QString& command);

    void apply(QString command);

    const QString& getName() const;
    int getOrder() const;

    bool getJoinPriv() const;
    bool getTitlePriv() const;
    bool getTopicPriv() const;
    bool getKickPriv() const;
    bool getMsgPriv() const;
    bool getSnowNoticePriv() const;
    bool getAdminPriv() const;

    int getImagesPriv() const;
    int getSmiliesPriv() const;
    int getEmoticonsPriv() const;
    int getThumbsPriv() const;
    int getAvatarsPriv() const;
    int getWebsitesPriv() const;
    int getObjectsPriv() const;

    void addUser(dAmnUser* user);
    void removeUser(dAmnUser* user);
};

#endif // DAMNPRIVCLASS_H
