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

#include <qglobal.h>

#include "timespan.h"

TimeSpan::TimeSpan(uint seconds)
    : _seconds(seconds)
{
}
TimeSpan::TimeSpan(uint minutes, uint seconds)
    : _seconds(minutes * 60 + seconds)
{
}
TimeSpan::TimeSpan(uint hours, uint minutes, uint seconds)
    : _seconds(hours * 3600 + minutes * 60 + seconds)
{
}
TimeSpan::TimeSpan(uint days, uint hours, uint minutes, uint seconds)
    : _seconds(days * 24 * 3600 + hours * 3600 + minutes * 60 + seconds)
{
}

uint TimeSpan::getSeconds() const
{
    return this->_seconds % 60;
}
uint TimeSpan::getMinutes() const
{
    return this->_seconds / 60 % 60;
}
uint TimeSpan::getHours() const
{
    return this->_seconds / 3600 % 24;
}
uint TimeSpan::getDays() const
{
    return this->_seconds / (3600 * 24);
}

uint TimeSpan::getTotalSeconds() const
{
    return this->_seconds;
}
double TimeSpan::getTotalMinutes() const
{
    return this->_seconds / 60;
}
double TimeSpan::getTotalHours() const
{
    return this->_seconds / 3600;
}

TimeSpan& TimeSpan::operator +=(const TimeSpan& right)
{
    this->_seconds += right._seconds;
    return *this;
}
TimeSpan& TimeSpan::operator -=(const TimeSpan& right)
{
    this->_seconds -= right._seconds;
    return *this;
}
TimeSpan TimeSpan::operator +(const TimeSpan& right) const
{
     return TimeSpan(*this) += right;
}
TimeSpan TimeSpan::operator -(const TimeSpan& right) const
{
    return TimeSpan(*this) -= right;
}

void TimeSpan::addSeconds(int seconds)
{
    this->_seconds += seconds;
}
void TimeSpan::addMinutes(int minutes)
{
    this->_seconds += minutes * 60;
}
void TimeSpan::addHours(int hours)
{
    this->_seconds += hours * 3600;
}
void TimeSpan::addDays(int days)
{
    this->_seconds += days * 24 * 3600;
}

void TimeSpan::setTotalSeconds(uint seconds)
{
    this->_seconds = seconds;
}
void TimeSpan::setTotalMinutes(uint minutes)
{
    this->_seconds = minutes * 60;
}
void TimeSpan::setTotalHours(uint hours)
{
    this->_seconds = hours * 3600;
}
void TimeSpan::setTotalDays(uint days)
{
    this->_seconds = days * 24 * 3600;
}
