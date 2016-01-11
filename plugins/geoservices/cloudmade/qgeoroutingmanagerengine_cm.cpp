/****************************************************************************
**
** Copyright (C) 2011 Cloudmade 
** All rights reserved.
** Contact: Cloudmade (info@cloudmade.com)
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
** Cloudmade at info@cloudmade.com.
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

#include "qgeoroutingmanagerengine_cm.h"
#include "qgeoroutereply_cm.h"

#include <QStringList>
#include <QNetworkProxy>
#include <qgeoboundingbox.h>

QGeoRoutingManagerEngineCm::QGeoRoutingManagerEngineCm(const QMap<QString, QVariant> &parameters, QGeoServiceProvider::Error *error, QString *errorString)
        : QGeoRoutingManagerEngine(parameters),
        m_host("navigation.cloudmade.com"),
        m_token(QGeoServiceProviderFactoryCm::defaultToken)
{
    m_networkManager = new QNetworkAccessManager(this);

    if (parameters.contains("routing.proxy")) {
        QString proxy = parameters.value("routing.proxy").toString();
        if (!proxy.isEmpty())
            m_networkManager->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy, 8080));
    }

    if (parameters.contains("routing.host")) {
        QString host = parameters.value("routing.host").toString();
        if (!host.isEmpty())
            m_host = host;
    }

    if (parameters.contains("routing.token")) {
        m_token = parameters.value("routing.token").toString();
    }
    else if (parameters.contains("token")) {
        m_token = parameters.value("token").toString();
    }

    setSupportsRouteUpdates(false);
    setSupportsAlternativeRoutes(false);
    setSupportsExcludeAreas(true);

    QGeoRouteRequest::FeatureTypes featureTypes;
    featureTypes |= QGeoRouteRequest::NoFeature;
    setSupportedFeatureTypes(featureTypes);

    QGeoRouteRequest::FeatureWeights featureWeights;
    featureWeights |= QGeoRouteRequest::AvoidFeatureWeight;
    setSupportedFeatureWeights(featureWeights);

    QGeoRouteRequest::ManeuverDetails maneuverDetails;
    maneuverDetails |= QGeoRouteRequest::BasicManeuvers;
    setSupportedManeuverDetails(maneuverDetails);

    QGeoRouteRequest::RouteOptimizations optimizations;
    optimizations |= QGeoRouteRequest::ShortestRoute;
    optimizations |= QGeoRouteRequest::FastestRoute;
    setSupportedRouteOptimizations(optimizations);

    QGeoRouteRequest::TravelModes travelModes;
    travelModes |= QGeoRouteRequest::CarTravel;
    travelModes |= QGeoRouteRequest::PedestrianTravel;
    travelModes |= QGeoRouteRequest::BicycleTravel;
    setSupportedTravelModes(travelModes);

    QGeoRouteRequest::SegmentDetails segmentDetails;
    segmentDetails |= QGeoRouteRequest::BasicSegmentData;
    setSupportedSegmentDetails(segmentDetails);

    if (error)
        *error = QGeoServiceProvider::NoError;

    if (errorString)
        *errorString = "";
}

QGeoRoutingManagerEngineCm::~QGeoRoutingManagerEngineCm() {}

QGeoRouteReply* QGeoRoutingManagerEngineCm::calculateRoute(const QGeoRouteRequest& request)
{
    QString reqString = calculateRouteRequestString(request);

    if (reqString.isEmpty()) {
        QGeoRouteReply *reply = new QGeoRouteReply(QGeoRouteReply::UnsupportedOptionError, "The given route request options are not supported by this service provider.", this);
        emit error(reply, reply->error(), reply->errorString());
        return reply;
    }

    QNetworkReply *networkReply = m_networkManager->get(QNetworkRequest(QUrl(reqString)));
    QGeoRouteReplyCm *reply = new QGeoRouteReplyCm(request, networkReply, this);

    connect(reply,
            SIGNAL(finished()),
            this,
            SLOT(routeFinished()));

    connect(reply,
            SIGNAL(error(QGeoRouteReply::Error, QString)),
            this,
            SLOT(routeError(QGeoRouteReply::Error, QString)));

    return reply;
}

bool QGeoRoutingManagerEngineCm::checkEngineSupport(const QGeoRouteRequest &request,
        QGeoRouteRequest::TravelModes travelModes) const
{
    QList<QGeoRouteRequest::FeatureType> featureTypeList = request.featureTypes();
    QGeoRouteRequest::FeatureTypes featureTypeFlag = QGeoRouteRequest::NoFeature;
    QGeoRouteRequest::FeatureWeights featureWeightFlag = QGeoRouteRequest::NeutralFeatureWeight;

    for (int i = 0; i < featureTypeList.size(); ++i) {
        featureTypeFlag |= featureTypeList.at(i);
        featureWeightFlag |= request.featureWeight(featureTypeList.at(i));
    }

    if ((featureTypeFlag & supportedFeatureTypes()) != featureTypeFlag)
        return false;

    if ((featureWeightFlag & supportedFeatureWeights()) != featureWeightFlag)
        return false;


    if ((request.maneuverDetail() & supportedManeuverDetails()) != request.maneuverDetail())
        return false;

    if ((request.segmentDetail() & supportedSegmentDetails()) != request.segmentDetail())
        return false;

    if ((request.routeOptimization() & supportedRouteOptimizations()) != request.routeOptimization())
        return false;

    if ((travelModes & supportedTravelModes()) != travelModes)
        return false;

    return true;
}

QString QGeoRoutingManagerEngineCm::calculateRouteRequestString(const QGeoRouteRequest &request)
{
    bool supported = checkEngineSupport(request, request.travelModes());

    if ((request.numberAlternativeRoutes() != 0) && !supportsAlternativeRoutes())
        supported = false;

    if (!supported)
        return "";

    QString requestString = "http://";
    requestString += m_host;
    if (!m_token.isNull())
        requestString += "/" + m_token;

    requestString += "/api/0.3/";

    int numWaypoints = request.waypoints().size();
    if (numWaypoints < 2)
        return "";

    for (int i = 0;i < numWaypoints;++i) {
        if (i)
	    requestString += ",";
        requestString += trimDouble(request.waypoints().at(i).latitude());
        requestString += ",";
        requestString += trimDouble(request.waypoints().at(i).longitude());
    }

    requestString += modesRequestString(request, request.travelModes());

    requestString += routeRequestString(request);

    // qDebug() << "SL:" << requestString;

    return requestString;
}

QString QGeoRoutingManagerEngineCm::modesRequestString(const QGeoRouteRequest &request,
        QGeoRouteRequest::TravelModes travelModes) const
{
    QString requestString;

    QGeoRouteRequest::RouteOptimizations optimization = request.routeOptimization();

    QStringList types;
    if (optimization.testFlag(QGeoRouteRequest::ShortestRoute))
        types.append("shortest");
    if (optimization.testFlag(QGeoRouteRequest::FastestRoute))
        types.append("fastest");
    QStringList modes;
    if (travelModes.testFlag(QGeoRouteRequest::CarTravel))
        modes.append("car");
    if (travelModes.testFlag(QGeoRouteRequest::PedestrianTravel))
        modes.append("foot");
    if (travelModes.testFlag(QGeoRouteRequest::BicycleTravel))
        modes.append("bicycle");

    requestString = "/" + modes[0];
    if (modes[0] == "car") 
        requestString += "/" + types[0];
    requestString += ".js";
    requestString += "?lang=" + locale().name();
    return requestString;
}

QString QGeoRoutingManagerEngineCm::routeRequestString(const QGeoRouteRequest &request) const
{
    QString requestString;

    int numAreas = request.excludeAreas().count();
    if (numAreas > 0) {
        requestString += "&blockedRoad";
        for (int i = 0;i < numAreas;++i) {
            requestString += i == 0 ? "=" : ",";
            QGeoBoundingBox box = request.excludeAreas().at(i);
            requestString += trimDouble(box.topLeft().latitude());
            requestString += ",";
            requestString += trimDouble(box.topLeft().longitude());
            requestString += ",";
            requestString += trimDouble(box.bottomRight().latitude());
            requestString += ",";
            requestString += trimDouble(box.bottomRight().longitude());
        }
    }

    return requestString;
}

QString QGeoRoutingManagerEngineCm::trimDouble(qreal degree, int decimalDigits)
{
    QString sDegree = QString::number(degree, 'g', decimalDigits);

    int index = sDegree.indexOf('.');

    if (index == -1)
        return sDegree;
    else
        return QString::number(degree, 'g', decimalDigits + index);
}

void QGeoRoutingManagerEngineCm::routeFinished()
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply*>(sender());

    if (!reply)
        return;

    if (receivers(SIGNAL(finished(QGeoRouteReply*))) == 0) {
        reply->deleteLater();
        return;
    }

    emit finished(reply);
}

void QGeoRoutingManagerEngineCm::routeError(QGeoRouteReply::Error error, const QString &errorString)
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply*>(sender());

    if (!reply)
        return;

    if (receivers(SIGNAL(error(QGeoRouteReply*, QGeoRouteReply::Error, QString))) == 0) {
        reply->deleteLater();
        return;
    }

    emit this->error(reply, error, errorString);
}
