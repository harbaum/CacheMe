#include <QDebug>

#include <QGeoServiceProvider>
#include <QGraphicsSceneMouseEvent>
#include <QGeoMapOverlay>
#include <QSettings>
#include <math.h>

#include "mapwidget.h"
#include "custompixmap.h"
#include "cacheprovider.h"
#include "config.h"

#if(QTM_VERSION < 0x010100)
#error "QtMobility version 1.1 or higher required!"
#endif

void MapWidget::updateZoomButtons() {
  this->m_mapOverlay->enableZoomButtons(
	zoomLevel() < maximumZoomLevel(),
	zoomLevel() > minimumZoomLevel());
  
  update();
}

void MapWidget::enableUpdateButton(bool on) {
  this->m_mapOverlay->enableUpdateButton(on);
  update();
}

void MapWidget::manualUpdate() {
  qDebug() << __FUNCTION__;

  emit reload(CacheProviderPlugin::Manual |
	      (m_hideOwn?(CacheProviderPlugin::NoFound|CacheProviderPlugin::NoOwned):CacheProviderPlugin::None));
}

void MapWidget::reCenter(const QGeoCoordinate &coo) {
  if(!mapWindowIsActive()) 
    m_newCenter = coo;
  else
    setCenter(coo);
}

MapWidget::MapWidget(QGeoMappingManager *mgr) : 
  QGraphicsGeoMap(mgr), m_bubble(NULL), m_posIsValid(false), m_gpsMarker(NULL),
  m_manager(mgr), m_wptDb(NULL), m_cacheListWasUpdated(false),
  m_screenSaver(NULL) {

  // check for imperial vs metric
  QLocale locale;
  m_imperial = (locale.measurementSystem() == QLocale::ImperialSystem);

  // try to get default from qsettings, otherwise start at berlin's zoo
  QSettings settings;
  settings.beginGroup("Map");
  setCenter(QGeoCoordinate(
		   settings.value("Center/Latitude", 52.514).toFloat(),
		   settings.value("Center/Longitude", 13.3611).toFloat()));
  setZoomLevel(settings.value("ZoomLevel", 15).toInt());
  setMapType((QGraphicsGeoMap::MapType)settings.value("Type", 
			      QGraphicsGeoMap::StreetMap).toInt());
  m_gpsLocked = settings.value("GpsLocked", false).toBool();
  m_hiRez = settings.value("HiRez", false).toBool();
  m_hideOwn = settings.value("HideOwn", false).toBool();
  m_disableScreensaver = settings.value("DisableScreenSaver", false).toBool();
  m_precisionAlpha = settings.value("PrecisionAlpha", 32).toInt();
  settings.endGroup();
  if(m_hiRez) m_iconLoader = new IconLoader(Config::MAP_WIDGET_ICON_SIZE_HIREZ);
  else        m_iconLoader = new IconLoader(Config::MAP_WIDGET_ICON_SIZE);

  if(m_disableScreensaver) {
    m_screenSaver = new QSystemScreenSaver();
    if(!m_screenSaver->setScreenSaverInhibit())
      qDebug() << __FUNCTION__ << "disabling the screensaver failed!";
  }

  // create the control overlays
  this->m_mapOverlay = new MapOverlay(this);
  this->m_mapBanner = new MapBanner(this);
  this->m_mapSpinner = NULL;
  
  // and update it to reflect restored map state
  updateZoomButtons();
  if(m_gpsLocked)
    this->m_mapOverlay->changeGpsButton(true);
  
  // create a group for the cache icons
  m_cacheGroup = new QGeoMapGroupObject();
  m_cacheGroup->setZValue(0);
  addMapObject(m_cacheGroup);
  
  // create a gps marker and precision indicator, but hide them
  m_compass = NAN;
  QPixmap *markerIcon = m_iconLoader->load("gps_marker");
  if(markerIcon) {
    m_gpsMarker = new QGeoMapPixmapObject(QGeoCoordinate(0,0), 
		  QPoint(-m_iconLoader->size()/2, -m_iconLoader->size()/2),
		  *markerIcon);

    m_gpsMarker->setZValue(3);
    m_gpsMarker->setVisible(false);
    addMapObject(m_gpsMarker);
  }
  
  m_gpsPrecision = new QGeoMapCircleObject();
  m_gpsPrecision->setPen(QPen((QColor(0, 0, 0, m_precisionAlpha))));
  m_gpsPrecision->setBrush(QBrush(QColor(0, 0, 0, m_precisionAlpha/2)));
  m_gpsPrecision->setZValue(2);
  addMapObject(m_gpsPrecision);

  // install timer to delay cache reload requests
  this->m_timer = new QTimer(this);
  this->m_timer->setSingleShot(true);
  QObject::connect(this->m_timer, SIGNAL(timeout()),
		   this, SLOT(reloadTimerExpired()));
  
  // connect to map signals to be able to handle viewport changes
  QObject::connect(this, SIGNAL(centerChanged(const QGeoCoordinate &)),
		   this, SLOT(centerChangedEvent(const QGeoCoordinate &)));
  QObject::connect(this, SIGNAL(zoomLevelChanged(qreal)),
		   this, SLOT(zoomLevelChangedEvent(qreal)));
}

