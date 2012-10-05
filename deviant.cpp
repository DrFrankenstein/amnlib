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

#include <QObject>
#include <QString>
#include <QChar>
#include <QUrl>

Deviant::Deviant(QObject* parent, const QString& name, const QChar& symbol, int usericon, const QString& realname, const QString& type)
    : QObject(parent), _name(name), _realname(realname), _type(type), _symbol(symbol), _usericon(usericon)
{
    this->setObjectName(this->_name);
}

const QString& Deviant::name() const
{
    return this->_name;
}

const QString& Deviant::realName() const
{
    return this->_realname;
}

const QString& Deviant::typeName() const
{
    return this->_type;
}

const QChar& Deviant::symbol() const
{
    return this->_symbol;
}

void Deviant::setName(const QString& name)
{
    this->_name = name;
}

void Deviant::setRealname(const QString& realname)
{
    this->_realname = realname;
}

void Deviant::setTypeName(const QString& type)
{
    this->_type = type;
}

void Deviant::setSymbol(const QChar& symbol)
{
    this->_symbol = symbol;
}

void Deviant::setUsericon(int usericon)
{
    this->_usericon = usericon;
}

Deviant::IconType Deviant::iconType() const
{
    switch(this->_usericon & 3)
    {
    case 0:
    case 1: return gif;
    case 2: return jpg;
    case 3: return png;
    }

    return gif; // keep compiler happy.
}

QUrl Deviant::iconUrl() const
{
    QString urlstr = "http://a.deviantart.com/avatars/",
            name = this->_name.toLower();

    if(this->_usericon)
    {
        QChar chunk = name[0];
        urlstr += (chunk.isLetterOrNumber()? chunk : '_') + '/';
        chunk = name[1];
        urlstr += (chunk.isLetterOrNumber()? chunk : '_') + '/';
        urlstr += name;
    }
    else
    {
        urlstr += "default";
    }

    switch(this->iconType())
    {
    case gif: urlstr += ".gif"; break;
    case jpg: urlstr += ".jpg"; break;
    case png: urlstr += ".png"; break;
    }

    return QUrl(urlstr);
}

QUrl Deviant::profileUrl() const
{
    return QUrl(QString("http://%1.deviantart.com/").arg(this->_name.toLower()));
}
