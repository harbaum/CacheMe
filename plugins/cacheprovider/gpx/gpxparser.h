/*
*/  

#ifndef GPXPARSER_H
#define GPXPARSER_H

#include <QXmlStreamReader>

#include "cachelist.h"

class GpxParser {
 public:
  static void verifyName(CacheList &, const QString &);

 protected:
  void parseRoot(QXmlStreamReader &, CacheList &);
  void parseWptRoot(QXmlStreamReader &, CacheList &);

 private:
  void parseGpxWptType(QXmlStreamReader &, Waypoint::Type &);
  void parseWptWpt(QXmlStreamReader &, CacheList &);
  void parseWptGpx(QXmlStreamReader &, CacheList &);
  void parseGpx(QXmlStreamReader &, CacheList &);
  void parseLoc(QXmlStreamReader &, CacheList &);
  void parseWpt(QXmlStreamReader &, CacheList &);
  void parseLocWpt(QXmlStreamReader &, CacheList &);

  void parseLocName(QXmlStreamReader &, Cache &);
  void parseUrl(QXmlStreamReader &, QUrl &);
  void parseString(QXmlStreamReader &, QString &);
  void parseTime(QXmlStreamReader &, QDateTime &);
  void parseCache(QXmlStreamReader &, Cache &);
  void parseDescription(QXmlStreamReader &, Description &);
  void parseRating(QXmlStreamReader &, qreal &);
  void parseType(QXmlStreamReader &, Cache::Type &);
  void parseContainer(QXmlStreamReader &, Container::Type &);
  void parseLocContainer(QXmlStreamReader &, Container::Type &);
  void parseAttributes(QXmlStreamReader &, Cache &);
  void parseAttribute(QXmlStreamReader &, Cache &);
  void parseLogs(QXmlStreamReader &, Cache &);
  void parseLog(QXmlStreamReader &, Cache &);
  void parseLogText(QXmlStreamReader &, Description &);
  void parseLogType(QXmlStreamReader &, Log::Type::Id &);

  void parseWptCoord(QXmlStreamReader &, QGeoCoordinate &);

#if 0
  void parseTravelbugs(QXmlStreamReader &, QList<Travelbug*> &);
  void parseTravelbug(QXmlStreamReader &, QList<Travelbug*> &);
#endif
};

#endif // GPXPARSER_H
