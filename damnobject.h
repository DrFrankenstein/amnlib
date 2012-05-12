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

#ifndef DAMNOBJECT_H
#define DAMNOBJECT_H

#include <QObject>
#include "mnlib_global.h"

class dAmnSession;

class MNLIBSHARED_EXPORT dAmnObject : public QObject
{
    Q_OBJECT

public:
    dAmnObject(dAmnSession* parent);

    dAmnSession* session() const;
};

#endif // DAMNOBJECT_H