MapWidget::~MapWidget() {
  qDebug() << __FUNCTION__;

  // save all kinds of map settings
  QSettings settings;
  settings.beginGroup("Map");
  settings.setValue("Center/Latitude", center().latitude());
  settings.setValue("Center/Longitude", center().longitude());
  settings.setValue("ZoomLevel", zoomLevel());
  settings.setValue("GpsLocked", m_gpsLocked);  
  settings.setValue("PrecisionAlpha", m_precisionAlpha);
  settings.endGroup();

  delete m_iconLoader;
  delete m_timer;
  delete m_mapOverlay;
  delete m_mapBanner;

  if(this->m_mapSpinner) 
    delete m_mapSpinner;
}

// ------------ slots to handle overlay button clicks --------------
void MapWidget::zoomIn() {
  if(zoomLevel() < maximumZoomLevel()) {
    setZoomLevel(zoomLevel()+1);
    updateZoomButtons();
  } 
}

/* never call this method directly as it is bypassed on */
/* e.g. maemo5. use emit showMessage() instead */
void MapWidget::addBanner(const QString &msg) {
  this->m_mapBanner->message(msg);
}

void MapWidget::zoomOut() {
  if(zoomLevel() > minimumZoomLevel()) {
    setZoomLevel(zoomLevel()-1);
    updateZoomButtons();
  }
}

void MapWidget::gpsFollow() {
  m_gpsLocked = true;
  this->m_mapOverlay->changeGpsButton(true);  
  update();
}

void MapWidget::toggleFullscreen() {
  emit fullscreen();
}

void MapWidget::resizeEvent(QGraphicsSceneResizeEvent *event) {
  QGraphicsGeoMap::resizeEvent(event);
  reload();
}

void MapWidget::showEvent(QShowEvent *) {
  reload();
}

void MapWidget::zoomLevelChangedEvent(qreal) {
  reload();
}

void MapWidget::centerChangedEvent(const QGeoCoordinate &newCenter) {

  if(!m_lastUpdateCenter.isValid()) {
    reload();
    return;
  }

  QPointF lastScreenPos(coordinateToScreenPosition(m_lastUpdateCenter));
  QPointF newScreenPos(coordinateToScreenPosition(newCenter));

  // if either horizontal or vertical moved more than 10% of screen, then
  // update
  if(fabs(lastScreenPos.x() - newScreenPos.x()) > size().width()/10 ||
     fabs(lastScreenPos.y() - newScreenPos.y()) > size().height()/10) {
    reload();
  }
}

void MapWidget::hideBubble() {
  // kill any existing bubble
  if(m_bubble) {
    removeMapObject(m_bubble);
    delete m_bubble;
    m_bubble = NULL;

    emit cacheIsSelected(false);
  }
}

