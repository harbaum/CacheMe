/* GcLive plugin for CacheMe */

#include <QDebug>
#include <QtPlugin>
#include <QSettings>
#include <QTime>
#include <QFile>
#include <QDir>
#include <QXmlStreamWriter>
#include <QSslError>

#include "gclivedialog.h"
#include "gclive.h"
#include "cache.h"
#include "json.h"

#include "kqoauthmanager.h"
#include "kqoauthrequest.h"

#define PLUGIN_NAME "GcLive"

#ifdef STAGING
#warning "Compiling for staging!"
#define URL_AUTH "https://staging.geocaching.com/oauth/mobileoauth.ashx"
#define URL_API "https://staging.api.groundspeak.com/Live/V6Beta/Geocaching.svc/"
#else
#define URL_AUTH "https://www.geocaching.com/OAuth/mobileoauth.ashx"
#define URL_API "https://api.groundspeak.com/LiveV6/Geocaching.svc/"
#endif

#define VERSION "0.9.4"

#if defined(Q_OS_SYMBIAN)
#define MANUFACTURER     "Nokia"
#define OPERATING_SYSTEM "Symbian"
#elif defined(Q_WS_MAEMO_5)
#define MANUFACTURER     "Nokia"
#define DEVICE_NAME      "N900"
#define OPERATING_SYSTEM "Maemo5"
#elif defined(Q_WS_QWS)
#define OPERATING_SYSTEM "Embedded Linux"
#elif defined(Q_WS_X11)
#define OPERATING_SYSTEM "Linux/X11"
#endif

#ifndef OPERATING_SYSTEM
#ifdef Q_OS_LINUX
#define OPERATING_SYSTEM "Linux"
#endif
#endif

void GcLive::onSslErrors( QNetworkReply *pReply, QList<QSslError> &pErrors ) {
  qDebug() << __FUNCTION__ << "SSL-Error: " << pReply->request().url();
  for ( int i = 0; i < pErrors.size(); i++ ) {
    QSslError error = pErrors.at( i );
    qDebug() << __FUNCTION__ << "Error: " << error.error() << " " << error.errorString();
  }

  pReply->ignoreSslErrors();
}

const QVariant GcLive::getInfo(const QString &name) const {
  if(name.compare("Name") == 0)
    return PLUGIN_NAME;

  if(name.compare("CanBeDefault") == 0)
    return true;

  if(name.compare("ManualUpdate") == 0)
    return true;

  if(name.compare("SupportsSearch") == 0)
    return true;

  if(name.compare("SupportsLogging") == 0)
    return true;

  if(name.compare("SupportsAuthorization") == 0)
    return true;

  if(name.compare("SearchSupportsContainerSelection") == 0)
    return !m_isBasicMember;

  if(name.compare("License") == 0)
    return QString(QObject::tr("Copyright 2012 by Till Harbaum") +
    "<center><b>Powered by Geocaching Live.</b><br>" 
    "<a href=\"http://www.geocaching.com/live\"><img src=\"data:image/png;base64,"
    "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\n"
    "3MWqCAUAAAAASUVORK5CYII=\""
    " alt=\"GcLive logo\"/></a></center>");
  
  return QVariant();
}

static void drop(QXmlStreamReader &xml) {
  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) 
      drop(xml);

    xml.readNext();
  }
}

bool GcLive::loadBasicLimits(QXmlStreamReader &xmlReader) {
  QXmlStreamAttributes attr = xmlReader.attributes();

  if(attr.hasAttribute("time") && attr.hasAttribute("count")) {
    m_basicDetailLock = QDateTime::fromString(attr.value("time").toString(), Qt::ISODate);
    m_basicDetailCount = attr.value("count").toString().toInt();

    qDebug() << __FUNCTION__ << "restored limits" << m_basicDetailCount << m_basicDetailLock;
  }
  drop(xmlReader);
  return true;
}

bool GcLive::loadGclive(QXmlStreamReader &xmlReader) {
  bool retval = true;
  xmlReader.readNext();

  // parse contents
  while(xmlReader.tokenType() != QXmlStreamReader::EndElement &&
	!xmlReader.atEnd() && !xmlReader.hasError()) {

    if(xmlReader.tokenType() == QXmlStreamReader::StartElement) {
      if(xmlReader.name() == "basicLimit") 
	retval &= loadBasicLimits(xmlReader);
      else if(xmlReader.name() == "cachelist") 
	retval &= m_cacheList.load(xmlReader);
      else if(xmlReader.name() == "cachedetails")
	retval &= m_cacheDetails.load(xmlReader);
      else {
	qDebug() << PLUGIN_NAME << __FUNCTION__ << "Unexpected element: " << xmlReader.name();
	drop(xmlReader);
      }
    }
    
    xmlReader.readNext();
  }
  return retval;
}

bool GcLive::load() {
  bool retval = true;
  QString fileName(m_dataDir + "/" + PLUGIN_NAME + ".xml");
  QFile file(fileName);

  if(!file.open(QIODevice::ReadOnly | QFile::Text)) {
    qDebug() << __FUNCTION__ << "unable to open " << fileName << "for reading";
    return false;
  }

  QXmlStreamReader xmlReader(&file);

  // parse contents
  while(xmlReader.tokenType() != QXmlStreamReader::EndElement &&
	!xmlReader.atEnd() && !xmlReader.hasError()) {

    if(xmlReader.tokenType() == QXmlStreamReader::StartElement) {
      if(xmlReader.name() == PLUGIN_NAME) 
	retval &= loadGclive(xmlReader);
      else {
	qDebug() << PLUGIN_NAME << __FUNCTION__ << "Unexpected element: " << xmlReader.name();
	drop(xmlReader);
      }
    }
    
    xmlReader.readNext();
  }

  file.close();

  return retval;
}

