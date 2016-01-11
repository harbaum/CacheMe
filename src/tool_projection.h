#ifndef TOOL_PROJECTION_H
#define TOOL_PROJECTION_H

#include "coordinatetool.h"
#include "waypointdb.h"
#include "cache.h"
#include "tool.h"
#include "infowidget.h"
#include "distancewidget.h"
#include "directionwidget.h"

class ToolProjection: public Tool {
  Q_OBJECT;

 public:
  ToolProjection(MapWidget *, WaypointDb *, QWidget * = 0);
  ~ToolProjection();

  static  QAction *registerMenu(QMenu *, QObject *, const char *);
  
 private slots:
  void distanceChanged(qreal);
  void directionChanged(qreal);
  void coordinateChanged(const QGeoCoordinate &);
  void saveWaypoint();

 private:
  void updateTarget();

  WaypointDb *m_wptDb;
  InfoWidget *m_latitude, *m_longitude;
  CoordinateTool *m_coordinateTool;
  DistanceWidget *m_distanceWidget;
  DirectionWidget *m_directionWidget;
  Waypoint m_wpt;
};

#endif // TOOL_PROJECTION_H