void MapWidget::showBubble(const Cache &cache) {
  qDebug() << __FUNCTION__ << cache.name();

  hideBubble();
  m_bubble = new MapBubble(this, size().width() > size().height(), cache);
  addMapObject(m_bubble);

  bool isValid = cache.url().isValid();
  emit cacheIsSelected(isValid);
}

void MapWidget::showBubble(const Waypoint &wpt) {
  qDebug() << __FUNCTION__ << wpt.name();

  hideBubble();
  m_bubble = new MapBubble(this, size().width() > size().height(), wpt);
  addMapObject(m_bubble);

  emit cacheIsSelected(false);
}

Cache MapWidget::selectedCache() {
  Q_ASSERT(m_bubble);

  return m_bubble->cache();
}

// currently connected to the "forward" button
void MapWidget::showDetails() {
  Q_ASSERT(m_bubble);

  // bubble may either be a cache or a waypoint one
  if(m_bubble->cache().coordinate().isValid())
    emit detailClicked(m_bubble->cache().name());
  else
    emit wptDetailClicked(m_bubble->wpt());
}

void MapWidget::mousePressEvent(QGraphicsSceneMouseEvent* event) {
  qDebug() << __FUNCTION__;

  if(this->m_mapOverlay->mousePress(event->pos()))
    update();

  if(this->m_mapOverlay->isInside(event->pos()))
    m_downPos = QPointF(0,0);
  else
    m_downPos = event->pos();

  m_dragging = false;
}

void MapWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
  if(this->m_mapOverlay->mouseRelease(event->pos()))
    update();

  if(m_dragging && m_gpsLocked) {
    m_gpsLocked = false;
    this->m_mapOverlay->changeGpsButton(false);
    update();
  }

  // this doesn't work on n900 as the n900 port of qt mobility fails
  // at detecting clicks onto bitmaps
  if(m_downPos.isNull() || m_dragging)
    return;

  // check if we clicked a cache
  bool clickedBubble = false;
  QString clickedCache;
  Waypoint clickedWaypoint;

  QList<QGeoMapObject*> objects = mapObjectsAtScreenPosition(m_downPos);
  if (objects.length() > 0) {
    for (int i = objects.length()-1; i >= 0; i--) {
      if ((objects[i]->objectName() == "cache") && clickedCache.isEmpty())
	clickedCache = objects[i]->property("name").toString();
      if ((objects[i]->objectName() == "waypoint") && clickedCache.isEmpty()) {
	clickedCache = objects[i]->property("cache").toString();
	clickedWaypoint = 
	  static_cast<MapWaypointObject *>(objects[i])->waypoint();
      }

      if (objects[i]->objectName() == "bubble") 
	clickedBubble = true;
    }
  }

  if(clickedBubble)
    showDetails();
  else {
    hideBubble();

    if(!clickedCache.isEmpty()) {
      m_selectedCache = clickedCache;

      // update list of custom waypoints for the current cache
      if(m_wptDb)
	m_customWpts =  m_wptDb->get(clickedCache);
     
      /* update caches to show waypoints of selected cache if present */
      emit reload(m_hideOwn?(CacheProviderPlugin::NoFound|CacheProviderPlugin::NoOwned):CacheProviderPlugin::None);
 
      if(!clickedWaypoint.coordinate().isValid()) 
    	emit cacheClicked(clickedCache);
      else {
	qDebug() << __FUNCTION__ << "User clicked waypoint" <<
	  clickedWaypoint.name() << "of cache" << clickedCache;

	showBubble(clickedWaypoint);
      }
    }
  }
}

void MapWidget::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
  if(this->m_mapOverlay->mouseMove(event->pos()))
    update();

  if(m_downPos.isNull())
    return;

  if(!m_dragging) {
    if((m_downPos - event->pos()).manhattanLength() < DRAG_FUZZ) 
      return;

    // dragged more than DRAG_FUZZ pixels: start actual drag
    m_dragging = true;
    QPointF dp(m_downPos - event->pos());
    pan(dp.x(), dp.y());
    return;
  }

  QPointF d(event->lastPos() - event->pos());
  pan(d.x(), d.y());
}

