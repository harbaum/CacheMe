/****************************************************************************
**
** Copyright (C) 2011 Cloudmade 
** All rights reserved.
** Contact: Cloudmade (qtmobility@cloudmade.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you have questions regarding the use of this file, please contact
** Cloudmade at qtmobility@cloudmade.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qgeosearchmanagerengine_cm.h"
#include "qgeosearchreply_cm.h"
#include "debug_cm.h"
#include "qgeoboundingcircle.h"

#include <qgeoaddress.h>
#include <qgeocoordinate.h>
#include <QNetworkProxy>
#include <QMap>
#include <QApplication>
#include <QFileInfo>

//http://wiki.openstreetmap.org/wiki/Nominatim

QGeoSearchManagerEngineCm::QGeoSearchManagerEngineCm(
    const QMap<QString, QVariant> &parameters, QGeoServiceProvider::Error *error,
    QString *errorString)
        : QGeoSearchManagerEngine(parameters),
	  m_host("geocoding.cloudmade.com"),
	  m_token(QGeoServiceProviderFactoryCm::defaultToken)
{
    m_networkManager = new QNetworkAccessManager(this);

    QList<QString> keys = parameters.keys();

    if (keys.contains("places.proxy")) {
        QString proxy = parameters.value("places.proxy").toString();
        if (!proxy.isEmpty())
            m_networkManager->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy, 8080));
    }

    if (keys.contains("places.host")) {
        QString host = parameters.value("places.host").toString();
        if (!host.isEmpty())
            m_host = host;
    }

    setSupportsGeocoding(true);
    setSupportsReverseGeocoding(true);

    QGeoSearchManager::SearchTypes supportedSearchTypes;
    supportedSearchTypes |= QGeoSearchManager::SearchLandmarks;
    supportedSearchTypes |= QGeoSearchManager::SearchGeocode;
    setSupportedSearchTypes(supportedSearchTypes);

    if (error)
        *error = QGeoServiceProvider::NoError;

    if (errorString)
        *errorString = "";
}

QGeoSearchManagerEngineCm::~QGeoSearchManagerEngineCm() {}

QGeoSearchReply* QGeoSearchManagerEngineCm::geocode(const QGeoAddress &address,
        QGeoBoundingArea *bounds)
{
    if (!supportsGeocoding()) {
        QGeoSearchReply *reply = new QGeoSearchReply(QGeoSearchReply::UnsupportedOptionError, "Geocoding is not supported by this service provider.", this);
        emit error(reply, reply->error(), reply->errorString());
        return reply;
    }
    QString searchString;

    if(!address.street().isEmpty()) {
        searchString += address.street();
        searchString += ",";
    }
    if(!address.city().isEmpty()) {
        searchString += address.city();
        searchString += ",";
    }
    if(!address.postcode().isEmpty()) {
        searchString += address.postcode();
        searchString += ",";
    }
    if(!address.district().isEmpty()) {
        searchString += address.district();
        searchString += ",";
    }
    if(!address.county().isEmpty()) {
        searchString += address.county();
        searchString += ",";
    }
    if(!address.state().isEmpty()) {
        searchString += address.state();
        searchString += ",";
    }
    if(!address.country().isEmpty()) {
        searchString += address.country();
        searchString += ",";
    }
    if(!address.countryCode().isEmpty()) {
        searchString += address.countryCode();
        searchString += ",";
    }

	// Note:  this code is not called?
	DBG_CM(SEARCH_M, INFO_L, "Warning - geocode() method is called\n");


    QString cmSearchUrl = "http://" + m_host + "/" + m_token +"/geocoding/v2/find.js?query=";
    cmSearchUrl += searchString;

	// return location information like road, city, county, country, postcode in returned results:
    cmSearchUrl += "&return_location=true";

    return search(cmSearchUrl, QGeoSearchManager::SearchGeocode, -1, 0,bounds);
}

QGeoSearchReply* QGeoSearchManagerEngineCm::reverseGeocode(const QGeoCoordinate &coordinate,
        QGeoBoundingArea *bounds)
{
    if (!supportsReverseGeocoding()) {
        QGeoSearchReply *reply = new QGeoSearchReply(QGeoSearchReply::UnsupportedOptionError, "Reverse geocoding is not supported by this service provider.", this);
        emit error(reply, reply->error(), reply->errorString());
        return reply;
    }

	// Prepare request url for ReverseGeocoding:

//	QString requestString = "http://" + m_host + "/" + m_token +"/geocoding/v2/find.js?around=51.51384,-0.10952&distance=closest";
	QString requestString = "http://" + m_host + "/" + m_token +"/geocoding/v2/find.js?around=";
	requestString += QString::number(coordinate.latitude());
	requestString += ",";
	requestString += QString::number(coordinate.longitude());
	//requestString += "&zoom=18";
	requestString += "&distance=closest";
	requestString += "&return_location=true";

    return search(requestString, bounds);
}

QGeoSearchReply* QGeoSearchManagerEngineCm::search(const QString &searchString,
        QGeoSearchManager::SearchTypes searchTypes,
        int limit,
        int offset,
        QGeoBoundingArea *bounds)
{
	// Prepare request url for Geocoding:
	QString requestString = "http://" + m_host + "/" + m_token +"/geocoding/v2/find.js?query=" + searchString;


	if (limit > 0) {
		// Number of results to return 
		requestString += "&results=" + QString::number(limit);
	}
	if (offset > 0) {
		// Number of results to skip from beginning (allow paging of results)
		requestString += "&skip=" + QString::number(offset);
	}

	if ((bounds) && (bounds->isValid())) {
        QGeoBoundingBox* box = 0;
        QGeoBoundingCircle* circle = 0;
        switch (bounds->type()) {

			// Note:  Searching with Bounding area specified could be extremely slow for large areas
			//		  so use it only with high zoom level in frontend map application
            case QGeoBoundingArea::BoxType:
                box = static_cast<QGeoBoundingBox*>(bounds);
                if (box && box->isValid()) {
					DBG_CM(SEARCH_M, INFO_L, "Using search with bounding box specified");
					// "southern_latitude,western_longitude,northern_latitude,eastern_longitude"
					requestString += "&bbox=";
					requestString += QString::number(box->bottomLeft().latitude()) + "," ;	// southern_latitude
					requestString += QString::number(box->bottomLeft().longitude()) + "," ;	// western_longitude
					requestString += QString::number(box->topRight().latitude()) + "," ;	// northern_latitude
					requestString += QString::number(box->topRight().longitude());			// eastern_longitude
					requestString += "&bbox_only=false";
                }
                break;
            case QGeoBoundingArea::CircleType:
                circle = static_cast<QGeoBoundingCircle*>(bounds);
                if (circle && circle->isValid()) {
					DBG_CM(SEARCH_M, ERR_L, "Using search with circle area specified: Not implemented. Around area is ignored!");
                }
                break;
            default:
                break;
        }

	}

	// return location information like road, city, county, country, postcode in returned results:
    // requestString += "&return_location=true";

	return search(requestString, bounds, limit, offset);
}

QGeoSearchReply* QGeoSearchManagerEngineCm::search(QString requestString,
                                                    QGeoBoundingArea *bounds,
                                                    int limit,
                                                    int offset)
{
    QNetworkRequest netRequest = QNetworkRequest(QUrl(requestString));

    QString ua = QFileInfo(QApplication::applicationFilePath()).fileName();
    ua.remove(QChar('"'), Qt::CaseInsensitive);
    ua += " (Qt";
    ua += qVersion();
    ua += " QtMobility 1.1 ) cm GeoSearchManager";
    netRequest.setRawHeader("User-Agent", ua.toAscii());

	DBG_CM(SEARCH_M, INFO_L, "netRequest" << netRequest.url().toString() );
	DBG_CM(SEARCH_M, INFO_L, "netRequest (URL encoded)" << netRequest.url().toEncoded() );

    QNetworkReply *networkReply = m_networkManager->get(netRequest);

    QGeoSearchReplyCm *reply = new QGeoSearchReplyCm(networkReply, limit, offset, bounds, this);

    connect(reply,
            SIGNAL(finished()),
            this,
            SLOT(placesFinished()));

    connect(reply,
            SIGNAL(error(QGeoSearchReply::Error, QString)),
            this,
            SLOT(placesError(QGeoSearchReply::Error, QString)));

    return reply;
}

void QGeoSearchManagerEngineCm::placesFinished()
{
    QGeoSearchReply *reply = qobject_cast<QGeoSearchReply*>(sender());

    if (!reply)
        return;

    if (receivers(SIGNAL(finished(QGeoSearchReply*))) == 0) {
        reply->deleteLater();
        return;
    }

    emit finished(reply);
}

void QGeoSearchManagerEngineCm::placesError(QGeoSearchReply::Error error, const QString &errorString)
{
    QGeoSearchReply *reply = qobject_cast<QGeoSearchReply*>(sender());

    if (!reply)
        return;

    if (receivers(SIGNAL(error(QGeoSearchReply*, QGeoSearchReply::Error, QString))) == 0) {
        reply->deleteLater();
        return;
    }

    emit this->error(reply, error, errorString);
}
