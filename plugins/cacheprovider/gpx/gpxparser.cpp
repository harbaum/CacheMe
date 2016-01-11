//
// gpxparser.cpp
//
// parser for groundspeak gpx xml files
//

#include "gpxparser.h"
#include "quazip.h"
#include "quazipfile.h"

#include <QFileInfo>
#include <QDebug>

static void parseDrop(QXmlStreamReader &xml) {
  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) 
      parseDrop(xml);

    xml.readNext();
  }
}

void GpxParser::parseString(QXmlStreamReader &xml, QString &str) {
  str.append(xml.readElementText());
}

void GpxParser::parseTime(QXmlStreamReader &xml, QDateTime &time) {
  QString dateStr = xml.readElementText().trimmed();
  time = QDateTime::fromString(dateStr, Qt::ISODate);
}

void GpxParser::parseUrl(QXmlStreamReader &xml, QUrl &url) {
  url.setUrl(xml.readElementText());
}

static bool attrIsTrue(QXmlStreamAttributes &attr, const QString &name) {
  return !attr.value(name).toString().compare("true", Qt::CaseInsensitive);
}

void GpxParser::parseDescription(QXmlStreamReader &xml, Description &desc) {

  QXmlStreamAttributes attr = xml.attributes();
  desc.set(attrIsTrue(attr, "html"), xml.readElementText().trimmed());
}

void GpxParser::parseRating(QXmlStreamReader &xml, qreal &rating) {
  rating = xml.readElementText().toFloat();
}

void GpxParser::parseContainer(QXmlStreamReader &xml, Container::Type &type) {
  int i;

  struct { 
    Container::Type container;
    QString str;
  } tags[] = {
    { Container::ContainerRegular,   "Regular" },
    { Container::ContainerSmall,     "Small" },
    { Container::ContainerMicro,     "Micro" },
    { Container::ContainerOther,     "Other" },
    { Container::ContainerNotChosen, "Not chosen,Unknown" },
    { Container::ContainerLarge,     "Large" },
    { Container::ContainerVirtual,   "Virtual" },
    { Container::ContainerUnknown,   "<unknown>" }
  };

  QString containerStr = xml.readElementText();

  for(i=0;(tags[i].container != Container::ContainerUnknown) && 
	!(tags[i].str.split(",").contains(containerStr, Qt::CaseInsensitive));
      i++);

  type = tags[i].container;
}

void GpxParser::parseType(QXmlStreamReader &xml, Cache::Type &type) {
  int i;

  struct { 
    Cache::Type type;
    QString str;
  } tags[] = {
    { Cache::TypeTraditional, "Traditional Cache,Traditional,Geocache" },
    { Cache::TypeMulti,       "Multi-cache,Multi,Multicache" },
    { Cache::TypeMystery,     "Unknown Cache,Other,Unknown" },
    { Cache::TypeVirtual,     "Virtual Cache,Virtual" },
    { Cache::TypeWebcam,      "Webcam Cache,Webcam" },
    { Cache::TypeEvent,       "Event Cache,Event,Geocoins:" },
    { Cache::TypeLetterbox,   "Letterbox Hybrid,Letterbox" },
    { Cache::TypeEarthcache,  "Earthcache" },
    { Cache::TypeWherigo,     "Wherigo Cache" },
    { Cache::TypeMegaEvent,   "Mega-Event Cache" },
    { Cache::TypeCito,        "Cache In Trash Out Event" },
    { Cache::TypeUnknown,     "<unknown>" }
  };

  QString typeStr = xml.readElementText();

  for(i=0;(tags[i].type != Cache::TypeUnknown) && 
	!(tags[i].str.split(",").contains(typeStr, Qt::CaseInsensitive));i++);
      
  type = tags[i].type;
}

static bool attrIs1(QXmlStreamAttributes &attr, const QString &name) {
  return !attr.value(name).toString().compare("1", Qt::CaseInsensitive);
}

