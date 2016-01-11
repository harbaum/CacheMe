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

#include "qgeocodeparser.h"
#include "debug_cm.h"

#include <QXmlStreamReader>
#include <QIODevice>
#include <QStringList>
#include "json.h"


QGeoCodeParser::QGeoCodeParser()
{
}

QGeoCodeParser::~QGeoCodeParser()
{
}

void QGeoCodeParser::debugMap(QVariantMap &map)
{
	foreach (QString key, map.keys()) {
		DBG_CM(SEARCH_M, INFO_L, key << ":" << map[key].typeName() << ", ");
	}
}

bool QGeoCodeParser::parse(QIODevice* source)
{
	QGeoBoundingBox bounds;
	QString jsonResponseStr = QString::fromUtf8(source->readAll());	// string containing response data to parse
	DBG_CM(SEARCH_M, INFO_L, "QGeoCodeParser::parse\n" << "netReply:" << jsonResponseStr );
	bool ok;
	QVariant root = Json::parse(jsonResponseStr, ok);

	if(!ok) {
		m_errorString = "JSon reply parsing error";
		DBG_CM(SEARCH_M, ERR_L, m_errorString);
		return false;
	}
	DBG_CM(SEARCH_M, INFO_L, "JSON reply parsed. Root item type: " << root.typeName() );
    QVariantMap rootMap = root.toMap();
	debugMap(rootMap);

	if (rootMap.contains(FEATURES_JSON)) {
		parseFeatures(rootMap[FEATURES_JSON]);
	}
	else
	{
		DBG_CM(SEARCH_M, ERR_L, "QGeoCodeParser::parse() error: ""features"" element in json reply not found\n" );
		m_errorString = "No search results  (""features"" element in json reply not found)";
		return false;
	}

	m_errorString = "";
	return true;
}

bool QGeoCodeParser::parseFeatures(QVariant &f)
{
	if ( !f.canConvert<QVariantList>() ) {

		m_errorString = "can't convert json ""features"" to list";
		DBG_CM(SEARCH_M, ERR_L, m_errorString);
		return false;
	}

	QVariantList fList = f.toList();
	// QList is implemented as an array of pointers, .at() operation is very fast
	for (int i = 0; i < fList.size(); ++i) {
		if (fList.at(i).canConvert<QVariantMap>()) {
			parseSingleFeature(fList.at(i));
		}
	}
    return true;
}

bool QGeoCodeParser::parseSingleFeature(const QVariant &f)
{
	if ( !f.canConvert<QVariantMap>() ) {

		m_errorString = "can't convert features list item to QVariantMap";
		DBG_CM(SEARCH_M, ERR_L, m_errorString);
		return false;
	}

	QVariantMap fMap = f.toMap();
	QLandmark landmark;

	bool centroidOk = parseCentroid(fMap, landmark);
	bool boundsOk = parseBounds(fMap, landmark);
	bool propertiesOk = parseProperties(fMap, landmark);
	bool locationOk = parseLocation(fMap, landmark);

	if (!propertiesOk) {
		DBG_CM(SEARCH_M, WARN_L, "parseSingleFeature(): properties parsed with errors, ignored!");
	}

	if (!locationOk) {
		DBG_CM(SEARCH_M, WARN_L, "parseSingleFeature(): location parsed with errors, ignored!");
	}

	if (centroidOk && boundsOk) {
		m_results.append(landmark);
		DBG_CM(SEARCH_M, INFO_L, "place successfully parsed and added to results!!");
	}
	else
	{
		DBG_CM(SEARCH_M, WARN_L, "parseSingleFeature(): place parsed with errors, and will not be added to results");
		return false;
	}

    return true;
}

bool QGeoCodeParser::parseCentroid(const QVariantMap &map, QLandmark &landmark)
{
	if (map.contains(CENTROID_JSON)) {
		if (map[CENTROID_JSON].canConvert<QVariantMap>()) {
			QVariantMap centroidMap = map[CENTROID_JSON].toMap();

			if (centroidMap.contains(COORDINATES_JSON)) {
				QGeoCoordinate coord;
				bool ok = parseCoordinates(centroidMap[COORDINATES_JSON], coord);
				if (ok) {
					landmark.setCoordinate(coord);
					return true;
				}
			}
		}
		else
		{
			DBG_CM(SEARCH_M, ERR_L, "centroid data can't be converted to QVariantMap");
		}
	}
	else
	{
		DBG_CM(SEARCH_M, WARN_L, "centroid is not found");
	}
    return false;
}

