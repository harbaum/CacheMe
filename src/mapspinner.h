#ifndef MAP_SPINNER_H
#define MAP_SPINNER_H

class MapWidget;
#include <QPainter>
#include <QTimer>
#include <QRect>

class MapSpinner : QObject {
  Q_OBJECT;

 public:
  MapSpinner(MapWidget *);
  ~MapSpinner();
  void paint(QPainter *);

 private slots:
  void timerExpired();

 private:
  QRect area();
  MapWidget *m_mapWidget;
  QTimer *m_timer;
  int m_step;
};

#endif // MAP_SPINNER_H
