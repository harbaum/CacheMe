#ifndef MAP_WIDGET_H
#define MAP_WIDGET_H

#include <QSystemScreenSaver>

#include <QGraphicsGeoMap>
#include <QGeoMappingManager>
#include <QGeoMapPixmapObject>
#include <QGeoMapCircleObject>
#include <QGeoMapGroupObject>
#include <QGeoPositionInfo>
#include <QTimer>

#include "cachelist.h"
#include "iconloader.h"
#include "locationprovider.h"
#include "waypointdb.h"

#include "mapoverlay.h"
#include "mapbanner.h"
#include "mapspinner.h"
#include "mapbubble.h"

QTM_USE_NAMESPACE

// limit number of caches displayed to cope with performance issues on
// handhelds
#define CACHELIST_LIMIT 99

#ifdef Q_OS_SYMBIAN
#define WORKAROUND_CONTAINS
#endif

class MapWidget : public QGraphicsGeoMap {
  Q_OBJECT;

 public:
  MapWidget(QGeoMappingManager*);
  ~MapWidget();
  bool mapWindowIsActive();
  void updateCaches(const CacheList &);
  void updateZoomButtons();
  void hideBubble();
  QString managerName();
  void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget * = 0);
  bool hiRez() const;
  void setHiRez(bool);
  bool hideOwn() const;
  void setHideOwn(bool);
  void setWaypointDb(WaypointDb *);
  bool screenSaverDisabled() const;
  void setScreenSaverDisabled(bool);
  Cache selectedCache();
  QGeoCoordinate gpsCoordinate();
  bool preparingReload();
  void update(const QRectF & = QRectF());

 signals:
  void reload(const int);
  void cacheClicked(const QString &);
  void detailClicked(const QString &);
  void wptDetailClicked(const Waypoint &);
  void showMessage(const QString &);
  void fullscreen();
  void cacheIsSelected(bool);
  void manualUpdateClicked(const int);
  
 public slots:
  void zoomIn();
  void zoomOut();
  void toggleFullscreen();
  void gpsFollow();
  void showBubble(const Cache &);
  void showBubble(const Waypoint &);
  void showDetails();
  void setBusy(bool);
  void addBanner(const QString &);
  void windowActivated(bool);
  void enableUpdateButton(bool);
  void manualUpdate();
  void reCenter(const QGeoCoordinate &);

 private slots:
  void reloadCustomWaypoints();
  void reloadTimerExpired();
  void zoomLevelChangedEvent(qreal);
  void centerChangedEvent(const QGeoCoordinate &);
  void positionUpdated(const QGeoPositionInfo &);
  void positionUpdated(const QCompassReading *);
  void updateCaches();
  void reload();

 protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *);
  void resizeEvent(QGraphicsSceneResizeEvent *);
  void showEvent(QShowEvent *);
  void drawScale(QPainter *);
  
 private:
  class MapCacheObject : public QGeoMapPixmapObject {
  public:
    MapCacheObject(IconLoader *, const Cache &, bool);
#ifdef WORKAROUND_CONTAINS
    bool contains( const QGeoCoordinate &) const;
#endif
  };

  class MapWaypointObject : public QGeoMapPixmapObject {
  public:
    MapWaypointObject(IconLoader *, const QString &, const Waypoint &);
#ifdef WORKAROUND_CONTAINS
    bool contains( const QGeoCoordinate &) const;
#endif
    Waypoint waypoint() const;
  private:
    Waypoint m_wpt;
  };

  // this should actually be much lower on non-touch devices
  static const int DRAG_FUZZ = 30;

  QPointF m_downPos;
  IconLoader *m_iconLoader;
  MapOverlay *m_mapOverlay;
  MapBanner *m_mapBanner;
  MapSpinner *m_mapSpinner;
  QTimer *m_timer;
  MapBubble *m_bubble;
  QGeoBoundingBox m_currentViewport;
  bool m_dragging;
  bool m_posIsValid;
  bool m_gpsLocked;
  bool m_hiRez;
  bool m_hideOwn;
  bool m_disableScreensaver;
  int m_precisionAlpha;
  QString m_selectedCache;
  QList<Waypoint> m_selectedWaypoints;
  qreal m_compass;

  // make sure map is only updated if it's dragged
  // significantly
  QGeoCoordinate m_lastUpdateCenter;

  QGeoMapPixmapObject *m_gpsMarker;
  QGeoMapCircleObject *m_gpsPrecision;
  QGeoMapGroupObject *m_cacheGroup;
  QGeoMappingManager *m_manager;

  WaypointDb *m_wptDb;
  QList<Waypoint> m_customWpts;

  bool m_cacheListWasUpdated;
  CacheList m_cacheList2Update;
  QSystemScreenSaver *m_screenSaver;
  QGeoCoordinate m_newCenter;
  bool m_imperial;
};

#endif // MAP_WIDGET_H
