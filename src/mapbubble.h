#ifndef MAPBUBBLE_H
#define MAPBUBBLE_H

#include <QGraphicsGeoMap>
#include <QGeoMapPixmapObject>

#include "iconloader.h"
#include "cache.h"

QTM_USE_NAMESPACE

class MapBubble : public QGeoMapPixmapObject {
  Q_OBJECT;

 public:
  MapBubble(const QGraphicsGeoMap *, bool, const Cache &);
  MapBubble(const QGraphicsGeoMap *, bool, const Waypoint &);
  Cache cache() const;
  Waypoint wpt() const;

 private:
  qreal calculateTipOffset(const QGraphicsGeoMap *, const QGeoCoordinate &, const QSize &);
  Cache m_cache;
  Waypoint m_wpt;
};

#endif // MAPBUBBLE_H
