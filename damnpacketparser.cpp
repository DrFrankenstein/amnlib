/*
    This file is part of
    amnlib - A C++ library for deviantART Message Network
    Copyright © 2011 Carl Tessier <http://drfrankenstein90.deviantart.com/>

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

#include "damnpacketparser.h"

class QByteArray;
class dAmnSession;

#include <QTextStream>
#include <QString>
#include <QChar>
#include <QHash>
#include "damnpacket.h"

dAmnPacketParser::dAmnPacketParser(dAmnSession* session)
    : session(session)
{
}

dAmnPacket* dAmnPacketParser::parsePacket(QByteArray* raw)
{
    QTextStream stream (raw);
    ParserState state = Cmd;

    QString cmd, param, arg_name, arg_value, data;
    QHash<QString, QString> args;

    while(!stream.atEnd())
    {
        QChar c;
        stream >> c;

        switch(state)
        {
        case Cmd:
            if(c == ' ')
            {
                if(cmd.isEmpty())
                {
                    MNLIB_CRIT("Parse error in packet: empty cmd.");
                    return NULL;
                }
                state = Param;
                break;
            }
            if(c == '\n')
            {
                if(cmd.isEmpty())
                {
                    MNLIB_CRIT("Parse error in packet: empty cmd.");
                    return NULL;
                }
                state = ArgName;
                break;
            }

            cmd += c;
            break;

        case Param:
            if(c == '\n')
            {
                state = ArgName;
                break;
            }

            param += c;
            break;

        case ArgName:
            if(c == '=')
            {
                if(arg_name.isEmpty())
                {
                    MNLIB_CRIT("Parse error in packet: Argument with no name.");
                    return NULL;
                }
                state = ArgValue;
                break;
            }
            if(c == '\n')
            {
                if(!arg_name.isEmpty())
                {
                    MNLIB_CRIT("Parse error in packet: Stray data in args: %s",
                               qPrintable(arg_name));
                    return NULL;
                }
                state = Data;
                break;
            }

            arg_name += c;
            break;

        case ArgValue:
            if(c == '\n')
            {
                if(arg_value.isEmpty())
                {
                    MNLIB_WARN("Argument '%s' in packet has empty value.",
                               qPrintable(arg_name));
                }
                args[arg_name] = arg_value;
                arg_name = QString();
                arg_value = QString();

                state = ArgName;
                break;
            }

            arg_value += c;
            break;

        case Data:
            data = c + stream.readAll();
        }
    }

    dAmnPacket* packet = new dAmnPacket(this->session,
                                        cmd, param, data);
    packet->setArgs(args);

    return packet;
}

QPair<QString, QString> dAmnPacketParser::splitPair(const QString& line)
{
    int idx = line.indexOf('=');
    return qMakePair(line.mid(0, idx), line.mid(idx + 1));
}
