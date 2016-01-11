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

#ifndef QROUTEJSONPARSER_H
#define QROUTEJSONPARSER_H

#include <QList>
#include <QMap>
#include <QString>

class QIODevice;

#include <qgeoroute.h>
#include <qgeoroutesegment.h>
#include <qgeocoordinate.h>
#include <qgeoboundingbox.h>
#include <qgeorouterequest.h>
#include <qgeomaneuver.h>

QTM_USE_NAMESPACE

class QGeoRouteJsonParser
{
public:
    QGeoRouteJsonParser(const QGeoRouteRequest &request);
    ~QGeoRouteJsonParser();

    bool parse(QIODevice* source);
    QList<QGeoRoute> results() const;
    QString errorString() const;

private:
    bool parseRouteGeometry(QList<QVariant> geometryList, QList<QGeoCoordinate> *geoPoints, QGeoBoundingBox *bounds);
    bool parseRouteInstructions(QList<QVariant> geometryList, QList<QGeoRouteSegment> *segments, QGeoRoute route);

    QGeoRouteRequest m_request;
    QList<QGeoRoute> m_results;
    QString m_errorString;
};

#endif
