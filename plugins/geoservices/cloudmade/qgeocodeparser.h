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

#ifndef QGEOCODEPARSER_H
#define QGEOCODEPARSER_H

#include <QString>
#include <QList>
#include <QVariant>

class QIODevice;

#include <qgeocoordinate.h>
#include <qgeoboundingbox.h>
#include <qgeoplace.h>
#include <qlandmark.h>
#include <qgeoaddress.h>

QTM_USE_NAMESPACE

#define FEATURES_JSON		"features"
#define BOUNDS_JSON			"bounds"
#define CENTROID_JSON		"centroid"
#define TYPE_JSON			"type"
#define COORDINATES_JSON	"coordinates"
#define POINT_JSON			"POINT"
#define PROPERTIES_JSON		"properties"
#define NAME_JSON			"name"
#define IS_IN_COUNTRY_JSON	"is_in:country"

#define WEBSITE_JSON		"website"


#define ADDR_COUNTRY_JSON	"addr:country"
#define ADDR_CITY_JSON		"addr:city"
#define ADDR_STREET_JSON	"addr:street"
#define ADDR_HOUSENUMBER_JSON	"addr:housenumber"
#define ADDR_POSTCODE_JSON	"addr:postcode"
#define ADDR_DISTRICT_JSON	"addr:district"

// Location:
#define LOCATION_JSON		"location"
#define COUNTY_JSON			"county"
#define COUNTRY_JSON		"country"
#define ROAD_JSON			"road"
#define CITY_JSON			"city"



class QGeoCodeParser
{
public:
    QGeoCodeParser();
    ~QGeoCodeParser();

    bool parse(QIODevice* source);

    QList<QGeoPlace> results() const;
    QString errorString() const;

private:
	bool parseBoundingBox(QGeoBoundingBox *bounds, const QVariant &jsonBBox);
	bool parseFeatures(QVariant &f);
	bool parseSingleFeature(const QVariant &f);

	bool parseCentroid(const QVariantMap &map, QLandmark &landmark);
	bool parseCoordinates(const QVariant &v, QGeoCoordinate &coord);
	bool parseBounds(const QVariantMap &map, QLandmark &landmark);
	bool parseProperties(const QVariantMap &map, QLandmark &landmark);
	bool parseLocation(const QVariantMap &map, QLandmark &landmark);

	void debugMap(QVariantMap &map);

    QList<QGeoPlace> m_results;
    QString m_errorString;
};

#endif
