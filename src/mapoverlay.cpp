#include <QDebug>

#include <QObject>

#include "mapwidget.h"

MapOverlay::MapOverlay(MapWidget *mapWidget) {
  qDebug() << __FUNCTION__;

  // create a buttonmanager and fill with buttons
  m_mapButtonManager = new MapButtonManager(mapWidget);
  
  MapButton *zoom_in = m_mapButtonManager->create("zoom_in", 1, 1);
  QObject::connect(zoom_in, SIGNAL(clicked()), mapWidget, SLOT(zoomIn()));
  
  MapButton *gps = m_mapButtonManager->create("gps", 1, 2, MapButton::Disabled);
  QObject::connect(gps, SIGNAL(clicked()), mapWidget, SLOT(gpsFollow()));
  
  MapButton *zoom_out = m_mapButtonManager->create("zoom_out", 1, 3);
  QObject::connect(zoom_out, SIGNAL(clicked()), mapWidget, SLOT(zoomOut()));
  
  MapButton *update = m_mapButtonManager->create("update", -1, 1, MapButton::Hidden);
  QObject::connect(update, SIGNAL(clicked()), mapWidget, SLOT(manualUpdate()));

  MapButton *fullscreen = m_mapButtonManager->create("fs", -1, -1);
  QObject::connect(fullscreen, SIGNAL(clicked()), mapWidget, SLOT(toggleFullscreen()));
};

MapOverlay::~MapOverlay() {
  qDebug() << __FUNCTION__;

  delete m_mapButtonManager;
};

void MapOverlay::enableZoomButtons(bool in, bool out) {
  m_mapButtonManager->setState("zoom_in", in?MapButton::Normal:MapButton::Disabled);
  m_mapButtonManager->setState("zoom_out", out?MapButton::Normal:MapButton::Disabled);
}

void MapOverlay::enableGpsButton(bool on) {
  m_mapButtonManager->setState("gps", on?MapButton::Normal:MapButton::Disabled);
}

void MapOverlay::enableUpdateButton(bool on) {
  m_mapButtonManager->setState("update", on?MapButton::Normal:MapButton::Hidden);
}

void MapOverlay::changeGpsButton(bool locked) {
  m_mapButtonManager->change("gps", locked?"gps_follow":"gps");
}

void MapOverlay::paint(QPainter *painter) {
  m_mapButtonManager->paint(painter);
};

bool MapOverlay::mousePress(const QPointF &p) {
  return m_mapButtonManager->mousePress(p);
}

bool MapOverlay::mouseMove(const QPointF &p) {
  return m_mapButtonManager->mouseMove(p);
}

bool MapOverlay::mouseRelease(const QPointF &p) {
  return m_mapButtonManager->mouseRelease(p);
}

bool MapOverlay::isInside(const QPointF &p) {
  return m_mapButtonManager->isInside(p);
}