bool GcLive::save() {
  bool retval = true;
  QString fileName(m_dataDir + "/" + PLUGIN_NAME + ".xml");
  QFileInfo fileInfo( fileName );

  qDebug() << __FUNCTION__ << fileName;

  // first make sure the datadir exists
  if(!QDir().exists(fileInfo.path())) {
    if(!QDir().mkpath(fileInfo.path())) {
      qDebug() << __FUNCTION__ << "cannot create data dir for " << fileName;
      return false;
    }
  }

  QFile file(fileName);
  if(!file.open(QIODevice::WriteOnly | QFile::Text)) {
    qDebug() << __FUNCTION__ << "unable to open " << fileName << "for writing";
    return false;
  }

  QXmlStreamWriter xmlWriter(&file);
  xmlWriter.setAutoFormatting(true);
  xmlWriter.writeStartDocument();
  xmlWriter.writeStartElement(PLUGIN_NAME);

  if(m_isBasicMember) {
    xmlWriter.writeStartElement("basicLimit");
    xmlWriter.writeAttribute("time", m_basicDetailLock.toString(Qt::ISODate));
    xmlWriter.writeAttribute("count", QString::number(m_basicDetailCount));
    xmlWriter.writeEndElement(); 
  }

  // write cache list
  retval &= m_cacheList.save(&xmlWriter, "cachelist");

  // write cache detail list
  retval &= m_cacheDetails.save(&xmlWriter, "cachedetails");

  xmlWriter.writeEndElement(); 
  xmlWriter.writeEndDocument(); 
  file.close();

  return true;
}

bool GcLive::parseBool(const QMap<QString, QVariant> &map, const QString &key, bool &val) {

  QMap<QString, QVariant>::const_iterator it = map.find(key);
  if(it == map.end()) {
    qDebug() << __FUNCTION__ << "Key \"" << key << "\" not found.";
    return false;
  } 

  // the value of this key should be a map
  QVariant result = it.value();
  if(QVariant::Bool != result.type()) {
    qDebug() << PLUGIN_NAME << __FUNCTION__ << "Unexpected value type:" << result.type();
    return false;
  }

  val = result.toBool();
  return true;
}

bool GcLive::parseInt(const QMap<QString, QVariant> &map, const QString &key, int &val) {

  QMap<QString, QVariant>::const_iterator it = map.find(key);
  if(it == map.end()) {
    qDebug() << __FUNCTION__ << "Key \"" << key << "\" not found.";
    return false;
  }

  // the value of this key should be a map
  QVariant result = it.value();
  if(QVariant::String != result.type()) {
    qDebug() << PLUGIN_NAME << __FUNCTION__ << "Unexpected value type:" << result.type();
    return false;
  }

  val = result.toInt();
  return true;
}

bool GcLive::parseFloat(const QMap<QString, QVariant> &map, const QString &key, qreal &val) {

  QMap<QString, QVariant>::const_iterator it = map.find(key);
  if(it == map.end()) {
    qDebug() << __FUNCTION__ << "Key \"" << key << "\" not found.";
    return false;
  } 

  // the value of this key should be a map
  QVariant result = it.value();

  // gclive often uses null entries for things that aren't there. suppress this
  if(result.isNull())
    return false;

  if(QVariant::String != result.type()) {
    qDebug() << PLUGIN_NAME << __FUNCTION__ << "Unexpected value type:" << result.type();
    return false;
  }

  val = result.toFloat();
  return true;
}

bool GcLive::parseString(const QMap<QString, QVariant> &map, const QString &key, QString &str) {

  QMap<QString, QVariant>::const_iterator it = map.find(key);
  if(it == map.end()) {
    qDebug() << __FUNCTION__ << "Key \"" << key << "\" not found.";
    return false;
  } 

  QVariant result = it.value();

  // gclive often uses null entries for things that aren't there. suppress this
  if(result.isNull())
    return false;

  // the value of this key should be a map
  if(QVariant::String != result.type()) {
    qDebug() << PLUGIN_NAME << __FUNCTION__ << "Unexpected value type:" << result.type();
    return false;
  }

  str = result.toString();

  return true;
}

bool GcLive::parseCacheLimits(const QVariantMap &map) {
  int max, cur, left;

  if(parseInt(map, "MaxCacheCount", max) &&
     parseInt(map, "CurrentCacheCount", cur) &&
     parseInt(map, "CachesLeft", left))
    qDebug() << __FUNCTION__ << cur << "caches of" << max << "used," << left << "left";
  
  return true;
}

bool GcLive::parseStatus(const QVariantMap &map) {
  int statusCode = map.value("StatusCode", -1).toInt();
  m_status = (statusCode == 0);

  if(statusCode != 0) {
    m_statusMessage = map.value("StatusMessage").toString();

    error(m_statusMessage);
    qDebug() << "Status message: " << m_statusMessage;
    qDebug() << map;

    // if this happens while we are not initialized, then this
    // probably means we are not properly authorized. Try to re-authorize
    // then
    if(!m_initialized)
      authorize();

    return false;
  }

  return true;
}

bool GcLive::parseProfile(const QVariantMap &map) {
  QVariantMap userMap = map.value("User").toMap();

  QVariantMap typeMap = userMap.value("MemberType").toMap();
  m_isBasicMember = (typeMap.value("MemberTypeId", 1).toInt() == 1);
  m_userName = userMap.value("UserName").toString();

  qDebug() << __FUNCTION__ << "Username:" << m_userName << 
    "basic member:" << m_isBasicMember;

  m_initialized = true;
  emit notifyBusy(false);

  return true;
}

bool GcLive::parsePerson(const QVariantMap &map, const QString &key, QString &owner) {
  if(!map.contains(key)) return false;
  return parseString(map.value(key).toMap(), "UserName", owner);
}

bool GcLive::parseType(const QMap<QString, QVariant> &map, Cache::Type &val) {
  if(!map.contains("CacheType")) {
    val = Cache::TypeUnknown;
    return false;
  }

  QVariantMap typeMap = map.value("CacheType").toMap();

  int i, ctid;
  static const struct { Cache::Type type; int ctid; } tags[] = {
    { Cache::TypeTraditional, 2 }, { Cache::TypeMulti,        3 },
    { Cache::TypeMystery,     8 }, { Cache::TypeVirtual,      4 },
    { Cache::TypeWebcam,     11 }, { Cache::TypeEvent,        6 },
    { Cache::TypeLetterbox,   5 }, { Cache::TypeEarthcache, 137 }, 
    { Cache::TypeWherigo,  1858 }, { Cache::TypeMegaEvent,  453 },
    { Cache::TypeCito,       13 }, { Cache::TypeUnknown,     -1 }
  };

  if(!parseInt(typeMap, "GeocacheTypeId", ctid)) {
    val = Cache::TypeUnknown;
    return false;
  }

  for(i=0;(tags[i].type != Cache::TypeUnknown) && (tags[i].ctid != ctid);i++);
  val = tags[i].type;

  return true;
}

