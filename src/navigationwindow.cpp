#include <QDebug>

#include <QLocale>
#include <QLabel>
#include <QGroupBox>
#include <math.h>

#include "navigationwindow.h"
#include "compass.h"
#include "swapbox.h"
#include "infowidget.h"

void NavigationWindow::centerOnWpt() {
  qDebug() << __FUNCTION__ << m_coordinateTool->coordinate();

  m_mapWidget->reCenter(m_coordinateTool->coordinate());
}

NavigationWindow::NavigationWindow(const Waypoint &wpt, 
	   LocationProvider *locationProvider, 
	   MapWidget *mapWidget, QWidget *parent) : 
  CustomWindow("NavigationWindow", parent),
  m_coordinateTool(NULL), m_coo(wpt.coordinate()), m_mapWidget(mapWidget) {

  setWindowTitle(tr("Navigation"));

  SwapBox *sbox = new SwapBox();

  if(!m_screenSaver.setScreenSaverInhibit())
    qDebug() << __FUNCTION__ << "disabling the screensaver failed!";

  // try to force the compass to be at least as wide
  // as the infos on the right (bottom)

  // left/top: the compass
  Compass *compass = new Compass(wpt, this);
  sbox->addWidget(compass);

  // right/bottom: the textual info
  QWidget *vbox = new QWidget(compass);
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(0);

#ifdef Q_OS_SYMBIAN
  QWidget *groupBox = new QWidget();
#else
  QGroupBox *groupBox = new QGroupBox();
#endif
  QVBoxLayout *ivbox = new QVBoxLayout;
  m_latitude = new InfoWidget(tr("Latitude"));
  ivbox->addWidget(m_latitude);
  m_longitude = new InfoWidget(tr("Longitude"));
  ivbox->addWidget(m_longitude);
  m_precision = new InfoWidget(tr("Precision"));
  ivbox->addWidget(m_precision);
  groupBox->setLayout(ivbox);
  layout->addWidget(groupBox);

  layout->addStretch();

#ifdef Q_OS_SYMBIAN
  groupBox = new QWidget();
#else
  groupBox = new QGroupBox();
#endif
  ivbox = new QVBoxLayout;
  QLabel *label = new QLabel("<b>" + wpt.description() + "</b>");
  QSizePolicy sp1(QSizePolicy::Ignored, QSizePolicy::Fixed);
  label->setSizePolicy(sp1);
  ivbox->addWidget(label);

  InfoWidget *latitude = new InfoWidget(tr("Latitude"), 
	CoordinateTool::latitudeString(wpt.coordinate()), this);
  ivbox->addWidget(latitude);
  InfoWidget *longitude = new InfoWidget(tr("Longitude"),
	CoordinateTool::longitudeString(wpt.coordinate()), this);
  ivbox->addWidget(longitude);
  m_distance = new InfoWidget(tr("Distance"));
  ivbox->addWidget(m_distance);
  groupBox->setLayout(ivbox);
  layout->addWidget(groupBox);

  vbox->setLayout(layout);
  sbox->addWidget(vbox);

  setCentralWidget(sbox);

  // send position/azimuth updates to compass widget
  connect(locationProvider, SIGNAL(positionUpdated(const QGeoPositionInfo &)), 
	  compass, SLOT(setPosition(const QGeoPositionInfo &)));
  connect(locationProvider, SIGNAL(positionUpdated(const QCompassReading *)), 
	  compass, SLOT(setPosition(const QCompassReading *)));

  // and process position locally
  connect(locationProvider, SIGNAL(positionUpdated(const QGeoPositionInfo &)), 
	  this, SLOT(setPosition(const QGeoPositionInfo &)));
}

