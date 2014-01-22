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

#include "scrapingauthenticationprovider.h"

class QObject;
class dAmnSession;

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QList>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QUrl>
#include <QByteArray>
#include <QVariant>
#include <QRegExp>
#include "damnobject.h"
#include "damnsession.h"

ScrapingAuthenticationProvider::ScrapingAuthenticationProvider(QObject* parent):
    QObject(parent), http(this)
{
    connect(&this->http, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)),
            this, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)));
    connect(&this->http, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
            this, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)));
}

void ScrapingAuthenticationProvider::requestAuthToken(const QString& username,
                                                      const QString& password,
                                                      bool remember_me)
{
    //if(this->cookieReply)
    //    return;

    QByteArray loginData = QString("username=%1&password=%2&remember_me=%3").arg(
                QUrl::toPercentEncoding(username),
                QUrl::toPercentEncoding(password),
                QString().number(remember_me)
    ).toLatin1();
    //qDebug() << loginData;

    QNetworkRequest request (QUrl("https://www.deviantart.com/users/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    request.setHeader(QNetworkRequest::ContentLengthHeader, loginData.size());

    MNLIB_DEBUG("Requesting authentication for user %s", qPrintable(username));
    this->cookieReply = this->http.post(request, loginData);
    connect(this->cookieReply, SIGNAL(finished()), this, SLOT(gotCookie()));
    connect(this->cookieReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(cookieError(QNetworkReply::NetworkError)));
}

void ScrapingAuthenticationProvider::gotCookie()
{
    if(this->cookieReply->error() == QNetworkReply::NoError)
    {
        //QNetworkRequest req = this->cookieReply->request();

        //qDebug() << "REQUEST--------------------------------------------------------";
        //qDebug() << "ContentTypeHeader=" << req.header(QNetworkRequest::ContentTypeHeader);
        //QByteArray header;
        //foreach(header, req.rawHeaderList())
        //{
        //   qDebug() << header << req.rawHeader(header);
        //}

        //qDebug() << "\nRESPONSE-----------------------------------------------------";
        //qDebug() << this->cookieReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        //qDebug() << this->cookieReply->attribute(QNetworkRequest::HttpReasonPhraseAttribute);
        //qDebug() << "ContentTypeHeader=" << req.header(QNetworkRequest::ContentTypeHeader);
        //qDebug() << "LocationHeader=" << req.header(QNetworkRequest::LocationHeader);
        //QNetworkReply::RawHeaderPair headerpair;
        //foreach(headerpair, this->cookieReply->rawHeaderPairs())
        //{
        //    qDebug() << headerpair.first << ": " << headerpair.second;
        //}

        QList<QNetworkCookie> cookies = this->http.cookieJar()
                ->cookiesForUrl(QUrl("https://www.deviantart.com/"));
        bool found = false;
        foreach(QNetworkCookie cookie, cookies)
        {
            //qDebug() << cookie.name() << '=' << cookie.value();
            if(cookie.name() == "auth")
            {
                found = true;
                break;
            }
        }

        if(found)
        {
            QNetworkRequest request (QUrl("http://chat.deviantart.com/chat/devart"));
            this->tokenReply = this->http.get(request);
            connect(this->tokenReply, SIGNAL(finished()), this, SLOT(gotToken()));
            connect(this->tokenReply, SIGNAL(error(QNetworkReply::NetworkError)),
                    this, SLOT(tokenError(QNetworkReply::NetworkError)));
        }
        else
        {
            MNLIB_CRIT("Authentication failed. (no cookie)");
            emit noToken();
        }
    }

    this->cookieReply->close();
    this->cookieReply->deleteLater();
    this->cookieReply = NULL;
}

void ScrapingAuthenticationProvider::gotToken()
{
    QString data (this->tokenReply->readAll());
    this->tokenReply->close();

    QRegExp rx ("dAmn_Login\\s*\\(\\s*\".*\"\\s*,\\s*\"([0-9a-f]{32})\"");
    int pos = rx.indexIn(data);
    if(pos >= 0)
    {
        QByteArray token = rx.cap(1).toLatin1();
        MNLIB_DEBUG("Token accepted: %s", token.data());
        emit tokenAvailable(token);
    }
    else
    {
        MNLIB_CRIT("Authentication failed. (couldn't scrape token)");
        emit noToken();
    }

    this->tokenReply->deleteLater();
    this->tokenReply = NULL;
}

void ScrapingAuthenticationProvider::cookieError(QNetworkReply::NetworkError errorcode)
{
    emit error(errorcode, tr("Could not get authentication cookie: %1").arg(this->cookieReply->errorString()));
}

void ScrapingAuthenticationProvider::tokenError(QNetworkReply::NetworkError errorcode)
{
    emit error(errorcode, tr("Could not fetch authentication token: %1").arg(this->tokenReply->errorString()));
}