void GpxParser::parseAttribute(QXmlStreamReader &xml, Cache &cache) {
  QXmlStreamAttributes attr = xml.attributes();
  int id = attr.value("id").toString().toInt();

  Attribute::Id attIds[] = {
    Attribute::Unknown,  /*  0 */   Attribute::Dogs,        /*  1 */
    Attribute::Fee,      /*  2 */   Attribute::Rappelling,  /*  3 */
    Attribute::Boat,     /*  4 */   Attribute::Scuba,       /*  5 */
    Attribute::Kids,     /*  6 */   Attribute::OneHour,     /*  7 */
    Attribute::Scenic,   /*  8 */   Attribute::Hiking,      /*  9 */
    Attribute::Climbing, /* 10 */   Attribute::Wading,      /* 11 */
    Attribute::Swimming, /* 12 */   Attribute::Available,   /* 13 */
    Attribute::Night,    /* 14 */   Attribute::Winter,      /* 15 */
    Attribute::Unknown,  /* 16 */   Attribute::PoisonOak,   /* 17 */ 
    Attribute::Snakes,   /* 18 */   Attribute::Ticks,       /* 19 */ 
    Attribute::Mine,     /* 20 */   Attribute::Cliff,       /* 21 */ 
    Attribute::Hunting,  /* 22 */   Attribute::Danger,      /* 23 */ 
    Attribute::WheelChair,/* 24 */  Attribute::Parking,     /* 25 */ 
    Attribute::Public,   /* 26 */   Attribute::Water,       /* 27 */ 
    Attribute::Restrooms,/* 28 */   Attribute::Phone,       /* 29 */ 
    Attribute::Picnic,   /* 30 */   Attribute::Camping,     /* 31 */ 
    Attribute::Bicycles, /* 32 */   Attribute::Motorcycles, /* 33 */ 
    Attribute::Quads,    /* 34 */   Attribute::Jeeps,       /* 35 */ 
    Attribute::Snowmobiles,/* 36 */ Attribute::Horses,      /* 37 */ 
    Attribute::Campfires,/* 38 */   Attribute::Thorn,       /* 39 */ 
    Attribute::Stealth,  /* 40 */   Attribute::Stroller,    /* 41 */ 
    Attribute::FirstAid, /* 42 */   Attribute::Cow,         /* 43 */ 
    Attribute::Flashlight,/* 44 */  Attribute::LandF,       /* 45 */ 
    Attribute::RV,        /* 46 */  Attribute::FieldPuzzle, /* 47 */ 
    Attribute::UV,        /* 48 */  Attribute::Snowshoes,   /* 49 */ 
    Attribute::Skiis,     /* 50 */  Attribute::STool,       /* 51 */ 
    Attribute::NightCache,/* 52 */  Attribute::ParkNGrab,   /* 53 */ 
    Attribute::AbandonedBuilding,/* 54 */ Attribute::HikeShort,/* 55 */ 
    Attribute::HikeMed,   /* 56 */  Attribute::HikeLong,    /* 57 */ 
    Attribute::Fuel,      /* 58 */  Attribute::Food         /* 59 */ };

  xml.readElementText();
  if(id > 0 && id <= 59) {
    Attribute attribute(attIds[id], attrIs1(attr, "inc"));
    cache.appendAttribute(attribute);
  }
}

void GpxParser::parseAttributes(QXmlStreamReader &xml, Cache &cache) {

  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "attribute") 
	parseAttribute(xml, cache);
      else {
	qDebug() << __FUNCTION__ << "Unexpected attributes element: " 
			       << xml.name();
	parseDrop(xml);
      }
    }

    xml.readNext();
  }
}

#if 0
void GpxParser::parseTravelbug(QXmlStreamReader &xml, 
			       QList<Travelbug*> &travelbugs) {
  Travelbug *travelbug = new Travelbug;
  travelbugs.append(travelbug);

  QXmlStreamAttributes attr = xml.attributes();
  travelbug->id = attr.value("id").toString().toInt();
  travelbug->ref = attr.value("ref").toString();

  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "name") 
	parseString(xml, travelbug->name);
      else {
	qDebug() << __FUNCTION__ << "Unexpected travelbug element: " 
			       << xml.name();
	parseDrop(xml);
      }
    }

    xml.readNext();
  }
}

