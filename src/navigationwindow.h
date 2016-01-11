#ifndef NAVIGATIONWINDOW_H
#define NAVIGATIONWINDOW_H

#include <QSystemScreenSaver>

#include "locationprovider.h"
#include "customwindow.h"
#include "coordinatetool.h"
#include "waypointdb.h"
#include "cache.h"

class QLabel;
class InfoWidget;

class NavigationWindow : public CustomWindow {
  Q_OBJECT;

 public:
  NavigationWindow(const Cache &, LocationProvider *, 
		   WaypointDb *, MapWidget *, QWidget * = 0);
  NavigationWindow(const Waypoint &, LocationProvider *, 
		   MapWidget *, QWidget * = 0);
  ~NavigationWindow();
  QString distanceString(qreal distance);
  QString latitudeString(const QGeoCoordinate &);
  QString longitudeString(const QGeoCoordinate &);
  QString zeroCut(qreal, int, int);

 public slots:
  void setPosition(const QGeoPositionInfo &);
  void setTarget(const QGeoCoordinate &);
  void centerOnWpt();

 private:
  Cache m_cache;
  Waypoint  m_wpt;
  CoordinateTool *m_coordinateTool;
  InfoWidget *m_latitude, *m_longitude, *m_precision, *m_distance;
  QGeoCoordinate m_coo;
  QSystemScreenSaver m_screenSaver;
  MapWidget *m_mapWidget;
};

#endif
