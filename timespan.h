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

#ifndef TIMESPAN_H
#define TIMESPAN_H

#include <qglobal.h>

class TimeSpan
{
    uint _seconds;

public:
    TimeSpan(uint seconds = 0);
    TimeSpan(uint minutes, uint seconds);
    TimeSpan(uint hours, uint minutes, uint seconds);
    TimeSpan(uint days, uint hours, uint minutes, uint seconds);

    uint getSeconds() const;
    uint getMinutes() const;
    uint getHours() const;
    uint getDays() const;

    uint getTotalSeconds() const;
    double getTotalMinutes() const;
    double getTotalHours() const;

    TimeSpan& operator +=(const TimeSpan& right);
    TimeSpan& operator -=(const TimeSpan& right);
    TimeSpan operator +(const TimeSpan& right) const;
    TimeSpan operator -(const TimeSpan& right) const;

    void addSeconds(int seconds);
    void addMinutes(int minutes);
    void addHours(int hours);
    void addDays(int days);

    void setTotalSeconds(uint seconds);
    void setTotalMinutes(uint minutes);
    void setTotalHours(uint hours);
    void setTotalDays(uint days);
};

#endif // TIMESPAN_H
