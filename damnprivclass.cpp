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

#include "damnprivclass.h"
#include "damnpacketparser.h"
#include "damnchatroom.h"

#include <QString>
#include <QStringList>
#include <QTextStream>

dAmnPrivClass::dAmnPrivClass(dAmnChatroom* parent)
    : QObject(parent)
{
}

dAmnPrivClass::dAmnPrivClass(dAmnChatroom* parent, const QString& name, uint order)
    : QObject(parent), _name(name), _order(order)
{
    this->setObjectName(this->_name);
}

dAmnPrivClass::dAmnPrivClass(dAmnChatroom* parent, const QString& command)
    : QObject(parent)
{
    int pos = command.indexOf(' ');
    //this->setObjectName(command.mid(0, pos));
    this->_name = command.mid(0, pos);
    this->setObjectName(this->_name);

    this->apply(command.mid(pos));
}

void dAmnPrivClass::apply(QString commands)
{
    QTextStream parser (&commands);

    while(!parser.atEnd())
    {
        QString command;
        parser >> command;

        QString priv;
        int value;

        switch(command[0].toLatin1())
        {
        case '+':
            priv = command.mid(1);
            value = 1;
        break;
        case '-':
            priv = command.mid(1);
            value = 0;
        break;

        default:
            QPair<QString, QString> pair = dAmnPacketParser::splitPair(command);
            priv = pair.first;
            value = pair.second.toInt();
        }

        switch(getPriv(priv))
        {
        case join:      this->_joinpriv = value;        break;
        case title:     this->_titlepriv = value;       break;
        case kick:      this->_kickpriv = value;        break;
        case msg:       this->_msgpriv = value;         break;
        case shownotice:this->_shownoticepriv = value;  break;
        case admin:     this->_adminpriv = value;       break;
        case images:    this->_imagespriv = value;      break;
        case smilies:   this->_smiliespriv = value;     break;
        case emoticons: this->_emoticonspriv = value;   break;
        case thumbs:    this->_thumbspriv = value;      break;
        case avatars:   this->_avatarspriv = value;     break;
        case websites:  this->_websitespriv = value;    break;
        case objects:   this->_objectspriv = value;     break;
        case order:     this->_order = value;           break;
        default: qt_noop();
        }
    }
}

QHash<QString, dAmnPrivClass::KnownPrivs> dAmnPrivClass::_kpriv_map;

void dAmnPrivClass::initKPriv()
{
#   define KPRIV(name) _kpriv_map[#name] = name;
    KPRIV(join) KPRIV(title) KPRIV(kick) KPRIV(msg) KPRIV(shownotice) KPRIV(admin)
    KPRIV(images) KPRIV(smilies) KPRIV(emoticons) KPRIV(thumbs) KPRIV(avatars) KPRIV(websites) KPRIV(objects)
    KPRIV(order)
#   undef KPRIV
}

dAmnPrivClass::KnownPrivs dAmnPrivClass::getPriv(QString privname)
{
    if(_kpriv_map.isEmpty())
        initKPriv();

    return _kpriv_map[privname];
}

const QString& dAmnPrivClass::name() const
{
    return this->_name;
}

uint dAmnPrivClass::orderValue() const
{
    return this->_order;
}

void dAmnPrivClass::setOrderValue(uint order)
{
    this->_order = order;
}

bool dAmnPrivClass::joinPriv() const { return this->_joinpriv; }
bool dAmnPrivClass::titlePriv() const { return this->_titlepriv; }
bool dAmnPrivClass::kickPriv() const { return this->_kickpriv; }
bool dAmnPrivClass::msgPriv() const { return this->_msgpriv; }
bool dAmnPrivClass::showNoticePriv() const { return this->_shownoticepriv; }
bool dAmnPrivClass::adminPriv() const { return this->_adminpriv; }

int dAmnPrivClass::imagesPriv() const { return this->_imagespriv; }
int dAmnPrivClass::smiliesPriv() const { return this->_smiliespriv; }
int dAmnPrivClass::emoticonsPriv() const { return this->_emoticonspriv; }
int dAmnPrivClass::thumbsPriv() const { return this->_thumbspriv; }
int dAmnPrivClass::avatarsPriv() const { return this->_avatarspriv; }
int dAmnPrivClass::websitesPriv() const { return this->_websitespriv; }
int dAmnPrivClass::objectsPriv() const { return this->_objectspriv; }

void dAmnPrivClass::addUser(dAmnUser* user)
{
    this->_users.insert(user);
}

void dAmnPrivClass::removeUser(dAmnUser* user)
{
    this->_users.remove(user);
}
