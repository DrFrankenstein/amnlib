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

    QString _name;

    uint _order;

    bool _joinpriv, _titlepriv, _topicpriv, _kickpriv, _msgpriv, _shownoticepriv, _adminpriv;
    int _imagespriv, _smiliespriv, _emoticonspriv, _thumbspriv, _avatarspriv, _websitespriv, _objectspriv;

    QSet<dAmnUser*> _users;

public:
    enum KnownPrivs
    {
        unknown,
        join, title, topic, kick, msg, shownotice, admin,
        images, smilies, emoticons, thumbs, avatars, websites, objects, order
    };

private:
    static QHash<QString, KnownPrivs> _kpriv_map;
    static void initKPriv();
    static KnownPrivs getPriv(QString privname);

public:
    dAmnPrivClass(dAmnChatroom* parent);
    dAmnPrivClass(dAmnChatroom* parent, const QString& name, uint order);
    dAmnPrivClass(dAmnChatroom* parent, const QString& command);

    void apply(QString command);

    const QString& name() const;
    void setName(const QString& name);
    uint orderValue() const;
    void setOrderValue(uint order);
    const QSet<dAmnUser*>& users() const;

    bool joinPriv() const;
    bool titlePriv() const;
    bool topicPriv() const;
    bool kickPriv() const;
    bool msgPriv() const;
    bool showNoticePriv() const;
    bool adminPriv() const;

    int imagesPriv() const;
    int smiliesPriv() const;
    int emoticonsPriv() const;
    int thumbsPriv() const;
    int avatarsPriv() const;
    int websitesPriv() const;
    int objectsPriv() const;

    void addUser(dAmnUser* user);
    void removeUser(dAmnUser* user);
};

#endif // DAMNPRIVCLASS_H
