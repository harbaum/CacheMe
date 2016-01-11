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

#include "qgeoroutejsonparser.h"
#include "json.h"

#include <QDebug>
#include <QNetworkReply>
#include <qgeorouterequest.h>
#include <qgeoroutereply.h>

QGeoRouteJsonParser::QGeoRouteJsonParser(const QGeoRouteRequest &request)
        : m_request(request)
{
}

QGeoRouteJsonParser::~QGeoRouteJsonParser()
{
}

bool QGeoRouteJsonParser::parse(QIODevice* source)
{
    QGeoRoute route;
    QList<QGeoRoute> results;
    QString jsonResponseStr = source->readAll();
    bool ok;
    QVariantMap rootMap = Json::parse(jsonResponseStr, ok).toMap();

    if(!ok) {
        m_errorString = "JSON parsing error in server response.";
	return false;
    }

    route.setRequest(m_request);

    foreach (QString key, rootMap.keys()) {                           
        if (key == "version") {
            if (rootMap[key].toString() != "0.3") {
                m_errorString = "Unknown API " + rootMap[key].toString() + " version in server response.";
                return false;
            } 
        } else if (key == "status") {
            QString status = rootMap[key].toString();
            if (status != "0" && status != "202") {
                m_errorString = "Error code " + status + "in server response.";
                return false;
            } 
        } else if (key == "status_message") {
            m_errorString = "Error " + rootMap[key].toString() + "in server response.";
        } else if (key == "route_summary") {
            QVariantMap routeSummaryMap = rootMap[key].toMap();
          
            foreach (QString sumKey, routeSummaryMap.keys()) {
                if (sumKey == "total_distance") {
                    route.setDistance(routeSummaryMap[sumKey].toDouble());
                } else if (sumKey == "total_time") {
                    route.setTravelTime(routeSummaryMap[sumKey].toDouble());
                } else if  (sumKey == "start_point") {
		  QString startPoint = routeSummaryMap[sumKey].toString();
                } else if (sumKey == "end_point") {
		  QString endPoint = routeSummaryMap[sumKey].toString();
                }
            }
            results.append(route);
        } else if (key == "route_geometry") {
	    QList<QGeoCoordinate> path;
	    QGeoBoundingBox bounds;

            if (parseRouteGeometry(rootMap[key].toList(), &path, &bounds)) {
	        route.setPath(path);
		route.setBounds(bounds);
	    }
	    m_results.append(route);
        } else if (key == "route_instructions") {
	    QList<QGeoRouteSegment> routeSegments;
            
	    if (parseRouteInstructions(rootMap[key].toList(), &routeSegments, route)) {
	        route.setFirstRouteSegment(routeSegments.at(0));
		// for (int i = 0; i < routeSegments.size() -1; ++i)
		//    routeSegments[i].setNextRouteSegment(routeSegments.at(i + 1));
	    }
        } else if (key == "transactionId") {
	    route.setRouteId(rootMap[key].toString());
        } else {
            m_errorString = "Unknown element in server response.";
	    return false;
        }
    }

    return true;
}

bool QGeoRouteJsonParser::parseRouteGeometry(QList<QVariant> geometryList, QList<QGeoCoordinate> *geoPoints, QGeoBoundingBox *bounds)
{
    QGeoCoordinate nw;
    QGeoCoordinate so;


    for (int i = 0; i < geometryList.size(); ++i) {
        QVariantList coords = geometryList.at(i).toList();
	if (coords.length() != 2) {
	    m_errorString = "Not a valid pair of coordinates.";
	    return false;
	}

	bool ok = false;
	QString latitudeString = coords[0].toString();
	double latitude = latitudeString.toDouble(&ok);
	if (!ok) {
	    m_errorString = QString("Cannot convert latitude \"%1\" to double.").arg(latitudeString);
	    return false;
	}

	QString longitudeString = coords[1].toString();
	double longitude = longitudeString.toDouble(&ok);
	if (!ok) {
	    m_errorString = QString("Cannot convert longitude \"%1\" to double.").arg(longitudeString);
	    return false;
	}

	QGeoCoordinate geoPoint(latitude, longitude);
        geoPoints->append(geoPoint);

	// calculate the bounding box, it isn't provided
	if (!nw.isValid())
	    nw = geoPoint;
	else {
	    if (geoPoint.longitude() < nw.longitude())
	        nw.setLongitude(geoPoint.longitude());
	    if (geoPoint.latitude() > nw.latitude())
	        nw.setLatitude(geoPoint.latitude());
	}
	if (!so.isValid())
	    so = geoPoint;
	else {
	    if (geoPoint.longitude() > so.longitude())
	        so.setLongitude(geoPoint.longitude());
	    if (geoPoint.latitude() < so.latitude())
	        so.setLatitude(geoPoint.latitude());
	}
    }

    if (nw.isValid() && so.isValid()) {
        *bounds = QGeoBoundingBox(nw, so);
    }

    return true;
}

