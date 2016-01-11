#ifndef MAP_BUTTON_MANAGER_H
#define MAP_BUTTON_MANAGER_H

#include <QPainter>

#include "mapbutton.h"
#include "iconloader.h"

class MapWidget;

class MapButtonManager : public QList <MapButton*>  {
    
 public:
  MapButtonManager(MapWidget *);
  ~MapButtonManager();
  MapButton* create(const QString &, int, int, MapButton::State = MapButton::Normal);
  void setState(const QString &, MapButton::State = MapButton::Normal);
  void paint(QPainter *);
  bool mousePress(const QPointF &);
  bool mouseMove(const QPointF &);
  bool mouseRelease(const QPointF &);
  bool isInside(const QPointF &) const;
  void change(const QString &, const QString &);

 private:
  MapWidget *m_mapWidget;
  IconLoader *m_iconLoader;
};

#endif // MAP_BUTTON_MANAGER_H
