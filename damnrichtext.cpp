/*
    This file is part of
    amnlib - A C++ library for deviantART Message Network
    Copyright © 2013 Carl Tessier <http://drfrankenstein90.deviantart.com/>

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

#include "damnrichtext.h"

#include "deviant.h"

#include <QString>
#include <QHash>
#include <QTextStream>
#include <QLinkedList>
#include <QStringBuilder>
#include <QStack>
#include <QUrl>
#include <QStringList>

dAmnRichText::dAmnRichText()
{
}

dAmnRichText::dAmnRichText(const QString& str)
{
    this->parse(str);
}

void dAmnRichText::parse(QString str)
{
    bool isTablump = false;
    QTextStream parser (&str);
    while(!parser.atEnd())
    {
        Element el;
        QString lump = nextLump(parser);
        if(lump.startsWith('&'))
        {
            isTablump = true;
            parseTablump(parser, el, lump.mid(1));
        }
        else
        {
            el.type = text;

            if(!isTablump) lump.prepend('\t');
            el.args.append(lump);
            this->sizehint += lump.size();
            isTablump = false;
        }

        this->tablumps.append(el);
    }
}

void dAmnRichText::parseTablump(QTextStream& parser, Element& el, const QString &typestr)
{
    ElementType type = getType(typestr);
    switch(type)
    {
    case link:
    {   QString lump;
        while((lump = nextLump(parser)) != "&")
            el.args.append(lump);
        break;
    }

    // all following cases fall through
    case thumb:
        el.args.append(nextLump(parser));

    case emote:
        el.args.append(nextLump(parser));
        el.args.append(nextLump(parser));

    case img:
    case start_iframe:
    case start_embed:
        el.args.append(nextLump(parser));

    case start_a:
    case dev:
    case avatar:
        el.args.append(nextLump(parser));

    case start_abbr:
    case start_acro:
        el.args.append(nextLump(parser));

    default: qt_noop();
    }

    el.type = type;
}

QString dAmnRichText::nextLump(QTextStream& parser)
{
    QString lump;
    while(!parser.atEnd())
    {
        QChar c;
        parser >> c;
        if(c == '\t')
            return lump;
        lump.append(c);
    }

    return QString();
}

QHash<QString, dAmnRichText::ElementType> dAmnRichText::map;

void dAmnRichText::initMap()
{
    map.insert("&b", start_b);
    map.insert("&/b", end_b);
    map.insert("&i", start_i);
    map.insert("&/i", end_i);
    map.insert("&u", start_u);
    map.insert("&/u", end_u);
    map.insert("&sub", start_sub);
    map.insert("&/sub", end_sub);
    map.insert("&sup", start_sup);
    map.insert("&/sup", end_sup);
    map.insert("&s", start_s);
    map.insert("&/s", end_s);
    map.insert("&p", start_p);
    map.insert("&/p", end_p);
    map.insert("&code", start_code);
    map.insert("&/code", end_code);
    map.insert("&bcode", start_bcode);
    map.insert("&/bode", end_bcode);
    map.insert("&li", start_li);
    map.insert("&/li", end_li);
    map.insert("&ul", start_ul);
    map.insert("&/ul", end_ul);
    map.insert("&ol", start_ol);
    map.insert("&/ol", end_ol);
    map.insert("&abbr", start_abbr);
    map.insert("&/abbr", end_abbr);
    map.insert("&acro", start_acro);
    map.insert("&/acro", end_acro);
    map.insert("&a", start_a);
    map.insert("&/a", end_a);
    map.insert("&link", link);
    map.insert("&iframe", start_iframe);
    map.insert("&/iframe", end_iframe);
    map.insert("&embed", start_embed);
    map.insert("&/embed", end_embed);
    map.insert("&br", br);
    map.insert("&dev", dev);
    map.insert("&avatar", avatar);
    map.insert("&img", img);
    map.insert("&emote", emote);
    map.insert("&thumb", thumb);
}

dAmnRichText::ElementType dAmnRichText::getType(const QString& name)
{
    return map.value(name);
}

QLinkedList<dAmnRichText::Element>::const_iterator dAmnRichText::parsedTablumps() const
{
    return this->tablumps.constBegin();
}

QString dAmnRichText::toPlain() const
{
    QString result;
    result.reserve(this->sizehint);

    QStack<Element> state;
    unsigned int listcount;

    Element el;
    foreach(el, this->tablumps)
    {
        switch(el.type)
        {
        case text: result += el.args[0]; break;

        case br: result += "\n"; break;
        case end_p: result += "\n\n"; break;

        case start_ol:
            listcount = 0;
        case start_ul:
        case start_abbr:
        case start_acro:
        case start_a:
            state.push(el);
            break;

        case start_li:
            if(state.top().type == start_ol)
                result += QString("\n %1. ").arg(listcount);
            else
                result += "\n * ";
            break;

        case end_ul:
        case end_ol: state.pop(); break;

        case end_a:
            el = state.pop();
            result += QString(" [%1]").arg(el.args[0]); // href
            break;

        case link:
            if(el.args.count() == 2)
                result += QString("%1 [%2]").arg(el.args[1]).arg(el.args[0]); // text, url
            else
                result += el.args[0];
            break;

        case img: result += QString("[%1]").arg(el.args[0]); break; // url
        case start_iframe: result += QString("[Website: %1 ").arg(el.args[0]); break;
        case start_embed: result += QString("[Embed: %1 ").arg(el.args[0]); break;
        case end_iframe: case end_embed: result += ']'; break;
        case dev: result += el.args[0] % el.args[1]; break; // symbol, name
        case avatar: result += QString("[%1]").arg(el.args[0]); break; // name
        case emote: result += el.args[0]; break;
        case thumb:
            result += QString("[%1: http://fav.me/%2 ]")
                        .arg(el.args[1]).arg(el.args[0]);  // title, id
            break;

        default: qt_noop();
        }
    }

    return result;
}

QString dAmnRichText::toHtml() const
{
    QString result;
    result.reserve(this->sizehint);

    Element el;
    foreach(el, this->tablumps)
    {
        switch(el.type)
        {
        case text: result += htmlEncode(el.args[0]); break;
        case start_b: result += "<strong>"; break;
        case end_b: result += "</strong>"; break;
        case start_i: result += "<em>"; break;
        case end_i: result += "</em>"; break;
        case start_u: result += "<span style=\"text-decoration:underline\">"; break;
        case end_u: result += "</span>"; break;
        case start_sub: result += "<sub>"; break;
        case end_sub: result += "</sub>"; break;
        case start_sup: result += "<sup>"; break;
        case end_sup: result += "</sup>"; break;
        case start_s: result += "<del>"; break;
        case end_s: result += "</del>"; break;
        case start_p: result += "<p>"; break;
        case end_p: result += "</p>"; break;
        case start_code: result += "<code>"; break;
        case end_code: result += "</code>"; break;
        case start_bcode: result += "<pre><code>"; break;
        case end_bcode: result += "</code></pre>"; break;
        case start_li: result += "<li>"; break;
        case end_li: result += "</li>"; break;
        case start_ol: result += "<ol>"; break;
        case end_ol: result += "</ol>"; break;
        case start_ul: result += "<ul>"; break;
        case end_ul: result += "</ul>"; break;
        case start_abbr: result += "<abbr title=\"" % htmlEncode(el.args[0]) % "\">"; break;
        case end_abbr: result += "</abbr>"; break;
        case start_acro: result += "<acronym title=\"" % htmlEncode(el.args[0]) % "\">"; break;
        case end_acro: result += "</acronym>"; break;
        case start_a: result += "<a href=\"" % htmlEncode(el.args[0])
                                 % "\" title=\"" % htmlEncode(el.args[1])
                                 % "\">"; break;
        case end_a: result += "</a>"; break;
        case link:
        {
            QString url = htmlEncode(el.args[0]);
            result += "<a href=\"" % url % "\" title=\"" % url % "\">"
                      % (el.args.count() == 2? htmlEncode(el.args[1]) : "[link]")
                      % "</a>";
            break;
        }
        case start_iframe: result += "<iframe src=\"" % htmlEncode(el.args[0])
                                     % "\" width=\"" % el.args[1]
                                     % "\" height=\"" % el.args[2] % "\">"; break;
        case end_iframe: result += "</iframe>"; break;
        case start_embed: result += "<embed src=\"" % htmlEncode(el.args[0])
                    % "\" width=\"" % el.args[1]
                    % "\" height=\"" % el.args[2] % "\">"; break;
        case end_embed: result += "</embed>"; break;
        case br: result += "<br>"; break;
        case dev:
        {
            QString name = el.args[1];
            result += el.args[0] % "<a href=\"http://" % name % ".deviantart.com/\">"
                    % name % "</a>"; break;
        }
        case avatar:
        {
            QString name = el.args[0];
            result += "<a href=\"http://" % name.toLower() % ".deviantart.com/\">"
                                       "<img src=\"" % Deviant::iconUrl(name, el.args[1].toInt()).toString()
                                       % "\" title=\"" % name % "\"</a>"; break;
        }
        case img: result += "<img src=\"" % htmlEncode(el.args[0])
                            % "\" alt=\"" % htmlEncode(el.args[1])
                            % "\" title=\"" % htmlEncode(el.args[2]) % "\">"; break;
        case emote: result += "<img alt=\"" % htmlEncode(el.args[0])
                            % "\" width=\"" % el.args[1] % "\" height=\"" % el.args[2]
                            % "\" title=\"" % htmlEncode(el.args[3])
                            % "\" src=\"" % htmlEncode(el.args[4]) % "\">"; break;
        case thumb:
        {
            QString id = el.args[0],
                    title = el.args[1];

            QStringList sflags = el.args[5].split(':');
            bool shadow = !sflags[0].toInt(),
                 mature = sflags[1].toInt(),
                 nothumb = sflags[2].toInt();

            Q_UNUSED(shadow); // TODO: not yet implemented

            result += "<a href=\"http://www.deviantart.com/deviation/" % id % "\">";

            if(mature)
            {
                result += "[Mature deviation: \"" % title % "\"]";
            }
            else if(nothumb)
            {
                result += "[Deviation: \"" % title % "\"]";
            }
            else
            {
                QStringList wh = el.args[2].split('x');
                result += "<img src=\"http://backend.deviantart.com/oembed?url=http://www.deviantart.com/deviation/" % id
                        % "&format=thumb150\" title=\"" % title
                        % "\" alt=\"" % title % "\" width=\"" % wh[0] % "\" height=\"" % wh[1] % "\">";
            }

            result += "</a>";
            break;
        }
        }
    }

    return result;
}

QString dAmnRichText::toDAml() const
{
    QString result;
    result.reserve(this->sizehint);

    Element el;
    foreach(el, this->tablumps)
    {
        switch(el.type)
        {
        case text: result += htmlEncode(el.args[0]); break;
        case start_b: result += "<b>"; break;
        case end_b: result += "</b>"; break;
        case start_i: result += "<i>"; break;
        case end_i: result += "</i>"; break;
        case start_u: result += "<u>"; break;
        case end_u: result += "</u>"; break;
        case start_sub: result += "<sub>"; break;
        case end_sub: result += "</sub>"; break;
        case start_sup: result += "<sup>"; break;
        case end_sup: result += "</sup>"; break;
        case start_s: result += "<s>"; break;
        case end_s: result += "</s>"; break;
        case start_p: result += "<p>"; break;
        case end_p: result += "</p>"; break;
        case start_code: result += "<code>"; break;
        case end_code: result += "</code>"; break;
        case start_bcode: result += "<bcode>"; break;
        case end_bcode: result += "</bcode>"; break;
        case start_li: result += "<li>"; break;
        case end_li: result += "</li>"; break;
        case start_ol: result += "<ol>"; break;
        case end_ol: result += "</ol>"; break;
        case start_ul: result += "<ul>"; break;
        case end_ul: result += "</ul>"; break;
        case start_abbr: result += "<abbr title=\"" % htmlEncode(el.args[0]) % "\">"; break;
        case end_abbr: result += "</abbr>"; break;
        case start_acro: result += "<acronym title=\"" % htmlEncode(el.args[0]) % "\">"; break;
        case end_acro: result += "</acronym>"; break;
        case start_a: result += "<a href=\"" % htmlEncode(el.args[0])
                                 % "\" title=\"" % htmlEncode(el.args[1])
                                 % "\">"; break;
        case end_a: result += "</a>"; break;
        case link:
            result += htmlEncode(el.args[0]);
            if(el.args.count() == 2)
                result += " (" % htmlEncode(el.args[1]) % ')';
            break;
        case start_iframe: result += "<iframe src=\"" % htmlEncode(el.args[0])
                                     % "\" width=\"" % el.args[1]
                                     % "\" height=\"" % el.args[2] % "\">"; break;
        case end_iframe: result += "</iframe>"; break;
        case start_embed: result += "<embed src=\"" % htmlEncode(el.args[0])
                    % "\" width=\"" % el.args[1]
                    % "\" height=\"" % el.args[2] % "\">"; break;
        case end_embed: result += "</embed>"; break;
        case br: result += "\n"; break;
        case dev: result += ":dev" % el.args[1] % ':'; break;
        case avatar: result += "icon" % el.args[0] % ':'; break;
        case img: result += "<img src=\"" % htmlEncode(el.args[0])
                            % "\" alt=\"" % htmlEncode(el.args[1])
                            % "\" title=\"" % htmlEncode(el.args[2]) % "\">"; break;
        case emote: result += htmlEncode(el.args[0]); break;
        case thumb: result += ":thumb" % el.args[0] % ':'; break;
        }
    }

    return result;
}

QString dAmnRichText::htmlEncode(QString text)
{   // TODO: Use Qt 5 function after upgrading from 4.8.
    return text.replace('<', "&lt;")
               .replace('>', "&gt;")
               .replace('&', "&amp;")
               .replace('"', "&quot;");
}
