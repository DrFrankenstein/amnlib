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

#include "deviant.h"

#include <QString>
#include <QChar>
#include <QUrl>

Deviant::Deviant(const QString& name, const QChar& symbol, int usericon, const QString& realname, const QString& type)
    : name(name), realname(realname), type(type), symbol(symbol), usericon(usericon)
{
}

const QString& Deviant::getName() const
{
    return this->name;
}

const QString& Deviant::getRealName() const
{
    return this->realname;
}

const QString& Deviant::getTypeName() const
{
    return this->type;
}

const QChar& Deviant::getSymbol() const
{
    return this->symbol;
}

Deviant::IconType Deviant::getIconType() const
{
    switch(this->usericon & 3)
    {
    case 0:
    case 1: return gif;
    case 2: return jpg;
    case 3: return png;
    }

    return gif; // keep compiler happy.
}

QUrl Deviant::getIconUrl() const
{
    QString urlstr = "http://a.deviantart.com/avatars/",
            name = this->name.toLower();

    if(this->usericon)
    {
        QChar chunk = name[0];
        urlstr += (chunk.isLetterOrNumber()? '_' : chunk) + '/';
        chunk = name[1];
        urlstr += (chunk.isLetterOrNumber()? '_' : chunk) + '/';
        urlstr += name;
    }
    else
    {
        urlstr += "default";
    }

    switch(this->getIconType())
    {
    case gif: urlstr += ".gif"; break;
    case jpg: urlstr += ".jpg"; break;
    case png: urlstr += ".png"; break;
    }

    return QUrl(urlstr);
}

QUrl Deviant::getProfileUrl() const
{
    return QUrl(tr("http://%1/").arg(this->name.toLower()));
}
