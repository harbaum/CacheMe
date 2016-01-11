#ifndef MAP_BANNER_H
#define MAP_BANNER_H

class MapWidget;
#include <QPainter>
#include <QString>

#include "mapbannerentry.h"

class MapBanner : QObject {
 Q_OBJECT;
 public:
  MapBanner(MapWidget *);
  ~MapBanner();
  void message(const QString &);
  void paint(QPainter *);

 private slots:
  void flush(MapBannerEntry*);

 private:
  MapWidget *m_mapWidget;
  MapBannerEntryList m_entries;
};

#endif // MAP_BANNER_H
