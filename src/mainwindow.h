#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsView>
#include <QGeoServiceProvider>
#include <QMenu>
#include <QMenuBar>
#include <QNetworkReply>

#include "customwindow.h"
#include "locationprovider.h"
#include "mapwidget.h"
#include "cachelist.h"
#include "cacheprovider.h"
#include "waypointdb.h"
#include "tool.h"

class MainWindow : public CustomWindow {
  Q_OBJECT;

 public:
  MainWindow(LocationProvider *, QWidget *parent = 0);
  ~MainWindow();

 protected:
  void resizeEvent(QResizeEvent* event);
  
 public slots:
  void statusMessage(const QString &);
  // called from map widget whenever the viewport changed
  void reloadCaches();
  void reloadCaches(const int);
  // called from cacheprovider
  void updateCaches(const CacheList &);
  void displayError(const QString &);
  // called from map whenever the user clicked a cache icon
  void selectCache(const QString &);
  void detailWaypoint(const Waypoint &);
  void detailCache(const QString &);
  void setBusy(bool on);
  void aboutBox(QWidget *, const QString &, const QString &);
  void enableToolProjection(bool);
  void enableToolSearch(bool);
  void enableToolAuthorize(bool);
  void openUrl(const QUrl &);

 private slots:
  void search();
  void authorize();
  void motd();
  void about();
  void settingsdialog();
  void help();
  void aboutPlugins();
  void showDetail(const Cache &);
  void showCacheList(const CacheList &);
  void launchToolRot13();
  void launchToolFormula();
  void launchToolRoman();
  void launchToolProjection();
  void replyFinished(QNetworkReply*);
  void handleReply(const Params &);
  void handleReplyCache(const Params &, const Cache &);
  void handleReplyCacheList(const Params &, const CacheList &);
  void showSearchResultsOnMap();

 signals:
  void activated(bool);

 private:
  void createMenu();
  bool event(QEvent *);

  QGeoServiceProvider *m_serviceProvider;
  MapWidget *m_mapWidget;
  CacheProvider *m_cacheProvider;
  LocationProvider *m_locationProvider;
  QString m_message;   
  QGraphicsView *m_view;

  WaypointDb *m_wptDb;
  int m_lastFlags;

  QAction *m_ActionToolProjection;
  QAction *m_ActionToolSearch; 
  QAction *m_ActionToolAuthorize; 
  QNetworkAccessManager *m_manager;

  QString m_motd;
  int m_motdVersion;
  QAction *m_motdAction; 
};

#endif
