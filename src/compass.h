#ifndef COMPASS_H
#define COMPASS_H

#include <QWidget>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QTimer>
#include <QPixmap>
#include <QSvgRenderer>
#include <QGeoPositionInfo>

#include <QCompass>

QTM_USE_NAMESPACE

#include "cache.h"

class Compass : public QWidget {
  Q_OBJECT;

 public:
  Compass(const Cache &, QWidget * = 0);
  Compass(const Waypoint &, QWidget * = 0);
  ~Compass();
  int heightForWidth(int) const;
  void init();

 protected:
  void paintEvent(QPaintEvent *);

 public slots:
  void setPosition(const QGeoPositionInfo &);
  void setTarget(const QGeoCoordinate &);
  void setPosition(const QCompassReading *);
  void timerExpired();
  void setType();
  void setType(const Waypoint &);

 private:
  bool load(QSvgRenderer *, const QString &);
  void setAzimuth(qreal);

  bool m_hasCompass;
  QSvgRenderer *m_roseRenderer, *m_needleRenderer;
  QSvgRenderer *m_backgroundRenderer, *m_cacheRenderer;
  QTimer *m_timer;
  qreal m_calibration, m_calibrationV;
  qreal m_roseAngle, m_needleAngle;
  qreal m_roseSpeed, m_roseVAngle;
  static const qreal RATE=10;
  QPixmap *m_bgPix, *m_calPix;
  Cache m_cache;
  QGeoCoordinate m_target, m_lastPos;
};

#endif // COMPASS_H