void GpxParser::parseTravelbugs(QXmlStreamReader &xml, 
			       QList<Travelbug*> &travelbugs) {

  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "travelbug") 
	parseTravelbug(xml, travelbugs);
      else {
	qDebug() << __FUNCTION__ << "Unexpected attributes element: " 
			       << xml.name();
	parseDrop(xml);
      }
    }

    xml.readNext();
  }
}
#endif

void GpxParser::parseLogText(QXmlStreamReader &xml, Description &logtext) {

  // QXmlStreamAttributes attr = xml.attributes();
  // logtext.encoded = attrIsTrue(attr, "encoded");
  
  logtext.set(false, xml.readElementText());
}

void GpxParser::parseLogType(QXmlStreamReader &xml, Log::Type::Id &logtype) {
  int i;

  struct { 
    Log::Type::Id type;
    QString str;
  } tags[] = {
    { Log::Type::Found,              "Found it,Found" },
    { Log::Type::NotFound,           "Didn't find it,Not Found" },
    { Log::Type::Maintenance,        "Owner Maintenance" },
    { Log::Type::WriteNote,          "Write Note,Note,Other" },
    { Log::Type::ReviewerNote,       "Post Reviewer Note" },
    { Log::Type::EnableListing,      "Enable Listing" },
    { Log::Type::PublishListing,     "Publish Listing" },
    { Log::Type::WillAttend,         "Will Attend" },
    { Log::Type::Attended,           "Attended" },
    { Log::Type::Photo,              "Webcam Photo taken" },
    { Log::Type::TempDisable,  "Temporarily Disable Listing,Cache Disabled!" },
    { Log::Type::NeedsMaintenance,   "Needs Maintenance" },
    { Log::Type::UpdatedCoordinates, "Updated Coordinates" },
    { Log::Type::Unarchive,          "Unarchive,Archive (show)" },
    { Log::Type::NeedsArchived,      "Needs Archived" },
    { Log::Type::Archive,            "Archive" },
    { Log::Type::Unknown,            "<unknown>" }
  };

  QString typeStr = xml.readElementText();

  for(i=0;(tags[i].type != Log::Type::Unknown) && 
	!(tags[i].str.split(",").contains(typeStr, Qt::CaseInsensitive));i++);
      
  logtype = tags[i].type;
}

void GpxParser::parseWptCoord(QXmlStreamReader &xml, QGeoCoordinate &coo) {
  QXmlStreamAttributes attr = xml.attributes();
  coo.setLatitude(attr.value("lat").toString().toFloat());
  coo.setLongitude(attr.value("lon").toString().toFloat());
  parseDrop(xml);
}

void GpxParser::parseLog(QXmlStreamReader &xml, Cache &cache) {
  Log log;

  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "date") {
	QDateTime date;
	parseTime(xml, date);
	log.setDate(date.date());
      } else if(xml.name() == "text") {
	Description logDescription;
	parseLogText(xml, logDescription);
	log.setDescription(logDescription);
      }
      else if(xml.name() == "finder") {
	QString name;
	parseString(xml, name);
	log.setFinder(name);
      } else if(xml.name() == "type") {
	Log::Type::Id id;
	parseLogType(xml, id);
	log.setType(id);
      } else if(xml.name() == "log_wpt") {
	QGeoCoordinate pos;
      	parseWptCoord(xml, pos);
      } else {
	qDebug() << __FUNCTION__ << "Unexpected log element: " << xml.name();
	parseDrop(xml);
      }
    }

    xml.readNext();
  }

  cache.appendLog(log);
}

void GpxParser::parseLogs(QXmlStreamReader &xml, Cache &cache) {

  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "log") 
	parseLog(xml, cache);
      else {
	qDebug() << __FUNCTION__ << "Unexpected attributes element: " 
			       << xml.name();
	parseDrop(xml);
      }
    }

    xml.readNext();
  }
}

