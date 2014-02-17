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

#ifndef DAMNRICHTEXT_H
#define DAMNRICHTEXT_H

#include <QString>
#include <QLinkedList>
#include <QHash>
#include <QTextStream>

#include "mnlib_global.h"

class MNLIBSHARED_EXPORT dAmnRichText
{
public:
    enum ElementType
    {
        text,
        start_b, end_b, start_i, end_i, start_u, end_u,
        start_sub, end_sub, start_sup, end_sup,
        start_s, end_s, start_p, end_p,
        start_code, end_code, start_bcode, end_bcode,
        start_li, end_li, start_ul, end_ul, start_ol, end_ol,
        start_abbr, end_abbr, start_acro, end_acro,
        start_a, end_a, link,
        start_iframe, end_iframe, start_embed, end_embed,
        br,
        dev, avatar, img, emote, thumb
    };

    struct Element
    {
        ElementType type;
        QList<QString> args;
    };

private:
    QLinkedList<Element> tablumps;

    static QHash<QString, ElementType> map;

    static void initMap();
    static ElementType getType(const QString &name);

    void parse(QString str);
    bool parseText(QTextStream& parser, Element& el);
    void parseTablump(QTextStream& parser, Element& el);
    QString nextLump(QTextStream& parser, bool typestr = false);

    static QString htmlEncode(QString text);

public:
    dAmnRichText();
    dAmnRichText(const QString& str);

    QString toPlain() const;
    QString toHtml() const;
    QString toDAml() const;
    QString toTablumps() const;
    QLinkedList<Element>::const_iterator parsedTablumps() const;
};

#endif // DAMNRICHTEXT_H