NavigationWindow::NavigationWindow(const Cache &cache, 
	   LocationProvider *locationProvider, 
	   WaypointDb *wptDb, MapWidget *mapWidget, QWidget *parent) : 
  CustomWindow("NavigationWindow", parent), m_cache(cache),
  m_coordinateTool(NULL), m_coo(cache.coordinate()), m_mapWidget(mapWidget) {

  setWindowTitle(tr("Navigation"));

  SwapBox *sbox = new SwapBox();

  if(!m_screenSaver.setScreenSaverInhibit())
    qDebug() << __FUNCTION__ << "disabling the screensaver failed!";

  // try to force the compass to be at least as wide
  // as the infos on the right (bottom)

  // left/top: the compass
  Compass *compass = new Compass(m_cache, this);
  sbox->addWidget(compass);

  // right/bottom: the textual info
  QWidget *vbox = new QWidget(compass);
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(0);

#ifdef Q_OS_SYMBIAN
  QWidget *groupBox = new QWidget();
#else
  QGroupBox *groupBox = new QGroupBox();
#endif
  QVBoxLayout *ivbox = new QVBoxLayout;
  m_latitude = new InfoWidget(tr("Latitude"));
  ivbox->addWidget(m_latitude);
  m_longitude = new InfoWidget(tr("Longitude"));
  ivbox->addWidget(m_longitude);
  m_precision = new InfoWidget(tr("Precision"));
  ivbox->addWidget(m_precision);
  groupBox->setLayout(ivbox);
  layout->addWidget(groupBox);

  layout->addStretch();

  m_coordinateTool = new CoordinateTool(m_cache, wptDb, mapWidget, this);
  layout->addWidget(m_coordinateTool->createWidget());

  QPushButton *centerMap = new QPushButton(tr("Set as map center"));
  connect(centerMap, SIGNAL(clicked()), this, SLOT(centerOnWpt()));
  layout->addWidget(centerMap);

  m_distance = new InfoWidget(tr("Distance"));
  m_coordinateTool->addWidget(m_distance);

  //  layout->addStretch();

  vbox->setLayout(layout);
  sbox->addWidget(vbox);

  setCentralWidget(sbox);

  connect(m_coordinateTool, SIGNAL(coordinateChanged(const QGeoCoordinate &)),
	  compass, SLOT(setTarget(const QGeoCoordinate &)));
  connect(m_coordinateTool, SIGNAL(typeChanged(const Waypoint &)),
	  compass, SLOT(setType(const Waypoint &)));
  connect(m_coordinateTool, SIGNAL(typeChanged()),
	  compass, SLOT(setType()));
  connect(m_coordinateTool, SIGNAL(coordinateChanged(const QGeoCoordinate &)),
	  this, SLOT(setTarget(const QGeoCoordinate &)));

  // send position/azimuth updates to compass widget
  connect(locationProvider, SIGNAL(positionUpdated(const QGeoPositionInfo &)), 
	  compass, SLOT(setPosition(const QGeoPositionInfo &)));
  connect(locationProvider, SIGNAL(positionUpdated(const QCompassReading *)), 
	  compass, SLOT(setPosition(const QCompassReading *)));

  // and process position locally
  connect(locationProvider, SIGNAL(positionUpdated(const QGeoPositionInfo &)), 
	  this, SLOT(setPosition(const QGeoPositionInfo &)));
}

NavigationWindow::~NavigationWindow() {
  qDebug() << __FUNCTION__;

  if(m_coordinateTool) delete m_coordinateTool;
}

void NavigationWindow::setTarget(const QGeoCoordinate &coo) {
  m_coo = coo;
}

void NavigationWindow::setPosition(const QGeoPositionInfo &pos) {
  qreal dist = pos.coordinate().distanceTo(m_coo);

  m_distance->setInfo(distanceString(dist));
  m_precision->setInfo(distanceString(
      pos.attribute(QGeoPositionInfo::HorizontalAccuracy)));
  m_latitude->setInfo(CoordinateTool::latitudeString(pos.coordinate()));
  m_longitude->setInfo(CoordinateTool::longitudeString(pos.coordinate()));
}

QString NavigationWindow::distanceString(qreal dist) {
  QString str;
  QLocale locale;

  if(locale.measurementSystem() == QLocale::ImperialSystem) {
    /* 10 ft == 3.0480m / 1m == 3.28084 ft */ 
    /* 10 yd == 9.144m / 1m == 1.0936133 yd */ 
    /* 100 yd == 91.44m / 1m == 1.0936133 yd */ 
    /* 1 mi = 1609.344m */ 
    if(dist<3.048)         str = QString::number(dist * 3.28084, 'f', 2) + " ft";
    else if(dist<9.144)    str = QString::number(dist * 1.0936133, 'f', 2) + " yd";
    else if(dist<91.44)    str = QString::number(dist * 1.0936133, 'f', 1) + " yd";
    else if(dist<16093.44) str = QString::number(dist / 1609.344, 'f', 2) + " mi";
    else if(dist<160934.4) str = QString::number(dist / 1609.344, 'f', 1) + " mi";
    else                   str = QString::number(dist / 1609.344, 'f', 0) + " mi";
  } else {
    if(dist<10)          str = QString::number(dist, 'f', 2) + " m";
    else if(dist<100)    str = QString::number(dist, 'f', 1) + " m";
    else if(dist<1000)   str = QString::number(dist, 'f', 0) + " m";
    else if(dist<100000) str = QString::number(dist/1000, 'f', 1) + " km";
    else                 str = QString::number(dist/1000, 'f', 0) + " km";
  }

  return str;
}