bool GcLive::parseContainer(const QMap<QString, QVariant> &map, Container &container) {
  if(!map.contains("ContainerType")) {
    container.set(Container::ContainerUnknown);
    return false;
  }

  QVariantMap containerMap = map.value("ContainerType").toMap();

  int id, i;
  static const struct { Container::Type type; int id; } tags[] = {
    { Container::ContainerRegular,   3 }, 
    { Container::ContainerSmall,     8 },
    { Container::ContainerMicro,     2 },
    { Container::ContainerOther,     6 },
    { Container::ContainerNotChosen, 1 },
    { Container::ContainerLarge,     4 },
    { Container::ContainerVirtual,   5 },
    { Container::ContainerUnknown,  -1 }
  };

  if(!parseInt(containerMap, "ContainerTypeId", id)) {
    container.set(Container::ContainerUnknown);
    return false;
  }

  container.set(Container::ContainerUnknown);
  for(i=0;(tags[i].type != Container::ContainerUnknown) && (tags[i].id != id);i++);
  container.set(tags[i].type);

  return container.isSet();
}

bool GcLive::parseDescription(const QMap<QString, QVariant> &map, 
			      const QString &key, Description &desc) {

  QString text;
  bool isHtml;

  if(!parseString(map, key, text))
    return false;

  if(!parseBool(map, key + "IsHtml", isHtml))
    isHtml = false;

  desc.set(isHtml, text);
  return true;
}

bool GcLive::parseDate(const QMap<QString, QVariant> &map, 
		       const QString &key, QDate &date) {
  QString dateStr;

  if(!parseString(map, key, dateStr)) {
    date = QDate();
    return false;
  }

  // extract everything in braces
  dateStr = dateStr.split("(")[1];
  dateStr = dateStr.split(")")[0];

  QStringList parts = dateStr.split("-");

  if(parts.size() < 2) return false;

  // we get three parts if the msecs were negative which seems to happen if date
  // is supposed to be unsed
  if(parts.size() == 3) {
    date = QDate();
    return false;
  }

  // time is in msec, so just cut off the last three digits to get seconds
  parts[0].chop(3);

  QDateTime dateTime;
  dateTime.setTime_t(parts[0].toInt());

  // setTime_t expects to do utc, so subtract at least the timezone hours
  // to get things right
  parts[1].chop(2);  // and extract hours from timezone

  dateTime = dateTime.addSecs(-3600*parts[1].toInt());
 
  date = dateTime.date();

  return true;
}

bool GcLive::parseLogType(const QMap<QString, QVariant> &map, Log::Type::Id &typeId) {

  if(!map.contains("LogType")) {
    typeId = Log::Type::Unknown;
    return false;
  }

  QVariantMap logTypeMap = map.value("LogType").toMap();
  
  int id, i;
  static const struct { 
    Log::Type::Id type;
    int id;
  } tags[] = {
    { Log::Type::Found,               2 },
    { Log::Type::NotFound,            3 },
    { Log::Type::Maintenance,        46 },
    { Log::Type::WriteNote,           4 },
    { Log::Type::ReviewerNote,       18 },
    { Log::Type::ReviewerNote,       68 },
    { Log::Type::EnableListing,      23 },
    { Log::Type::PublishListing,     24 },
    { Log::Type::WillAttend,          9 },
    { Log::Type::Attended,           10 },
    { Log::Type::Photo,              11 },
    { Log::Type::TempDisable,        22 },
    { Log::Type::NeedsMaintenance,   45 },
    { Log::Type::UpdatedCoordinates, 47 },
    { Log::Type::Unarchive,           1 },
    { Log::Type::Unarchive,          12 },
    { Log::Type::NeedsArchived,       7 },
    { Log::Type::Archive,             5 },
    { Log::Type::Unknown,            -1 }
  };

  if(!parseInt(logTypeMap, "WptLogTypeId", id)) {
    typeId = Log::Type::Unknown;    
    return false;
  }

  typeId = Log::Type::Unknown;
  for(i=0;(tags[i].type != Log::Type::Unknown) && (tags[i].id != id);i++);
  typeId = tags[i].type;

  return true;
}

bool GcLive::parseLog(const QVariantMap &map, Cache &cache) {
  QString text;
  if(!parseString(map, "LogText", text)) 
    return false;
  
  Log log;
  Description desc;
  desc.set(false, text);
  log.setDescription(desc);
  
  QString finder;
  if(parsePerson(map, "Finder", finder))
    log.setFinder(finder);
  
  QDate visitedAt;
  if(parseDate(map, "VisitDate", visitedAt)) 
    log.setDate(visitedAt);
  
  Log::Type::Id typeId;
  if(parseLogType(map, typeId))
    log.setType(typeId);
  
  cache.appendLog(log);
  
  return true;
}

bool GcLive::parseLogs(const QVariantMap &map, Cache &cache) {
  if(!map.contains("GeocacheLogs")) 
    return false;

  QVariantList list = map.value("GeocacheLogs").toList();

  for(QVariantList::const_iterator it = list.begin(); 
      it != list.end(); ++it) 
    parseLog((*it).toMap(), cache);

  return true;
}

bool GcLive::parseAttribute(const QVariantMap &map, Cache &cache) {
  int id;

  if(parseInt(map, "AttributeTypeID", id)) {
    bool yes;
    if(!parseBool(map, "IsOn", yes))
      yes = true;

    Attribute attribute((Attribute::Id)id, yes);
    cache.appendAttribute(attribute);
  }
  
  return true;
}

bool GcLive::parseAttributes(const QVariantMap &map, Cache &cache) {
  if(!map.contains("Attributes")) 
    return false;

  QVariantList list = map.value("Attributes").toList();

  for(QVariantList::const_iterator it = list.begin(); 
      it != list.end(); ++it) 
    parseAttribute((*it).toMap(), cache);

  return true;
}