bool QGeoRouteJsonParser::parseRouteInstructions(QList<QVariant> instructionList, QList<QGeoRouteSegment> *routeSegments, QGeoRoute route)
{
    bool success = false;
    QGeoRouteSegment oldSegment;
    int oldPointIndex = -1;
    QList<QGeoCoordinate> path = route.path();

    for (int i = 0; i < instructionList.size(); ++i) {
	QList<QVariant> instruction = instructionList.at(i).toList();
	QGeoRouteSegment segment;
	QGeoManeuver maneuver;

	QString instructionText = QString::fromUtf8(instruction.at(0).toByteArray());
	maneuver.setInstructionText(instructionText);
	double distance = instruction.at(1).toDouble();
	maneuver.setDistanceToNextInstruction(distance);
	segment.setDistance(distance);

	int pointIndex = instruction.at(2).toInt();
	if (pointIndex < path.size() ) {
	    maneuver.setPosition(path.at(pointIndex));
	    if (i > 0) {
		QList<QGeoCoordinate> subPath;
		for (int j = oldPointIndex; j <= pointIndex; ++j) {
		    subPath.append(path.at(j));
		}
		oldSegment.setPath(subPath);
	    }
	} else {
	    m_errorString = QString("Position index \"%1\" too big in server response.").arg(pointIndex);
	    success = false;
	    break;
	}

	int traveltime = instruction.at(3).toInt();
	maneuver.setTimeToNextInstruction(traveltime);
	segment.setTravelTime(traveltime);

	// turn types from second segment on ...
	if (i > 0) {
	    QString turnType = instruction.at(7).toString();
	    if (turnType == "C")
		maneuver.setDirection(QGeoManeuver::DirectionForward);
	    else if (turnType == "TL")
		maneuver.setDirection(QGeoManeuver::DirectionLeft);
	    else if (turnType == "TSLL")
		maneuver.setDirection(QGeoManeuver::DirectionLightLeft);
	    else if (turnType == "TSHL")
		maneuver.setDirection(QGeoManeuver::DirectionHardLeft);
	    else if (turnType == "TR")
		maneuver.setDirection(QGeoManeuver::DirectionRight);
	    else if (turnType == "TSLR")
		maneuver.setDirection(QGeoManeuver::DirectionLightRight);
	    else if (turnType == "TSHR")
		maneuver.setDirection(QGeoManeuver::DirectionHardRight);
	    else if (turnType == "TU")
		maneuver.setDirection(QGeoManeuver::DirectionUTurnLeft); // there is no right/left u-turn in cloudemade
	    else
		maneuver.setDirection(QGeoManeuver::NoDirection);
	}

	segment.setManeuver(maneuver);
	if (i > 0) {
	    success = true;
	    oldSegment.setNextRouteSegment(segment);
	    routeSegments->append(oldSegment);
	}
	oldPointIndex = pointIndex;
	oldSegment = segment;
    }
    QList<QGeoCoordinate> subPath;
    for (int j = oldPointIndex; j < path.size(); ++j) {
	subPath.append(path.at(j));
    }
    oldSegment.setPath(subPath);
    routeSegments->append(oldSegment);
    
    return success;
}

QList<QGeoRoute> QGeoRouteJsonParser::results() const
{
    return m_results;
}

QString QGeoRouteJsonParser::errorString() const
{
    return m_errorString;
}