MapWidget::MapCacheObject::MapCacheObject(IconLoader *loader, const Cache &cache, bool highlight) :
  QGeoMapPixmapObject(cache.coordinate()) {

  CustomPixmap *pix = 
    highlight?loader->loadHighlight(cache):loader->load(cache);
  setOffset(pix->offset());
  setPixmap(*pix);

  setZValue(0);  // below gps marker and bubble
  setObjectName("cache");
  setProperty("name", cache.name());
};

#ifdef WORKAROUND_CONTAINS
// the pixmap object seems to actually check for clicks onto the bitmap. thus
// we only check for the click being withing the entire pixmap size. furthermore
// the pixmap precise stuff doesn't seem to work at all on the n900/maemo
bool MapWidget::MapCacheObject::contains(const QGeoCoordinate &coo) const {
  return boundingBox().contains(coo);
}
#endif

MapWidget::MapWaypointObject::MapWaypointObject(IconLoader *loader, 
		const QString &cacheName, const Waypoint &waypoint) :
  QGeoMapPixmapObject(waypoint.coordinate()) {

  // set icon
  CustomPixmap *pix = 
    loader->load(waypoint.iconFile(), 2*loader->size()/3); 

  setPixmap(*pix);
  setOffset(pix->offset());
  
  setZValue(1);  // below gps marker and bubble, but above caches
  setObjectName("waypoint");
  setProperty("cache", cacheName);
  m_wpt = waypoint;
};

Waypoint MapWidget::MapWaypointObject::waypoint() const {
  return m_wpt;
}

#ifdef WORKAROUND_CONTAINS
bool MapWidget::MapWaypointObject::contains(const QGeoCoordinate &coo) const {
  return boundingBox().contains(coo);
}
#endif

#include <QGraphicsScene>
#include <QGraphicsView>

void MapWidget::updateCaches() {
  // are there pending cachelist updates?
  if(m_cacheListWasUpdated) {
    qDebug() << __FUNCTION__ << "replaying update";
    updateCaches(m_cacheList2Update);
    m_cacheListWasUpdated = false;
    m_cacheList2Update.clear();
  }

  if(m_newCenter.isValid()) {
    setCenter(m_newCenter);
    m_newCenter = QGeoCoordinate();
  }
}

void MapWidget::windowActivated(bool activated) {
  if(activated) {
    qDebug() << __FUNCTION__;

    // emit one shot timer
    QTimer::singleShot(200, this, SLOT(updateCaches()));
  }
}

bool MapWidget::mapWindowIsActive() {
  // try to get access to parent window
  bool windowIsActive = false;
  
  if(this->scene()) 
    if(this->scene()->views().size() == 1) 
      if(this->scene()->views()[0]) 
	if(this->scene()->views()[0]->window()) 
	  windowIsActive = 
	    this->scene()->views()[0]->window()->isActiveWindow();

  return windowIsActive;
}