bool QGeoCodeParser::parseCoordinates(const QVariant &v, QGeoCoordinate &coord)
{
	if (v.canConvert<QVariantList>()) {
		QVariantList list = v.toList();
		if (list.size() != 2) {
			DBG_CM(SEARCH_M, ERR_L, "Error parsing coordinates - list size must be 2");
			return false;
		}
		coord.setLatitude(list.first().toDouble());
		coord.setLongitude(list.last().toDouble());
		return true;
	}
	else
	{
		DBG_CM(SEARCH_M, ERR_L, "Error parsing coordinates - can't convert to list");
	}

    return false;
}

bool QGeoCodeParser::parseBounds(const QVariantMap &map, QLandmark &landmark)
{
	if (map.contains(BOUNDS_JSON)) {
		QGeoBoundingBox bounds;

		bool ok = parseBoundingBox(&bounds, map[BOUNDS_JSON]);
		if (ok) {
			landmark.setViewport(bounds);
			return true;
		}
	}
	else
	{
		DBG_CM(SEARCH_M, WARN_L, "parseBounds(): QVariantMap does not contain bounds entry");
	}

    return false;
}


bool QGeoCodeParser::parseProperties(const QVariantMap &map, QLandmark &landmark)
{
	if (map.contains(PROPERTIES_JSON) &&
		map[PROPERTIES_JSON].canConvert<QVariantMap>() ) {

		DBG_CM(SEARCH_M, INFO_L, "Start parsing properties..");

		QVariantMap pMap = map[PROPERTIES_JSON].toMap();
		QGeoAddress address;
		QString field;

		field = NAME_JSON;
		if (pMap.contains(field) && pMap[field].canConvert<QString>()) {
			landmark.setName(pMap[field].toString());
			DBG_CM(SEARCH_M, INFO_L, "place name found: " << pMap[field].toString() );
		}

		field = IS_IN_COUNTRY_JSON;
		if (pMap.contains(field) && pMap[field].canConvert<QString>()) {
			address.setCountry(pMap[field].toString());
			DBG_CM(SEARCH_M, INFO_L, "place country found: " << pMap[field].toString() );
		}

		field = WEBSITE_JSON;
		if (pMap.contains(field) && pMap[field].canConvert<QString>()) {
			landmark.setUrl(QUrl(pMap[field].toString()));
			DBG_CM(SEARCH_M, INFO_L, "place website found: " << pMap[field].toString());
		}

		field = ADDR_COUNTRY_JSON;
		if (pMap.contains(field) && pMap[field].canConvert<QString>()) {
			address.setCountryCode(pMap[field].toString());
			DBG_CM(SEARCH_M, INFO_L, "place countryCode found: " << pMap[field].toString());
		}

		field = ADDR_CITY_JSON;
		if (pMap.contains(field) && pMap[field].canConvert<QString>()) {
			address.setCity(pMap[field].toString());
			DBG_CM(SEARCH_M, INFO_L, "place city found: " << pMap[field].toString());
		}

		field = ADDR_STREET_JSON;
		if (pMap.contains(field) && pMap[field].canConvert<QString>()) {
			DBG_CM(SEARCH_M, INFO_L, "place street found: " << pMap[field].toString());
			if (pMap.contains(ADDR_HOUSENUMBER_JSON) && pMap[ADDR_HOUSENUMBER_JSON].canConvert<QString>()) {
				DBG_CM(SEARCH_M, INFO_L, "place housenumber found: " << pMap[ADDR_HOUSENUMBER_JSON].toString());
				address.setStreet(pMap[ADDR_HOUSENUMBER_JSON].toString() + " " + pMap[field].toString());
			}
			else
			{
				address.setStreet(pMap[field].toString());
			}
		}

		field = ADDR_POSTCODE_JSON;
		if (pMap.contains(field) && pMap[field].canConvert<QString>()) {
			address.setPostcode(pMap[field].toString());
			DBG_CM(SEARCH_M, INFO_L, "place postcode found: " << pMap[field].toString());
		}

		field = ADDR_DISTRICT_JSON;
		if (pMap.contains(field) && pMap[field].canConvert<QString>()) {
			address.setDistrict(pMap[field].toString());
			DBG_CM(SEARCH_M, INFO_L, "place district found: " << pMap[field].toString());
		}

		landmark.setAddress(address);
/*
		foreach (QString key, pMap.keys()) {

			if (pMap[key].canConvert<QString>()) {
                landmark.setCustomAttribute(key, pMap[key]);
			}

		}
*/
		return true;
	}
	else
	{
		DBG_CM(SEARCH_M, WARN_L, "parseProperties(): QVariantMap does not contain ""properties"" entry, or it is invalid");
		return false;
	}

}