void GpxParser::parseCache(QXmlStreamReader &xml, Cache &cache) {

  QXmlStreamAttributes attr = xml.attributes();
  cache.setId(attr.value("id").toString().toInt());
  cache.setAvailable(attrIsTrue(attr, "available") &&
		     !attrIsTrue(attr, "archived"));
  
  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "name") {
	QString name;
	parseString(xml, name);
	cache.setDescription(name);
      } else if(xml.name() == "short_description") {
	Description shortDescription;
	parseDescription(xml, shortDescription);
	cache.setShortDescription(shortDescription);
      } else if(xml.name() == "long_description") {
	Description longDescription;
	parseDescription(xml, longDescription);
	cache.setLongDescription(longDescription);
      } else if(xml.name() == "difficulty") {
	qreal difficulty;
	parseRating(xml, difficulty);
	cache.setDifficulty(difficulty);
      } else if(xml.name() == "terrain") {
	qreal terrain;
	parseRating(xml, terrain);
	cache.setTerrain(terrain);
      } else if(xml.name() == "owner") {
	QString owner;
      	parseString(xml, owner);
	cache.setOwner(owner);
      } else if(xml.name() == "type") {
	Cache::Type type;
	parseType(xml, type);
	cache.setType(type);
      } else if(xml.name() == "container") {
	Container::Type containerType;
	parseContainer(xml, containerType);
	cache.setContainer(containerType);
      } else if(xml.name() == "encoded_hints") {
	Description hints;
	QString hintsText;
	parseString(xml, hintsText);
	if(hintsText.simplified().size() > 0) {
	  hints.set(false, hintsText);
	  cache.setHint(hints);
	}
      } else if(xml.name() == "attributes") 
	parseAttributes(xml, cache);
      else if(xml.name() == "logs") 
	parseLogs(xml, cache);
      else if(xml.name() == "travelbugs") 
	parseDrop(xml);
      else if(xml.name() == "placed_by") 
	parseDrop(xml);
      else if(xml.name() == "country") 
	parseDrop(xml);
      else if(xml.name() == "state") 
	parseDrop(xml);
      else {
	qDebug() << __FUNCTION__ << "Unexpected cache element: " << xml.name();
	parseDrop(xml);
      }
    }

    xml.readNext();
  }
}

void GpxParser::parseWpt(QXmlStreamReader &xml, CacheList &cacheList) {
  Cache cache;

  QXmlStreamAttributes attr = xml.attributes();
  QGeoCoordinate coo;
  coo.setLatitude(attr.value("lat").toString().toFloat());
  coo.setLongitude(attr.value("lon").toString().toFloat());
  cache.setCoordinate(coo);

  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "name") {
	QString name;
	parseString(xml, name);
	cache.setName(name);
      } else if(xml.name() == "time") {
	QDateTime dateTime;
	parseTime(xml, dateTime);
	cache.setDateOfPlacement(dateTime.date());
      } else if(xml.name() == "cmt") {
	Description shortDescription;
	parseDescription(xml, shortDescription);
	cache.setShortDescription(shortDescription);
      } else if(xml.name() == "desc") {
	QString description;
	parseString(xml, description);
	cache.setDescription(description);
      } else if(xml.name() == "url") {
	QUrl url;
	parseUrl(xml, url);
	cache.setUrl(url);
      } else if(xml.name() == "urlname")
	parseDrop(xml);
      else if(xml.name() == "sym")
	parseDrop(xml);
      else if(xml.name() == "type")
	parseDrop(xml);
      else if(xml.name() == "cache")
	parseCache(xml, cache);
      else {
	qDebug() << __FUNCTION__ << "Unexpected wpt element: " << xml.name();
	parseDrop(xml);
      }
    }

    xml.readNext();
  }

  cacheList.append(cache);
}

void GpxParser::parseGpx(QXmlStreamReader &xml, CacheList &cacheList) {
  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "name") {
	QString name;
	parseString(xml, name);
	cacheList.setName(name);
      } else if(xml.name() == "time") {
	QDateTime dateTime;
	parseTime(xml, dateTime);
	cacheList.setDate(dateTime.date());
      } else if(xml.name() == "desc")
	parseDrop(xml);
      else if(xml.name() == "author")
      	parseDrop(xml);
      else if(xml.name() == "email")
	parseDrop(xml);
      else if(xml.name() == "keywords")
	parseDrop(xml);
      else if(xml.name() == "bounds") 
	parseDrop(xml);
      else if(xml.name() == "wpt")
	parseWpt(xml, cacheList);
      else {
	qDebug() << __FUNCTION__ << "Unexpected gpx element: " << xml.name();
	parseDrop(xml);
      }
    }

    xml.readNext();
  }
}

