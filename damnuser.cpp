#include "damnuser.h"

#include <QString>
#include <QChar>
#include <QSet>
#include <QTextStream>
#include <QPair>

class dAmnChatroom;

dAmnUser::dAmnUser(dAmnSession* parent, const QString& name, const QChar& symbol, int usericon, const QString& realname, const QString& type, const QString& gpc)
    : Deviant(parent, name, symbol, usericon, realname, type), _gpc(gpc)
{
}

QSet<dAmnChatroom*>& dAmnUser::chatrooms()
{
    return this->_joinedChatrooms;
}

void dAmnUser::setProperties(QString props)
{
    QTextStream reader (&props);

    while(!reader.atEnd())
    {
        QString line = reader.readLine();
        QPair<QString,QString> pair = dAmnPacketParser::splitPair(line);

        if(pair.first == "usericon")
        {
            bool ok;
            this->setUsericon(pair.second.toInt(&ok));
            if(!ok) MNLIB_WARN("Invalid usericon value for user %s: %s",
                               qPrintable(this->name()), qPrintable(pair.second));
        }
        else if(pair.first == "symbol") this->setSymbol(pair.second.at(0));
        else if(pair.first == "realname") this->setRealname(pair.second);
        else if(pair.first == "typename") this->setTypeName(pair.second);
        else if(pair.first == "gpc") this->_gpc = pair.second;
        else
        {
            MNLIB_WARN("Unknown user property %s = %s for %s",
                       qPrintable(pair.first), qPrintable(pair.second), qPrintable(this->name()));
        }
    }
}
