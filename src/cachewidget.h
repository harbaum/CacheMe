#ifndef CACHEWIDGET_H
#define CACHEWIDGET_H

#include <QWidget>
#include <QGeoCoordinate>
#include "iconloader.h"

class CacheWidget : public QWidget {
  Q_OBJECT;
 public:
  CacheWidget(IconLoader *, const Cache &, QGeoCoordinate &, QWidget * = 0);
  QPixmap renderRating(qreal, int);
  QWidget *ratingWidget(const QString &, const Rating &);
  QWidget *directionWidget(qreal, qreal);
  QWidget *containerWidget(const QString &, const Container &);
  QString name() const;

private:
  IconLoader *m_iconLoader;
  QPixmap *m_pixStar, *m_pixStarLight, *m_pixStarHalf;
  QString m_name;
};

#endif
