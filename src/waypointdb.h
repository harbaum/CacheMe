#ifndef WAYPOINTDB_H
#define WAYPOINTDB_H

#include <QObject>
#include <QString>
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "cache.h"

class WaypointDb  : public QObject {
  Q_OBJECT;

 public:
  WaypointDb();
  ~WaypointDb();
  QString getUnusedId(const Cache &);
  QString getUnusedName(const Cache &);
  QList<Waypoint> get(const Cache &);
  QList<Waypoint> get(const QString &);
  void append(Waypoint &);
  void update(Waypoint &);
  void removeByName(const QString &);

 signals:
  void waypointsChanged();

 private:
  bool write();
  bool read();
  QString fileName();
  void readGpx(QXmlStreamReader &);
  void readWaypoint(QXmlStreamReader &);
  
  QList<Waypoint> m_wpts;
};

#endif // WAYPOINTDB_H
