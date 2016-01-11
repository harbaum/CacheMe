/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
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

#include "qgeomappingmanagerengine_osm.h"
#include "qgeomapreply_osm.h"

#include <qgeotiledmaprequest.h>

#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkProxy>
#include <QSize>
#include <QDir>
#include <QDateTime>
#include <QApplication>
#include <QFileInfo>

#if (QTM_VERSION >= QTM_VERSION_CHECK(1, 2, 2))
#define TRANSITMAP  QGraphicsGeoMap::TransitMap
#else
#define TRANSITMAP  QGraphicsGeoMap::SatelliteMapDay
#endif

QGeoMappingManagerEngineOsm::QGeoMappingManagerEngineOsm(const QMap<QString, QVariant> &parameters, QGeoServiceProvider::Error *error, QString *errorString)
        : QGeoTiledMappingManagerEngine(parameters),
        m_parameters(parameters)
{
    Q_UNUSED(error)
    Q_UNUSED(errorString)

    setTileSize(QSize(256,256));
    setMinimumZoomLevel(0.0);
    setMaximumZoomLevel(18.0);
    m_tyleStyleId = "1";

    QList<QGraphicsGeoMap::MapType> types;
    types << QGraphicsGeoMap::StreetMap;
    types << QGraphicsGeoMap::TerrainMap;  // used for cyclemap
    types << TRANSITMAP;

    setSupportedMapTypes(types);

    m_nam = new QNetworkAccessManager(this);

#ifdef USE_NETWORK_CACHE
    m_cache = new QNetworkDiskCache(this);

    QDir dir = QDir::temp();
    dir.mkdir("maptiles-osm");
    dir.cd("maptiles-osm");

    m_cache->setCacheDirectory(dir.path());
#endif

    QList<QString> keys = m_parameters.keys();

    if (keys.contains("mapping.proxy")) {
        QString proxy = m_parameters.value("mapping.proxy").toString();
        if (!proxy.isEmpty())
            m_nam->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy, 8080));
    }

#ifdef USE_NETWORK_CACHE
    if (keys.contains("mapping.cache.directory")) {
        QString cacheDir = m_parameters.value("mapping.cache.directory").toString();
        if (!cacheDir.isEmpty())
            m_cache->setCacheDirectory(cacheDir);
    }

    if (keys.contains("mapping.cache.size")) {
        bool ok = false;
        qint64 cacheSize = m_parameters.value("mapping.cache.size").toString().toLongLong(&ok);
        if (ok)
            m_cache->setMaximumCacheSize(cacheSize);
    }

    m_nam->setCache(m_cache);
#endif
}

QGeoMappingManagerEngineOsm::~QGeoMappingManagerEngineOsm() {}

QGeoTiledMapReply* QGeoMappingManagerEngineOsm::getTileImage(const QGeoTiledMapRequest &request)
{
   QString rawRequest = getRequestString(request);

    //    qDebug() << "SL:" << rawRequest;

    QNetworkRequest netRequest = QNetworkRequest(QUrl(rawRequest));
    netRequest.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    QString ua = QFileInfo(QApplication::applicationFilePath()).fileName();
    ua.remove(QChar('"'), Qt::CaseInsensitive);
    ua += " (Qt";
    ua += qVersion();
    ua += " QtMobility 1.1 ) osm GeoSearchManager";
    netRequest.setRawHeader("User-Agent", ua.toAscii());
#ifdef USE_NETWORK_CACHE
    m_cache->metaData(netRequest.url()).setLastModified(QDateTime::currentDateTime());
#endif

    QNetworkReply* netReply = m_nam->get(netRequest);

    QGeoTiledMapReply* mapReply = new QGeoMapReplyOsm(netReply, request, this);

    return mapReply;
}

QString QGeoMappingManagerEngineOsm::getRequestString(const QGeoTiledMapRequest &request) const
{
  QString host("tile.openstreetmap.org");

  if(request.mapType() == QGraphicsGeoMap::TerrainMap)
    host = "tile.opencyclemap.org/cycle";

  if(request.mapType() == TRANSITMAP)
    host = "tile.xn--pnvkarte-m4a.de/tilegen";

  return QString("http://" + host + "/" + 
		 QString::number(request.zoomLevel()) + "/"+
		 QString::number(request.column()) + "/" +
		 QString::number(request.row()) + ".png");
}

void QGeoMappingManagerEngineOsm::setOption(/*QString optionName, QString optionValue*/)
{
    // Options recognized in child classed  (override this method)
    m_tyleStyleId = "2";

    /*if (optionName == "tyleStyleId")
    {
        m_tyleStyleId = optionValue;
        return 0;   // success!  TBD: rework return codes
    }
    else
        return -1; */
}