void GpxParser::parseLocContainer(QXmlStreamReader &xml, Container::Type &type) {
  int i;

  struct { 
    Container::Type container;
    int id;
  } tags[] = {
    { Container::ContainerMicro,     1 },
    { Container::ContainerSmall,     2 },
    { Container::ContainerRegular,   3 },
    { Container::ContainerLarge,     4 },
    { Container::ContainerOther,     5 },  // needs verification
    { Container::ContainerNotChosen, 6 },  // needs verification
    { Container::ContainerVirtual,   7 },  // needs verification
    { Container::ContainerUnknown,   -1 }
  };

  int containerId = xml.readElementText().toInt();

  for(i=0;(tags[i].container != Container::ContainerUnknown) && 
	tags[i].id != containerId;
      i++);
  
  type = tags[i].container;
}

void GpxParser::parseLocName(QXmlStreamReader &xml, Cache &cache) {
  QXmlStreamAttributes attr = xml.attributes();
  cache.setName(attr.value("id").toString());
  
  QString description;
  parseString(xml, description);
  cache.setDescription(description);
}

void GpxParser::parseLocWpt(QXmlStreamReader &xml, CacheList &cacheList) {
  Cache cache;

  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "name") 
	parseLocName(xml, cache);
      else if(xml.name() == "coord") {
	QGeoCoordinate coo;
	parseWptCoord(xml, coo);
	cache.setCoordinate(coo);
      } else if(xml.name() == "link") {
	QUrl url;
	parseUrl(xml, url);
	cache.setUrl(url);
      } else if(xml.name() == "type") {
	Cache::Type type;
	parseType(xml, type);
	cache.setType(type);
      } else if(xml.name() == "difficulty") {
	qreal difficulty;
	parseRating(xml, difficulty);
	cache.setDifficulty(difficulty);
      } else if(xml.name() == "terrain") {
	qreal terrain;
	parseRating(xml, terrain);
	cache.setTerrain(terrain);
      } else if(xml.name() == "container") {
	Container::Type containerType;
	parseLocContainer(xml, containerType);
	cache.setContainer(containerType);
      } else {
	qDebug() << __FUNCTION__ << "Unexpected loc/wpt element: " << xml.name();
	parseDrop(xml);
      }
    }

    xml.readNext();
  }

  cacheList.append(cache);
}

void GpxParser::parseLoc(QXmlStreamReader &xml, CacheList &cacheList) {
  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "waypoint") 
	parseLocWpt(xml, cacheList);
      else {
	qDebug() << __FUNCTION__ << "Unexpected loc element: " << xml.name();
	parseDrop(xml);
      }
    }

    xml.readNext();
  }
}

void GpxParser::parseRoot(QXmlStreamReader& xml, CacheList &cacheList) {
  while(!xml.atEnd() && !xml.hasError()) {
    xml.readNext();

    if(xml.tokenType() == QXmlStreamReader::StartDocument) 
      continue;

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "gpx") 
	parseGpx(xml, cacheList);
      else if(xml.name() == "loc") 
	parseLoc(xml, cacheList);
      else {
	qDebug() << __FUNCTION__ << "Unexpected element: " << xml.name();
	parseDrop(xml);
      }
    }
  }
}