bool GcLive::parseWptType(const QMap<QString, QVariant> &map, Waypoint::Type &type) {
  int id, i;
  static const struct { 
    Waypoint::Type type;
    int id;
  } tags[] = {
    { Waypoint::Multistage,   219 },
    { Waypoint::Parking,      217 },
    { Waypoint::Final,        220 },
    { Waypoint::Question,     218 },
    { Waypoint::Trailhead,    221 },
    { Waypoint::Refpoint,     452 },
    { Waypoint::Unknown,       -1 }
  };

  if(!parseInt(map, "WptTypeID", id)) {
    type = Waypoint::Unknown;    
    return false;
  }

  type = Waypoint::Unknown;
  for(i=0;(tags[i].type != Waypoint::Unknown) && (tags[i].id != id);i++);
  type = tags[i].type;

  return true;
}

bool GcLive::parseWaypoint(const QVariantMap &map, Cache &cache) {
  qreal latitude, longitude;
  if(parseFloat(map, "Latitude", latitude) && 
     parseFloat(map, "Longitude", longitude)) {
    Waypoint wpt;
    QGeoCoordinate coo(latitude, longitude);
    wpt.setCoordinate(coo);

    QString name;
    if(parseString(map, "Description", name)) wpt.setDescription(name);
    
    QString desc;
    if(parseString(map, "Comment", desc)) wpt.setComment(desc);
    
    QString code;
    if(parseString(map, "Code", code)) wpt.setName(code);
    
    Waypoint::Type type;
    if(parseWptType(map, type))
      wpt.setType(type);
  
    qDebug() << wpt;
  
    cache.appendWaypoint(wpt);
  }
  
  return true;
}

bool GcLive::parseWaypoints(const QVariantMap &map, Cache &cache) {
  if(!map.contains("AdditionalWaypoints")) 
    return false;

  QVariantList list = map.value("AdditionalWaypoints").toList();

  for(QVariantList::const_iterator it = list.begin(); 
      it != list.end(); ++it) 
    parseWaypoint((*it).toMap(), cache);

  return true;
}

bool GcLive::parseImage(const QVariantMap &map, Cache &cache) {
  Image image;

  QString name;
  if(parseString(map, "Name", name)) image.setName(name);

  QString desc;
  if(parseString(map, "Description", desc)) image.setDescription(desc);

  QString url;
  if(parseString(map, "MobileUrl", url)) image.setUrl(url);
  else
    if(parseString(map, "Url", url)) image.setUrl(url);
  
  if(name.isEmpty() || url.isEmpty())
    return false;

  cache.appendImage(image);
  return true;
}

bool GcLive::parseImages(const QVariantList &list, Cache &cache) {
  for(QVariantList::const_iterator it = list.begin(); 
      it != list.end(); ++it) 
    parseImage((*it).toMap(), cache);
  
  return true;
}

bool GcLive::parseGeocache(const QVariantMap &map) {
  Cache cache;

  //  qDebug() << __FUNCTION__ << map;

  bool available;
  if(parseBool(map, "Available", available)) cache.setAvailable(available);

  qreal latitude, longitude;
  if(parseFloat(map, "Latitude", latitude) && 
     parseFloat(map, "Longitude", longitude)) {
    QGeoCoordinate coo(latitude, longitude);
    cache.setCoordinate(coo);
  }

  QDate createdAt;
  if(parseDate(map, "DateCreated", createdAt)) 
    cache.setDateOfPlacement(createdAt);

  parseWaypoints(map, cache);
  parseAttributes(map, cache);
  parseLogs(map, cache);

  Description desc;
  if(parseDescription(map, "LongDescription", desc)) 
    cache.setLongDescription(desc);
  if(parseDescription(map, "ShortDescription", desc)) 
    cache.setShortDescription(desc);
  if(parseDescription(map, "EncodedHints", desc)) 
    cache.setHint(desc);

  int id;
  if(parseInt(map, "ID", id)) cache.setId(id);

  qreal difficulty, terrain;
  if(parseFloat(map, "Difficulty", difficulty)) cache.setDifficulty(difficulty);
  if(parseFloat(map, "Terrain", terrain)) cache.setTerrain(terrain);

  bool found;
  if(parseBool(map, "HasbeenFoundbyUser", found)) cache.setFound(found);

  QString owner;
  if(parsePerson(map, "Owner", owner)) {
    cache.setOwner(owner);
    cache.setOwned(owner.compare(m_userName, Qt::CaseInsensitive) == 0);
  }

  QString guid;
  if(parseString(map, "GUID", guid)) cache.setGuid(guid);

  QString name;
  if(parseString(map, "Name", name)) cache.setDescription(name);

  QString code;
  if(parseString(map, "Code", code)) cache.setName(code);

  QString url;
  if(parseString(map, "Url", url)) cache.setUrl(url);

  Cache::Type type;
  if(parseType(map, type)) cache.setType(type);

  Container container;
  if(parseContainer(map, container))  cache.setContainer(container.get());

  // (spoiler) images
  if(map.contains("Images"))
    parseImages(map.value("Images").toList(), cache);

  if(m_currentRequest == Search)
    m_searchResult.append(cache);

  if(m_currentRequest == Overview) {

    if(cache.waypoints().size() == 0) {
      // check if there are details known for this cache and attches
      // waypoints if present
      
      QList<Cache>::const_iterator i;
      for(i = m_cacheDetails.begin(); i != m_cacheDetails.end(); ++i ) {
	if((*i).name().compare(cache.name()) == 0) {
	  qDebug() << __FUNCTION__ << "matching detail entry" << cache.name();
	  QList<Waypoint> wpts = (*i).waypoints();
	  foreach(Waypoint wpt, wpts) 
	    cache.appendWaypoint(wpt);
	}
      }
    }

    m_cacheList.append(cache);
  }

  if((m_currentRequest == Info) ||(m_currentRequest == Detail)) {
    // limit number of caches details in memory to 25
    while(m_cacheDetails.size() > 25)
      m_cacheDetails.removeFirst();
      
    m_cacheDetails.append(cache);
  }

  return true;
}

bool GcLive::parseGeocaches(const QVariantList &list) {
  for(QVariantList::const_iterator it = list.begin(); 
      it != list.end(); ++it) 
    parseGeocache((*it).toMap());

  return true;
}

