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
#include "damnpacket.h"
#include "damnchatroom.h"

#include <QString>
#include <QStringList>
#include <QTextStream>

dAmnPrivClass::dAmnPrivClass(dAmnChatroom* parent)
    : QObject(parent)
{
}

dAmnPrivClass::dAmnPrivClass(dAmnChatroom* parent, const QString& command)
    : QObject(parent)
{
    int pos = command.indexOf(' ');
    //this->setObjectName(command.mid(0, pos));
    this->name = command.mid(0, pos);

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
            QPair<QString, QString> pair = dAmnPacket::parsePair(command);
            priv = pair.first;
            value = pair.second.toInt();
        }

        switch(getPriv(priv))
        {
        case join:      this->joinpriv = value;        break;
        case title:     this->titlepriv = value;       break;
        case kick:      this->kickpriv = value;        break;
        case msg:       this->msgpriv = value;         break;
        case shownotice:this->shownoticepriv = value;  break;
        case admin:     this->adminpriv = value;       break;
        case images:    this->imagespriv = value;      break;
        case smilies:   this->smiliespriv = value;     break;
        case emoticons: this->emoticonspriv = value;   break;
        case thumbs:    this->thumbspriv = value;      break;
        case avatars:   this->avatarspriv = value;     break;
        case websites:  this->websitespriv = value;    break;
        case objects:   this->objectspriv = value;     break;
        default: qt_noop();
        }
    }
}

QHash<QString, dAmnPrivClass::KnownPrivs> dAmnPrivClass::kpriv_map;

void dAmnPrivClass::initKPriv()
{
#   define KPRIV(name) kpriv_map[#name] = name;
    KPRIV(join) KPRIV(title) KPRIV(kick) KPRIV(msg) KPRIV(shownotice) KPRIV(admin)
    KPRIV(images) KPRIV(smilies) KPRIV(emoticons) KPRIV(thumbs) KPRIV(avatars) KPRIV(websites) KPRIV(objects)
#   undef KPRIV
}

dAmnPrivClass::KnownPrivs dAmnPrivClass::getPriv(QString privname)
{
    if(kpriv_map.isEmpty())
        initKPriv();

    return kpriv_map[privname];
}

const QString& dAmnPrivClass::getName() const
{
    return this->name;
}

int dAmnPrivClass::getOrder() const
{
    return this->order;
}

bool dAmnPrivClass::getJoinPriv() const { return this->joinpriv; }
bool dAmnPrivClass::getTitlePriv() const { return this->titlepriv; }
bool dAmnPrivClass::getKickPriv() const { return this->kickpriv; }
bool dAmnPrivClass::getMsgPriv() const { return this->msgpriv; }
bool dAmnPrivClass::getSnowNoticePriv() const { return this->shownoticepriv; }
bool dAmnPrivClass::getAdminPriv() const { return this->adminpriv; }

int dAmnPrivClass::getImagesPriv() const { return this->imagespriv; }
int dAmnPrivClass::getSmiliesPriv() const { return this->smiliespriv; }
int dAmnPrivClass::getEmoticonsPriv() const { return this->emoticonspriv; }
int dAmnPrivClass::getThumbsPriv() const { return this->thumbspriv; }
int dAmnPrivClass::getAvatarsPriv() const { return this->avatarspriv; }
int dAmnPrivClass::getWebsitesPriv() const { return this->websitespriv; }
int dAmnPrivClass::getObjectsPriv() const { return this->objectspriv; }
