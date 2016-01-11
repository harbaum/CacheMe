#ifndef CACHEWINDOW_H
#define CACHEWINDOW_H

#include "locationprovider.h"
#include "customwindow.h"
#include "cache.h"
#include "iconloader.h"
#include "htmlview.h"
#include "mapwidget.h"
#include "waypointdb.h"
#include "cacheprovider.h"

class CacheWindow : public CustomWindow {
  Q_OBJECT;

 public:
  CacheWindow(const Cache &, LocationProvider *, WaypointDb *, MapWidget *, CacheProvider *, QWidget * = 0);
  ~CacheWindow();

 private slots:
  void showDescription();
  void showNavigation();
  void showHint();
  void showLogs();
  void showAttributes();
  void reload();

 private:
  Cache m_cache;
  IconLoader m_iconLoader;
  LocationProvider *m_locationProvider;
  WaypointDb *m_wptDb;
  MapWidget *m_mapWidget;
  CacheProvider *m_cacheProvider;
};

#endif