bool GcLive::decodeJson(const QString &data) {
  bool ok;

  m_status = false;

  // json is parsing a QString containing the data to convert
  QVariant result = Json::parse(data, ok);
  if(!ok) {
    qDebug() << PLUGIN_NAME << __FUNCTION__ << "Json deconding failed.";
    return false;
  }

  // we expect a qvariantmap
  if(QVariant::Map != result.type()) {
    qDebug() << PLUGIN_NAME << __FUNCTION__ << "Unexpected result type:" << result.type();
    return false;
  } 

  QMap<QString, QVariant> map = result.toMap();

  // there must be a "status" entry
  if(!parseStatus(map.value("Status").toMap())) {
    qDebug() << __FUNCTION__ << "Status check failed";
    return false;
  } 

  bool retval = true;

  // status ok, start seperating the replies
  if(map.contains("Profile"))
    retval = retval & parseProfile(map.value("Profile").toMap());

  if(map.contains("Geocaches"))
    retval = retval & parseGeocaches(map.value("Geocaches").toList());

  if(map.contains("CacheLimits"))
    parseCacheLimits(map.value("CacheLimits").toMap());

  return retval;
}

void GcLive::replyFinished(QNetworkReply *reply) {
  qDebug() << __FUNCTION__;

  if(reply->error() == QNetworkReply::UnknownNetworkError) {
    qDebug() << __FUNCTION__ << "bailing out due to unknown error";
    error("");  // clear error to make sure more messages are displayed
    error(tr("!Network connection failed, "
	     "please make sure you are connected to the internet and restart CacheMe!"));
    return;
  }

  if(reply->error()) {
    if(reply->errorString().contains("Error",  Qt::CaseInsensitive))
      error(reply->errorString());
    else
      error(tr("Error") + ": " + reply->errorString());
  }

  // invoke appropriate decoder
  if(reply->isFinished()) {
    bool initialized = m_initialized;

    // initialization is finished, any reply received now should
    // be a reply to a real request
    
    if(initialized) {
      emit done();
      emit notifyBusy(false);
    }
    
    QString allData = QString::fromUtf8(reply->readAll());
    //    qDebug() << __FUNCTION__ << allData;
    
    decodeJson(allData);

    if(initialized) {
      Params params;

      if(m_currentRequest == Overview) {
	params.insert("Type", "Overview");

	// just for fun save the current list
	save();
	m_overviewSent = true;

	emit replyCacheList(params, m_cacheList);

      } else if(m_currentRequest == SubmitLog) {

	params.insert("Type", "LogSubmitted");
	params.insert("Status", m_status);  
	params.insert("Message", m_statusMessage);  
	emit GcLive::reply(params);

      } else if(m_currentRequest == Search) {
	qDebug() << __FUNCTION__ << "search result" << m_searchResult.size();

	if(!m_searchResult.size()) {
	  error(tr("No matching geocaches were found!"));
	} else {
	  params.insert("Type", "Search");
	  emit replyCacheList(params, m_searchResult);
	}
      } else if(m_currentRequest == Info) {
	params.insert("Type", "Info");

	// if there are waypoints included refresh the overview
	if(m_cacheDetails.last().waypoints().size() != 0)
	    emit reload();
	
	save();
	emit replyCache(params, m_cacheDetails.last());

      } else if(m_currentRequest == Detail) {
	params.insert("Type", "Detail");

	save();
	emit replyCache(params, m_cacheDetails.last());
      }

      m_currentRequest = None;
      
      if(!busy()) 
	emit next();
    } else {
      // we weren't initialized, but are now? request overview!
      if(m_initialized) {
	emit reload();
	emit next();
      }
    }
  }
    
  reply->deleteLater();
}

void GcLive::onOpenUrl(const QUrl &url) {
  Params params;
  params.insert("Type", "OpenUrl");
  params.insert("Url", url.toString());
  emit reply(params);
}

void GcLive::authorize() {
  emit notifyBusy(true);

  error(tr("Opening Browser for Authorization"));

  if(!m_oauthRequest) 
    m_oauthRequest = new KQOAuthRequest;

  if(!m_oauthManager) {
    // try to do the authorization
    m_oauthManager = new KQOAuthManager(this); 

    connect(m_oauthManager, SIGNAL(temporaryTokenReceived(QString,QString)),
	    this, SLOT(onTemporaryTokenReceived(QString, QString)));
    
    connect(m_oauthManager, SIGNAL(accessTokenReceived(QString,QString)),
	    this, SLOT(onAccessTokenReceived(QString,QString)));

    connect(m_oauthManager, SIGNAL(authorizationReceived(QString,QString)),
            this, SLOT( onAuthorizationReceived(QString, QString)));

    connect(m_oauthManager, SIGNAL(openUrl(const QUrl &)),
            this, SLOT( onOpenUrl(const QUrl &)));
  }
  
  m_oauthRequest->initRequest(KQOAuthRequest::TemporaryCredentials, QUrl(URL_AUTH));
  m_oauthRequest->setConsumerKey(m_consumerKey);
  m_oauthRequest->setConsumerSecretKey(m_consumerSecretKey);
  m_oauthRequest->setHttpMethod(KQOAuthRequest::GET);
  m_oauthRequest->setEnableDebugOutput(true);

  m_oauthManager->setHandleUserAuthorization(true);
  
  m_oauthManager->executeRequest(m_oauthRequest);
}

GcLive::GcLive(): m_cacheDetails(PLUGIN_NAME " Details"), m_cacheList(PLUGIN_NAME), m_currentRequest(None) {  
  m_oauthRequest = NULL;
  m_oauthManager = NULL; 
  m_isBasicMember = true;
  m_initialized = false;
  m_overviewSent = false;
  m_basicDetailCount = 0;
  
  // connect network manager
  m_manager = new QNetworkAccessManager(this);
  connect(this->m_manager, SIGNAL(finished(QNetworkReply*)),
	  this, SLOT(replyFinished(QNetworkReply*)));
  connect(this->m_manager, SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)),
	  this, SLOT(onSslErrors(QNetworkReply *, const QList<QSslError> &)));
  
  QSettings settings;
  settings.beginGroup(PLUGIN_NAME);
  m_token = settings.value("token").toString();
  m_uniqueId = settings.value("uniqueId", -1).toInt();
  
  if(m_uniqueId == -1) {
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
    
    m_uniqueId = qrand();
    settings.setValue("uniqueId", m_uniqueId);
  }
  
  settings.endGroup();
}

