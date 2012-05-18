#include "damnuser.h"

#include <QString>
#include <QChar>
#include <QSet>

class dAmnChatroom;

dAmnUser::dAmnUser(dAmnSession* parent, const QString& name, const QChar& symbol, int usericon, const QString& realname, const QString& type)
        : Deviant(parent, name, symbol, usericon, realname, type)
{
}

QSet<dAmnChatroom*>& dAmnUser::getChatrooms()
{
    return this->joinedChatrooms;
}