bool QGeoCodeParser::parseLocation(const QVariantMap &map, QLandmark &landmark)
{
	if (map.contains(LOCATION_JSON) &&
		map[LOCATION_JSON].canConvert<QVariantMap>() ) {

		DBG_CM(SEARCH_M, INFO_L, "Start parsing location..");

		QVariantMap pMap = map[LOCATION_JSON].toMap();
		QGeoAddress address = landmark.address();
		QString field;


		field = COUNTY_JSON;
		if (pMap.contains(field) && pMap[field].canConvert<QString>()) {
			address.setCounty(pMap[field].toString());
			DBG_CM(SEARCH_M, INFO_L, "place county found: " << pMap[field].toString() );
		}

		field = COUNTRY_JSON;
		if (pMap.contains(field) && pMap[field].canConvert<QString>()) {
			address.setCountry(pMap[field].toString());
			DBG_CM(SEARCH_M, INFO_L, "place country found: " << pMap[field].toString() );
		}

		field = ROAD_JSON;
		if (pMap.contains(field) && pMap[field].canConvert<QString>()) {
			DBG_CM(SEARCH_M, INFO_L, "place road found: " << pMap[field].toString());

			// If we have housenumber - add it before road:
			QString houseNumber = "";
			if (map.contains(PROPERTIES_JSON) &&
				map[PROPERTIES_JSON].canConvert<QVariantMap>() ) {

				QVariantMap propMap = map[PROPERTIES_JSON].toMap();
				if (propMap.contains(ADDR_HOUSENUMBER_JSON) && propMap[ADDR_HOUSENUMBER_JSON].canConvert<QString>()) {
					houseNumber = propMap[ADDR_HOUSENUMBER_JSON].toString();
				}
			}

			if (houseNumber.length() > 0) {
				address.setStreet(houseNumber + " " + pMap[field].toString());
			}
			else
			{
				address.setStreet(pMap[field].toString());
			}
		}

		field = CITY_JSON;
		if (pMap.contains(field) && pMap[field].canConvert<QString>()) {
			address.setCity(pMap[field].toString());
			DBG_CM(SEARCH_M, INFO_L, "place city found: " << pMap[field].toString());
		}

		landmark.setAddress(address);
		return true;
	}
	else
	{
		return false;
	}
}


QList<QGeoPlace> QGeoCodeParser::results() const
{
    return m_results;
}

QString QGeoCodeParser::errorString() const
{
    return m_errorString;
}

bool QGeoCodeParser::parseBoundingBox(QGeoBoundingBox *bounds, const QVariant &jsonBBox)
{
	if ( !jsonBBox.canConvert<QVariantList>() ) {

		DBG_CM(SEARCH_M, WARN_L, "parseBoundingBox() can't convert json bounds to list");
		return false;
	}

	if ( jsonBBox.toList().size() != 2 ) {

		DBG_CM(SEARCH_M, WARN_L, "parseBoundingBox(): Bbox list size must be 2");
		return false;
	}

	QVariant bottomLeft = jsonBBox.toList().first();
	QVariant topRight = jsonBBox.toList().last();

	bounds->setBottomLeft(QGeoCoordinate(bottomLeft.toList().first().toDouble(), bottomLeft.toList().last().toDouble()));
	bounds->setTopRight(QGeoCoordinate(topRight.toList().first().toDouble(), topRight.toList().last().toDouble()));

    return true;
}