void MapWidget::updateCaches(const CacheList &cacheListIn) {
  static bool tooOld = false;

  if(!mapWindowIsActive()) {
    qDebug() << __FUNCTION__ << "delaying update";
    m_cacheListWasUpdated = true;
    m_cacheList2Update = cacheListIn;
    return;
  }

  // create a local non-const copy as me may have to sort it
  CacheList cacheList(cacheListIn);  

  // save cache if existing bubble
  Cache bubbleCache;
  Waypoint bubbleWaypoint;
  if(m_bubble) {
    bubbleCache = m_bubble->cache();
    bubbleWaypoint = m_bubble->wpt();
  }
  hideBubble();

  // remove all existing caches
  m_cacheGroup->clearChildObjects();

  // determine cachelist age
  int weekAge = cacheList.date().daysTo(QDate::currentDate())/7;
  if(!cacheList.date().isValid()) weekAge = 0;

  if(!tooOld && weekAge > 0) {
    emit showMessage(tr("Displayed cache data is %1 weeks old!").arg(weekAge));
    tooOld = true;
  } else if(tooOld && weekAge == 0)
    tooOld = false;

  if(cacheList.size() > CACHELIST_LIMIT) {
    emit showMessage(tr("Limiting number of displayed caches"));

    qDebug() << __FUNCTION__ << "limiting cache list of" << 
      cacheList.size() << "to" << CACHELIST_LIMIT;

    // remove everything not on screen
    cacheList.clip(viewport());
    qDebug() << __FUNCTION__ << "size after clipping" << cacheList.size();

    if(cacheList.size() > CACHELIST_LIMIT) {
      // sort with respect to map center
      cacheList.sortByDistance(center());
    }
  }

  // draw all new caches
  QList<Cache>::const_iterator i;
  int j;
  for( j=0, i = cacheList.begin(); j < CACHELIST_LIMIT && i != cacheList.end(); ++j, ++i ) {
    // show if nno found/owned are to be hidden or if the cache is neither found nor owned 
    if(!m_hideOwn || (!i->owned().isSetAndTrue() && !i->found().isSetAndTrue())) {

      bool isSelected = (i->name() == m_selectedCache);

      // highlight if cache has waypoints
      m_cacheGroup->addChildObject(new MapCacheObject(m_iconLoader, *i, 
	      isSelected && ((i->waypoints().size() > 0) || (m_customWpts.size() > 0))));
      
      // also draw waypoints of selected cache
      if(isSelected) 
	m_selectedWaypoints = i->waypoints();
    }
  }

  // draw all waypoints from list
  foreach(Waypoint wpt, m_selectedWaypoints)
    m_cacheGroup->addChildObject(new MapWaypointObject(m_iconLoader, m_selectedCache, wpt));

  // draw all custom waypoints
  foreach(Waypoint wpt, m_customWpts)
    m_cacheGroup->addChildObject(new MapWaypointObject(m_iconLoader, m_selectedCache, wpt));

  // restore previous bubble
  if(bubbleCache.coordinate().isValid()) {
    // The ugly part: gc.com is randomizing coordinates and
    // the new coordinates used to draw the icon are likely
    // different from the ones saved from the previous bubble
    // position. So we search for the bubble cache within the
    // new cache list and use it's coordinates instead
    
    bool coordinateUpdated = false;
    for( i = cacheList.begin(); i != cacheList.end(); ++i ) {
      if(i->name() == bubbleCache.name()) {
	bubbleCache.setCoordinate(i->coordinate());
	coordinateUpdated = true;
      }
    }

    // only redraw bubble if the cache is still part of the
    // new cache list and if it's cache is still on-screen
    if(coordinateUpdated && 
       viewport().contains(bubbleCache.coordinate()))
      showBubble(bubbleCache);
  }

  if(bubbleWaypoint.coordinate().isValid())
    showBubble(bubbleWaypoint);
}

void MapWidget::reloadTimerExpired() {
  qDebug() << __FUNCTION__;

  // check if viewport actually changed
  if(m_currentViewport != viewport()) {
    m_lastUpdateCenter = center();

    emit reload(m_hideOwn?(CacheProviderPlugin::NoFound|CacheProviderPlugin::NoOwned):CacheProviderPlugin::None);
    m_currentViewport = viewport();
  }
}

bool MapWidget::preparingReload() {
  return this->m_timer->isActive();
}

void MapWidget::reload() {
  // start the timer if it's not already running, re-start from zero otherwise
  // using less than 1000ms here makes sure that requests are being sent for
  // every updated gps position (reported every 1000ms)
  if(!this->m_timer->isActive())
    this->m_timer->start(900);
  else
    this->m_timer->setInterval(900);
}

