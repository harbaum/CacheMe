#ifndef CACHELISTWINDOW_H
#define CACHELISTWINDOW_H

#include <QGeoCoordinate>
#include <QListWidget>

#include "customwindow.h"
#include "cachelist.h"
#include "iconloader.h"
#include "cacheprovider.h"
#include "cacheproviderplugin.h"

class LocationProvider;
class WaypointDb;
class MapWidget;

class CacheListWindow : public CustomWindow {
  Q_OBJECT;

 public:
  enum SortOrder {
    SortByName, SortByMapDistance
  };

  CacheListWindow(const QGeoCoordinate &, const QString &, const CacheList &, 
		  LocationProvider *, WaypointDb *, MapWidget *, CacheProvider *,
		  QWidget * = 0);
  ~CacheListWindow();

 private slots:
  void handleReplyCache(const Params &, const Cache &);
  void selected(QListWidgetItem *);
  void sort(SortOrder);
  void showOnMap();

 signals:
  void entrySelected(const QString &);
  void requestShowOnMap();

 private:
  QListWidget *m_listWidget;
  CacheList m_cacheList;
  IconLoader m_iconLoader;
  QString m_name;
  QGeoCoordinate m_mapCenter;

  // info required to open the cache details window
  LocationProvider *m_locationProvider;
  WaypointDb *m_wptDb;
  MapWidget *m_mapWidget;
  CacheProvider *m_cacheProvider;
};

#endif
