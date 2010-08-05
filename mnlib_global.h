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

#ifndef MNLIB_GLOBAL_H
#define MNLIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(MNLIB_LIBRARY)
#  define MNLIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MNLIBSHARED_EXPORT Q_DECL_IMPORT
#endif

extern const char* MNLIB_VERSION;
extern const char* DAMN_VERSION;

#if MNLIB_DEBUG_BUILD
#   define MNLIB_STUB qDebug("%s: STUB", __FUNCTION__);
#else
#   define MNLIB_STUB
#endif

// Variadic macros are a non-standard extension to C++.
// They seem to be supported by current versions of both compilers I target,
// which are CL and GCC.
// If you have any problems compiling, use the following instead.
#if 0
#   define MNLIB_WARN(text) qWarning("%s: %s", __FUNCTION__, (text))
#   define MNLIB_CRIT(text) qCritical("%s: %s", __FUNCTION__, (text))
#   define MNLIB_FAIL(text) qFatal("%s: %s", __FUNCTION__, (text))
#else
#   define MNLIB_WARN(text, ...) qWarning("%s: " text, __FUNCTION__, ##__VA_ARGS__)
#   define MNLIB_CRIT(text, ...) qCritical("%s: " text, __FUNCTION__, ##__VA_ARGS__)
#   define MNLIB_FAIL(text, ...) qFatal("%s: " text, __FUNCTION__,  ##__VA_ARGS__)
#endif

#if MNLIB_DEBUG_BUILD
#   define MNLIB_DEBUG(text, ...) qDebug("%s: " text, __FUNCTION__, ##__VA_ARGS__)
#else
#   define MNLIB_DEBUG(text, ...) (void)qt_noop()
#endif

#endif // MNLIB_GLOBAL_H
