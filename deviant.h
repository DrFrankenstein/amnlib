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

#include <QObject>
#include <QString>
#include <QChar>

class QUrl;

class Deviant : public QObject
{
    Q_OBJECT

    QString name, realname, type;
    QChar symbol;
    int usericon;

public:
    enum IconType
    {
        gif, jpg, png
    };

    Deviant(const QString& name, const QChar& symbol = QChar::Null,
            int usericon = 0, const QString& realname = QString(), const QString& type = QString());

    const QString& getName() const;
    const QString& getRealName() const;
    const QString& getTypeName() const;
    const QChar& getSymbol() const;
    IconType getIconType() const;

    QUrl getIconUrl() const;
    QUrl getProfileUrl() const;
};

#endif // DEVIANT_H
