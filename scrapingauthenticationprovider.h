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

#ifndef SCRAPINGAUTHENTICATIONPROVIDER_H
#define SCRAPINGAUTHENTICATIONPROVIDER_H

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "mnlib_global.h"
#include "damnobject.h"

class QNetworkProxy;
class QAuthenticator;
template<typename T> class QList;
class QSslError;

class dAmnSession;

class MNLIBSHARED_EXPORT ScrapingAuthenticationProvider : public QObject
{
    Q_OBJECT

    QNetworkAccessManager http;
    QNetworkReply* cookieReply, * tokenReply;

public:
    explicit ScrapingAuthenticationProvider(QObject* parent = NULL);

    void requestAuthToken(const QString& username, const QString& password, bool remember_me = true);

signals:
    void proxyAuthenticationRequired(const QNetworkProxy& proxy,
                                     QAuthenticator* authenticator);
    void sslErrors(QNetworkReply* reply, const QList<QSslError>& errors);

    void tokenAvailable(const QByteArray& token);
    void error(QNetworkReply::NetworkError error);
    void noToken();

private slots:
    void gotCookie();
    void gotToken();
};

#endif // SCRAPINGAUTHENTICATIONPROVIDER_H