void GpxParser::parseGpxWptType(QXmlStreamReader &xml, Waypoint::Type &wpttype) {
  int i;

  struct { 
    Waypoint::Type type;
    QString str;
  } tags[] = {
    { Waypoint::Multistage,   "Stages of a Multicache" },
    { Waypoint::Parking,      "Parking Area" },
    { Waypoint::Final,        "Final Location" },
    { Waypoint::Question,     "Question to Answer" },
    { Waypoint::Trailhead,    "Trailhead" },
    { Waypoint::Refpoint,     "Reference Point" },
    { Waypoint::Unknown,      "<unknown>" }
  };

  QString typeStr = xml.readElementText();

  for(i=0;(tags[i].type != Waypoint::Unknown) && 
	!(tags[i].str.split(",").contains(typeStr, Qt::CaseInsensitive));i++);

  wpttype = tags[i].type;
}

void GpxParser::parseWptWpt(QXmlStreamReader &xml, CacheList &cacheList) {
  Waypoint wpt;

  QXmlStreamAttributes attr = xml.attributes();
  QGeoCoordinate coo;
  coo.setLatitude(attr.value("lat").toString().toFloat());
  coo.setLongitude(attr.value("lon").toString().toFloat());
  wpt.setCoordinate(coo);

  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "time" ||
	 xml.name() == "url" ||
	 xml.name() == "type" ||
	 xml.name() == "urlname")
	parseDrop(xml);
      else if(xml.name() == "name") {
	QString name;
	parseString(xml, name);
	wpt.setName(name);
      } else if(xml.name() == "cmt") {
	QString cmt;
	parseString(xml, cmt);
	wpt.setComment(cmt);
      } else if(xml.name() == "desc") {
	QString desc;
	parseString(xml, desc);
	wpt.setDescription(desc);
      } else if(xml.name() == "sym") {
	Waypoint::Type type;
	parseGpxWptType(xml, type);
	wpt.setType(type);
      } else {
	qDebug() << __FUNCTION__ << "Unexpected wptgpx/wpt element: " << xml.name();
	parseDrop(xml);
      }
    }

    xml.readNext();
  }

  QString wptId = wpt.name().right(wpt.name().size()-2);

  bool found = false;
  QList<Cache>::iterator i;
  for( i = cacheList.begin(); i != cacheList.end(); ++i ) {
    if(i->name().endsWith(wptId)) {
      i->appendWaypoint(wpt);
      found = true;
    }
  }

  if(!found)
    qDebug() << __FUNCTION__ << "orphaned" << wpt.name();
}

void GpxParser::parseWptGpx(QXmlStreamReader &xml, CacheList &cacheList) {
  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "name" || 
	 xml.name() == "desc" || 
	 xml.name() == "author" || 
	 xml.name() == "email" || 
	 xml.name() == "url" || 
	 xml.name() == "urlname" || 
	 xml.name() == "time" || 
	 xml.name() == "keywords" ||
	 xml.name() == "bounds") 
	parseDrop(xml);
      else if(xml.name() == "wpt") { 
	parseWptWpt(xml, cacheList);
      } else {
	qDebug() << __FUNCTION__ << "Unexpected wptgpx element: " << xml.name();
	parseDrop(xml);
      }
    }

    xml.readNext();
  }
}

void GpxParser::parseWptRoot(QXmlStreamReader& xml, CacheList &cacheList) {

  while(!xml.atEnd() && !xml.hasError()) {
    xml.readNext();

    if(xml.tokenType() == QXmlStreamReader::StartDocument) 
      continue;

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "gpx") {
	parseWptGpx(xml, cacheList);
      } else {
	qDebug() << __FUNCTION__ << "Unexpected element: " << xml.name();
	parseDrop(xml);
      }
    }
  }
}

void GpxParser::verifyName(CacheList &cacheList, const QString &name) {
  qDebug() << __FUNCTION__ << cacheList.size() << name;

  QList<Cache>::iterator i;
  for( i = cacheList.begin(); i != cacheList.end(); ++i ) {
    /* set owned flag if the owner name equals our name */
    i->setOwned(i->owner().compare(name, Qt::CaseInsensitive) == 0);

    /* do the same for all logs */
    bool found = false;
    foreach(Log log, i->logs()) {
      if(log.type().is(Log::Type::Found) && 
	 log.finder().compare(name, Qt::CaseInsensitive) == 0)
	found = true;
    }

    /* and set the foung flag accordingly */
    i->setFound(found);
  }
}