void GcLive::requestUserInfo() {
  QString request("\"ProfileOptions\":{},"
		  "\"DeviceInfo\":{"
		  "\"ApplicationSoftwareVersion\":\"" VERSION "\","
#ifdef MANUFACTURER
		  "\"DeviceManufacturer\":\"" MANUFACTURER "\","
#endif
#ifdef OPERATING_SYSTEM
		  "\"DeviceOperatingSystem\":\"" OPERATING_SYSTEM "\","
#endif
#ifdef DEVICE_NAME
		  "\"DeviceName\":\"" DEVICE_NAME "\","
#endif
		  "\"DeviceUniqueId\":" + QString::number(m_uniqueId) +
		  "}");

  requestPost("GetYourUserProfile", request);
}

GcLive::~GcLive() {
  if(m_oauthRequest)
    delete m_oauthRequest;

  if(m_oauthManager)
    delete m_oauthManager;
}

void GcLive::onTemporaryTokenReceived(QString token, QString tokenSecret) {
  qDebug() << "Temporary token received: " << token << tokenSecret;
  
  if( m_oauthManager->lastError() == KQOAuthManager::NoError)
    m_oauthManager->getUserAuthorization(QUrl(URL_AUTH));
}

void GcLive::onAuthorizationReceived(QString token, QString verifier) {
  qDebug() << "User authorization received: " << token << verifier;

  if( m_oauthManager->lastError() == KQOAuthManager::NoError)
    m_oauthManager->getUserAccessTokens(QUrl(URL_AUTH));
}

void GcLive::requestGet(const QString &cmd) {
  QNetworkRequest request;
  request.setRawHeader("User-Agent", "CacheMe " PLUGIN_NAME " Plugin");
  QUrl url(URL_API + cmd);
  url.addEncodedQueryItem("accessToken", m_token.toAscii());
  url.addQueryItem("format", "json");
  request.setUrl(url);
  QNetworkReply *reply = this->m_manager->get(request);    
  if(reply->error())
    replyFinished(reply);
}

void GcLive::requestPost(const QString &cmd, const QString &req) {
  // wrap token entry around request
  QString data("{ \"AccessToken\":\"" + m_token + "\"," + req + "}");

  QNetworkRequest request;
  request.setRawHeader("User-Agent", "CacheMe " PLUGIN_NAME " Plugin");
  QUrl url(URL_API + cmd);
  url.addQueryItem("format", "json");
  request.setUrl(url);
  request.setRawHeader("Content-Type", "application/json");

  QNetworkReply *reply = this->m_manager->post(request, data.toUtf8());    
  if(reply->error())
    replyFinished(reply);
}

void GcLive::onAccessTokenReceived(QString token, QString) {
  qDebug() << "Access token received: " << token;

  QSettings settings;
  settings.beginGroup(PLUGIN_NAME);
  settings.setValue("token", token);
  settings.endGroup();

  m_token = token;

  requestUserInfo();
}

QObject *GcLive::object() {
  return this;
}

void GcLive::init(QWidget *, const QMap<QString, QVariant> &params) {   
  qDebug() << PLUGIN_NAME << __FUNCTION__;

#ifdef STAGING
  error("Staging!!");
#endif

  m_overviewSent = false;
  m_consumerKey = params.value("ConsumerKey").toString();
  m_consumerSecretKey = params.value("ConsumerSecretKey").toString();
  m_dataDir = params.value("DataDir").toString();

  load();

  if(!m_initialized) {
    if(m_token.isEmpty()) 
      authorize();
    else
      requestUserInfo();
  } else
    emit reload();
}

bool GcLive::busy() {
  return !m_initialized;
}

void GcLive::requestOverview(const QGeoBoundingBox &bbox, const int flags) {

  // first request goes through (but usually should have come from cache)
  if((m_overviewSent || m_cacheList.size() > 0 ) && !(flags & CacheProviderPlugin::Manual)) {
    Params params;
    params.insert("Type", "Overview");

    qDebug() << __FUNCTION__ << "suppressing automatic request";

    // handle all entries with empty waypoints list
    QList<Cache>::iterator c;
    for(c = m_cacheList.begin(); c != m_cacheList.end(); ++c ) {
      if((*c).waypoints().size() == 0) {

	// check if there are details known for this cache and attches
	// waypoints if present
	QList<Cache>::const_iterator i;
	for(i = m_cacheDetails.begin(); i != m_cacheDetails.end(); ++i ) {
	  if((*i).name().compare((*c).name()) == 0) {
	    QList<Waypoint> wpts = (*i).waypoints();
	    foreach(Waypoint wpt, wpts) 
	      (*c).appendWaypoint(wpt);
	  }
	}
      }
    }
    
    // return whatever we have in our list
    emit done();
    emit replyCacheList(params, m_cacheList);
    emit next();
    
    return;
  }
    
  QString request("\"IsLite\":true,"
		  "\"MaxPerPage\":50,");
  
  // basic members will only see traditional caches
  if(m_isBasicMember) 
    request += "\"GeocacheType\":{\"GeocacheTypeIds\":[2]},";
  
  // attach optional options
  if(flags & CacheProviderPlugin::NoFound) 
    request += "\"NotFoundByUsers\":{\"UserNames\":[\"" + m_userName + "\"] },";
  if(flags & CacheProviderPlugin::NoOwned) 
    request += "\"NotHiddenByUsers\":{\"UserNames\":[\"" + m_userName + "\"] },";
  
  request += "\"Viewport\": {"
    "\"BottomRight\":{"
    "\"Latitude\":" + QString::number(bbox.bottomRight().latitude()) + ","
    "\"Longitude\":" + QString::number(bbox.bottomRight().longitude()) +
    "},"
    "\"TopLeft\":{"
    "\"Latitude\":" + QString::number(bbox.topLeft().latitude()) + ","
    "\"Longitude\":" + QString::number(bbox.topLeft().longitude()) +
    "}";
  
  m_cacheList.clear();
  emit notifyBusy(true);

  m_currentRequest = Overview;
  requestPost("SearchForGeocaches", request);
}

