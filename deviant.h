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

#ifndef DEVIANT_H
#define DEVIANT_H

#include "mnlib_global.h"

#include <QObject>
#include <QString>
#include <QChar>

class QUrl;

class MNLIBSHARED_EXPORT Deviant : public QObject
{
    Q_OBJECT

    QString _name, _realname, _type;
    QChar _symbol;
    int _usericon;

public:
    enum IconType
    {
        gif, jpg, png
    };

    Deviant(QObject* parent, const QString& name, const QChar& symbol = QChar::Null,
            int usericon = 0, const QString& realname = QString(), const QString& type = QString());

    const QString& name() const;
    const QString& realName() const;
    const QString& typeName() const;
    const QChar& symbol() const;
    IconType iconType() const;

    QUrl iconUrl() const;
    QUrl profileUrl() const;

protected:
    void setName(const QString& name);
    void setRealname(const QString& realname);
    void setTypeName(const QString& type);
    void setSymbol(const QChar& symbol);
    void setUsericon(int usericon);
};

#endif // DEVIANT_H
