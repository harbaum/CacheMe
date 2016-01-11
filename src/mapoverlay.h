#ifndef MAP_OVERLAY_H
#define MAP_OVERLAY_H

#include <QPainter>

#include "mapbuttonmanager.h"

class MapWidget;

class MapOverlay {
 public:
  MapOverlay(MapWidget *);
  ~MapOverlay();
  void paint(QPainter *);
  void enableZoomButtons(bool, bool);
  void enableGpsButton(bool);
  void enableUpdateButton(bool);
  void changeGpsButton(bool);

  bool mousePress(const QPointF &);
  bool mouseMove(const QPointF &);
  bool mouseRelease(const QPointF &);
  bool isInside(const QPointF &);

 private:
  MapButtonManager *m_mapButtonManager;
};

#endif // MAP_OVERLAY_H