void GcLive::requestInfoDetail(const QString &name, bool info) {
  // search for cache in cacheDetails
  QList<Cache>::const_iterator i;
  Cache cache;
  for(i = m_cacheDetails.begin(); i != m_cacheDetails.end(); ++i ) 
    if((*i).name().compare(name) == 0)
      cache = (*i);
  
  // check if we can fulfil that from cache
  if(cache.name().compare(name) == 0) {
    Params params;
    params.insert("Type", info?"Info":"Detail");

    emit done();
    emit replyCache(params, cache);
    emit next();
    
    return;
  }
  
  if(m_isBasicMember) {
    // don't process if date has been set, is in the future and it three
    // items have already been downloaded
    if(m_basicDetailLock.isValid() && 
       m_basicDetailLock > QDateTime::currentDateTime() &&
       m_basicDetailCount >= 3) {
      
      GcLiveDialog dialog;
      dialog.limits(m_basicDetailLock, m_basicDetailCount);
      dialog.exec();
      
      emit done();
      emit next();
      return;
    }
    
    if(!m_basicDetailLock.isValid()) {
      // start with fresh lock timer
      m_basicDetailLock = QDateTime::currentDateTime().addDays(1);
      m_basicDetailCount = 0;
    } else {
      // reset limit if timer has exceeded
      if(m_basicDetailLock <= QDateTime::currentDateTime()) {
	m_basicDetailLock = QDateTime::currentDateTime().addDays(1);
	m_basicDetailCount = 0;
      }
    }
    
    GcLiveDialog dialog;
    dialog.limits(m_basicDetailLock, m_basicDetailCount);
    if(dialog.exec() == QDialog::Rejected) {
      emit done();
      emit next();
      return;
    }
    
    m_basicDetailCount++;
  }
  
  QString request("\"IsLite\":false,"
		  "\"MaxPerPage\":1,"
		  "\"CacheCode\":{"
		  "\"CacheCodes\":[\"" + name + "\"]"
		  "}");
  
  emit notifyBusy(true);
  m_currentRequest = info?Info:Detail;
  requestPost("SearchForGeocaches", request);
}

void GcLive::requestSearch(const QGeoCoordinate &center, const QString &text, 
			   const QStringList &typesList, 
			   const QStringList &containerList,
			   bool excludeOwn, bool excludeFound) {

  QString request("\"IsLite\":true,\"MaxPerPage\":50,");
  
  request += "\"PointRadius\":{"
    "\"DistanceInMeters\":20000000,"
    "\"Point\":{"
    "\"Latitude\":" + QString::number(center.latitude()) + ","
    "\"Longitude\":"+ QString::number(center.longitude()) + "}"
    "},";
  
  if(!text.isEmpty()) {
    QString stext = text.simplified(); // remove multiple white spaces etc

    // check if this looks like a cache code and search for that instead
    if(stext.length() == 7 && stext.startsWith("gc", Qt::CaseInsensitive))
      request += 
	"\"CacheCode\":{"
	"\"CacheCodes\":[\"" + stext + "\"]"
	"},";
    else
      request += 
	"\"GeocacheName\":{"
	"\"GeocacheName\":\"" + stext + "\""
	"},";
  }
  
  if(excludeOwn) 
    request += "\"NotHiddenByUsers\":{\"UserNames\":[\"" + m_userName + "\"] },";

  if(excludeFound)
    request += "\"NotFoundByUsers\":{\"UserNames\":[\"" + m_userName + "\"] },";

  
  static const struct { Cache::Type type; int ctid; } typeIds[] = {
    { Cache::TypeTraditional, 2 }, { Cache::TypeMulti,        3 },
    { Cache::TypeMystery,     8 }, { Cache::TypeVirtual,      4 },
    { Cache::TypeWebcam,     11 }, { Cache::TypeEvent,        6 },
    { Cache::TypeLetterbox,   5 }, { Cache::TypeEarthcache, 137 }, 
    { Cache::TypeWherigo,  1858 }, { Cache::TypeMegaEvent,  453 },
    { Cache::TypeCito,       13 }, { Cache::TypeUnknown,     -1 }
  };
  
  // build type list
  int i;
  bool first = true;
  request += "\"GeocacheType\":{\"GeocacheTypeIds\":[";
  // parse types list
  foreach(QString typeStr, typesList) {
    Cache::Type type = (Cache::Type)typeStr.toInt();
    for(i=0;(typeIds[i].type != Cache::TypeUnknown) && 
	  (typeIds[i].type != type);i++);
    if(typeIds[i].type != Cache::TypeUnknown) {
      if(!first) request += ",";
      request += QString::number(typeIds[i].ctid);
      first = false;
    }
  }
  request += "]}";
  
  if(!m_isBasicMember) {
    request += ",";

    static const struct { Container::Type type; int id; } containerIds[] = {
      { Container::ContainerRegular,   3 }, 
      { Container::ContainerSmall,     8 },
      { Container::ContainerMicro,     2 },
      { Container::ContainerOther,     6 },
      { Container::ContainerNotChosen, 1 },
      { Container::ContainerLarge,     4 },
      { Container::ContainerVirtual,   5 },
      { Container::ContainerUnknown,  -1 }
    };

    // build container list
    first = true;
    request += "\"GeocacheContainerSize\":{\"GeocacheContainerSizeIds\":[";
    foreach(QString containerStr, containerList) {
      Container::Type container = (Container::Type)containerStr.toInt();
      for(i=0;(containerIds[i].type != Container::ContainerUnknown) && 
	    (containerIds[i].type != container);i++);
      if(containerIds[i].type != Container::ContainerUnknown) {
	if(!first) request += ",";
	request += QString::number(containerIds[i].id);
	first = false;
      }
    }
    request += "]}";
  }
  
  qDebug() << request;

  m_searchResult.clear();
  emit notifyBusy(true);
  m_currentRequest = Search;

  requestPost("SearchForGeocaches", request);
}