void MapWidget::positionUpdated(const QCompassReading *reading) {
  // don't do anything if map is not visible (especially to deal with#
  // buggy symbian qt versions)
  if(!mapWindowIsActive()) 
    return;

  // we accept 90% calibration
  if(reading->calibrationLevel() > 0.9) {
    qreal newVal = reading->azimuth();
    if(newVal != m_compass) {
      QPixmap *markerIcon = m_iconLoader->load("gps_arrow", 0, newVal);

      if(m_gpsMarker) m_gpsMarker->setPixmap(*markerIcon);
      m_compass = newVal;
    }
  } else {
    if(!isnan(m_compass)) {
      QPixmap *markerIcon = m_iconLoader->load("gps_marker");
      if(m_gpsMarker) m_gpsMarker->setPixmap(*markerIcon);

      m_compass = NAN;
    }
  }
}

void MapWidget::positionUpdated(const QGeoPositionInfo &pos) {
  // don't do anything if map is not visible (especially to deal with#
  // buggy symbian qt versions)
  if(!mapWindowIsActive()) 
    return;

  // change state of gps button to reflect gps availability
  if(m_posIsValid != pos.coordinate().isValid()) {
    this->m_mapOverlay->enableGpsButton(pos.coordinate().isValid());
    update();
    m_posIsValid = pos.coordinate().isValid();
  }

  // re-center map if gpsLock is active
  if(pos.coordinate().isValid() && m_gpsLocked && !m_dragging) 
    setCenter(pos.coordinate());

  // draw gps marker
  if(m_gpsMarker) {
    if(pos.coordinate().isValid()) {
      m_gpsMarker->setVisible(true);
      m_gpsMarker->setCoordinate(pos.coordinate());

      // horizontal accuracy, will return -1 if unset
      // and circle will be hidden if radius < 0
      m_gpsPrecision->setRadius(pos.attribute(QGeoPositionInfo::HorizontalAccuracy));
      m_gpsPrecision->setCenter(pos.coordinate());
    } else {
      m_gpsMarker->setVisible(false);
      m_gpsPrecision->setRadius(-1);
    }
  }
}

QString MapWidget::managerName() {
  return m_manager->managerName();
}

QGeoCoordinate MapWidget::gpsCoordinate() {
  if(!m_gpsMarker || !m_gpsMarker->isVisible()) 
    return QGeoCoordinate();

  return m_gpsMarker->coordinate();
}

void MapWidget::drawScale(QPainter *painter) {
  int start_x = 28;
  int start_y = size().height() - 20;
  int lenght = 62;
  int multiplier = 5;
  int height = 10;
  
  // m_imperial

  // check for visible width
  qreal width = viewport().bottomLeft().distanceTo(viewport().bottomRight());
  qDebug() << __FUNCTION__ << "map width in meter = " << width;

  // try to find next smaller power of 10
  int pten = log(width) / log(10);
  qDebug() << __FUNCTION__ << pow(10, pten);

  //draw zoom scale
  painter->setBrush(Qt::transparent);
  painter->setPen(QPen(QBrush(Qt::black), 2));
  painter->drawRect (QRect(start_x, start_y , lenght * multiplier, height)); //frame
  
  //inserts
  painter->setBrush(Qt::black );
  painter->setPen(QPen(QBrush(Qt::black), 1));
  painter->drawRect (QRect(start_x, start_y , lenght, height));
  painter->drawRect (QRect(start_x + 2*lenght, start_y , lenght, height));
  painter->drawRect (QRect(start_x + 4*lenght, start_y , lenght, height));
  
  QFont font("Arial");
  font.setPixelSize(20);
  font.setWeight(QFont::Bold);
  painter->setFont(font);
  
  QString txt;
  int scale = 125 * pow(2, 18 - zoomLevel());
  if(zoomLevel() > 15) {
    txt = " m";
    txt.prepend(QString::number(scale));
  }
  else{
    txt = " km";
    txt.prepend(QString::number(scale/1000));
  }
  //qDebug() << "Zoom level:" << zoomLevel() << ", 5 bar = " << txt;
  painter->drawText(start_x + lenght * multiplier + 10, start_y - 7, 100, 40, 0, txt);
}

void MapWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget) {
  // first call parents paint function
  QGraphicsGeoMap::paint(painter, option, widget);

#if 0
  // only show scale when we are above zoom level 5 since below it is 
  // not exact enough due the curvature of the earth  
  if(zoomLevel() > 5) 
    drawScale(painter);
#endif

  // then draw our overlay on top
  this->m_mapOverlay->paint(painter);
  this->m_mapBanner->paint(painter);

  if(this->m_mapSpinner)
    this->m_mapSpinner->paint(painter);
}

void MapWidget::setBusy(bool on) {
  if(on) {
    Q_ASSERT(!m_mapSpinner);
    m_mapSpinner = new MapSpinner(this);
  } else {
    Q_ASSERT(m_mapSpinner);
    delete m_mapSpinner;
    m_mapSpinner = NULL;
  }
}

bool MapWidget::hiRez() const {
  return m_hiRez;
}

void MapWidget::setHiRez(bool hiRez) {
  if(m_hiRez != hiRez) {
    m_hiRez = hiRez;

    delete m_iconLoader;
    if(m_hiRez) 
      m_iconLoader = new IconLoader(Config::MAP_WIDGET_ICON_SIZE_HIREZ);
    else
      m_iconLoader = new IconLoader(Config::MAP_WIDGET_ICON_SIZE);

    // this will also cause the overlay to redraw
    emit reload(m_hideOwn?(CacheProviderPlugin::NoFound|CacheProviderPlugin::NoOwned):CacheProviderPlugin::None);
  }
}

bool MapWidget::hideOwn() const {
  return m_hideOwn;
}

void MapWidget::setHideOwn(bool hideOwn) {
  qDebug() << __FUNCTION__;

  if(m_hideOwn != hideOwn) {
    m_hideOwn = hideOwn;

    // this will also cause the overlay to redraw
    emit reload(m_hideOwn?(CacheProviderPlugin::NoFound|CacheProviderPlugin::NoOwned):CacheProviderPlugin::None);
  }
}

bool MapWidget::screenSaverDisabled() const {
  return m_disableScreensaver;
}

void MapWidget::setScreenSaverDisabled(bool disableScreensaver) {
  qDebug() << __FUNCTION__;

  if(m_disableScreensaver != disableScreensaver) {
    m_disableScreensaver = disableScreensaver;

    // Who the f..k came up with this stupid screen saver API?
    if(m_disableScreensaver) {
      Q_ASSERT(!m_screenSaver);
      m_screenSaver = new QSystemScreenSaver();
      if(!m_screenSaver->setScreenSaverInhibit())
	qDebug() << __FUNCTION__ << "disabling the screensaver failed!";
    } else {
      Q_ASSERT(m_screenSaver);
      delete m_screenSaver;
      m_screenSaver = NULL;
    }
  }
}

void MapWidget::reloadCustomWaypoints() {
  // waypointsDb has signalled that the waypoints have changed
  bool hadCustomWpts = (m_customWpts.size() != 0);

  if(m_wptDb && !m_selectedCache.isEmpty()) {
    m_customWpts =  m_wptDb->get(m_selectedCache);

    // if there weren't any custom waypoints and there now aren't
    // we have nothing to redraw
    if((m_customWpts.size() != 0) || hadCustomWpts) 
      emit reload(m_hideOwn?(CacheProviderPlugin::NoFound|CacheProviderPlugin::NoOwned):
		  CacheProviderPlugin::None);
  }
}

void MapWidget::setWaypointDb(WaypointDb *wptDb) {
  qDebug() << __FUNCTION__;
  m_wptDb = wptDb;

  QObject::connect(this->m_wptDb, SIGNAL(waypointsChanged()),
		   this, SLOT(reloadCustomWaypoints()));
}

void MapWidget::update(const QRectF &rect) {
  // don't update in background, symbian belle doesn't like that
  if(!mapWindowIsActive())
    return;

  QGraphicsGeoMap::update(rect);
}