void GcLive::requestLog(const QString &cache, const QString &text, const QDate &date, Log::Type::Id typeId,
			bool favorite, bool encrypt, bool publish) {
  qDebug() << __FUNCTION__;

  int i, id = -1;
  static const struct { 
    Log::Type::Id type;
    int id;
  } tags[] = {
    { Log::Type::Found,               2 },
    { Log::Type::NotFound,            3 },
    { Log::Type::Maintenance,        46 },
    { Log::Type::WriteNote,           4 },
    { Log::Type::ReviewerNote,       18 },
    { Log::Type::ReviewerNote,       68 },
    { Log::Type::EnableListing,      23 },
    { Log::Type::PublishListing,     24 },
    { Log::Type::WillAttend,          9 },
    { Log::Type::Attended,           10 },
    { Log::Type::Photo,              11 },
    { Log::Type::TempDisable,        22 },
    { Log::Type::NeedsMaintenance,   45 },
    { Log::Type::UpdatedCoordinates, 47 },
    { Log::Type::Unarchive,           1 },
    { Log::Type::Unarchive,          12 },
    { Log::Type::NeedsArchived,       7 },
    { Log::Type::Archive,             5 },
    { Log::Type::Unknown,            -1 }
  };

  for(i=0;(tags[i].type != Log::Type::Unknown) && (tags[i].type != typeId);i++);
  id = tags[i].id;

  // first convert to utc
  QDateTime dateTime(date, QTime(12,0,0), Qt::UTC);

  QString dateStr("/Date(" + QString::number(dateTime.toTime_t()) + "000)/");

  // convert newlines to JSON newlines
  QString newtext(text);
  newtext.replace("\n", "\\n");

  QString request("\"CacheCode\": \"" + cache + "\","
		  "\"Note\": \"" + newtext + "\","
		  "\"PromoteToLog\": " + (publish?"true":"false") + ","
		  "\"EncryptLogText\": " + (encrypt?"true":"false") + ","
		  "\"FavoriteThisCache\": " + (favorite?"true":"false") + ","
		  "\"UTCDateLogged\": \"" + dateStr + "\","
		  "\"WptLogTypeId\": " + QString::number(id)
		  );

  m_currentRequest = SubmitLog;

  //   qDebug() << __FUNCTION__ << request;

  requestPost("CreateFieldNoteAndPublish", request);

  // remove matching cache from details list, so it gets
  // refreshed next time. Don't do this for basic members
  // as their downloads are limited
  if(!m_isBasicMember) {
    m_cacheDetails.removeByName(cache);
    save();
  }
}

void GcLive::request(const Params &params) {
  QString type = params.value("Type").toString();

  Q_ASSERT(m_currentRequest == None);

  if(type.compare("Overview") == 0) {
    // assembler parameters from request
    QGeoCoordinate topLeft, bottomRight;
    QGeoBoundingBox bbox;

    topLeft.setLatitude(params.value("TLLat").toFloat());
    topLeft.setLongitude(params.value("TLLon").toFloat());
    bottomRight.setLatitude(params.value("BRLat").toFloat());
    bottomRight.setLongitude(params.value("BRLon").toFloat());
    bbox = QGeoBoundingBox(topLeft, bottomRight);

    int flags = params.value("Flags").toInt();

    requestOverview(bbox, flags);
  }
  
  else if(type.compare("Info") == 0) {
    QString name(params.value("Name").toString());
    requestInfoDetail(name, true);
  }

  else if(type.compare("Detail") == 0) {
    QString name(params.value("Name").toString());
    requestInfoDetail(name, false);
  }

  else if(type.compare("MapSearchResults") == 0) {

    // make the search results the current cache list
    m_cacheList = m_searchResult;

    // update waypoints

    // handle all entries with empty waypoints list
    QList<Cache>::iterator c;
    for(c = m_cacheList.begin(); c != m_cacheList.end(); ++c ) {
      if((*c).waypoints().size() == 0) {

	// check if there are details known for this cache and attches
	// waypoints if present
	QList<Cache>::const_iterator i;
	for(i = m_cacheDetails.begin(); i != m_cacheDetails.end(); ++i ) {
	  if((*i).name().compare((*c).name()) == 0) {
	    QList<Waypoint> wpts = (*i).waypoints();
	    foreach(Waypoint wpt, wpts) 
	      (*c).appendWaypoint(wpt);
	  }
	}
      }
    }

    save();

    emit done();
    emit replyCacheList(params, m_cacheList);
    emit next();
  }  

  else if(type.compare("CreateLog") == 0) {
    QString text(params.value("Text").toString());
    Log::Type::Id type = (Log::Type::Id)params.value("TypeId").toInt();
    QDate date(params.value("Date").toDate());
    QString cache(params.value("Cache").toString());
    bool favorite = params.value("Favorite", false).toBool();
    bool encrypt = params.value("Encrypt", false).toBool();
    bool publish = params.value("Publish", true).toBool();

    requestLog(cache, text, date, type, favorite, encrypt, publish);
  }

  else if(type.compare("Search") == 0) {
    QGeoCoordinate center;
    center.setLatitude(params.value("Latitude").toFloat());
    center.setLongitude(params.value("Longitude").toFloat());
    QString text(params.value("Text").toString());
    QStringList typesList = params.value("Types").toString().split(";");
    QStringList contList = params.value("Containers").toString().split(";");
    bool excludeOwned = params.value("ExcludeOwned", false).toBool();
    bool excludeFound = params.value("ExcludeFound", false).toBool();

    requestSearch(center, text, typesList, contList, excludeOwned, excludeFound);
  }

  else if(type.compare("Drop") == 0) {
    // drop cache from details list
    QString cache(params.value("Cache").toString());
    if(!cache.isEmpty()) m_cacheDetails.removeByName(cache);
    else                 qDebug() << __FUNCTION__ << "no cache " << cache << " in detail list";
    emit done();
    emit next();
  }

  else if(type.compare("Authorize") == 0) {
    if(!m_initialized) {
      error(tr("Authorization already in progress"));
      emit done();
      emit next();
    } else {
      m_initialized = false;

      emit done();
      authorize();
      emit reload();
    }
  }

  else {
    qDebug() << PLUGIN_NAME << __FUNCTION__ << "unexpected request" << type;
    emit done();
    emit next();
  }
}

// use this to react to settings changes, e.g. change of user name
void GcLive::registerSettings(QDialog *) { }

// send an error message to the cache provider
void GcLive::error(const QString &msg) {
  Params params;
  params.insert("Type", "Error");

  if(!msg.isEmpty()) {
    if(msg.at(0) == QChar('!')) 
      params.insert("Message", 
		    "!" + tr(PLUGIN_NAME) + ": " + QString(msg).remove(0,1));
    else
      params.insert("Message", tr(PLUGIN_NAME) + ": " + msg);
  }
  
  emit reply(params);
}

Q_EXPORT_PLUGIN2(gclive, GcLive);
