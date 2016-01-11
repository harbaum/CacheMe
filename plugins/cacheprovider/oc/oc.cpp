/* Oc plugin for CacheMe */

#include <QDebug>
#include <QtPlugin>
#include <QSettings>
#include <QTime>

#include "oc.h"
#include "cache.h"
#include "json.h"

#ifdef Q_WS_MAEMO_5
#define PLUGIN_NAME "OpenC"
#else
#define PLUGIN_NAME "OpenCaching"
#endif

#define URL_API "http://www.opencaching.com/api/"
// #define URL_API "http://test.opencaching.com/api/"
#define KEY     "ARye4gZxZXeezVF3"

const QVariant Oc::getInfo(const QString &name) const {
  if(name.compare("Name") == 0)
    return PLUGIN_NAME;

  if(name.compare("CanBeDefault") == 0)
    return true;

  if(name.compare("License") == 0)
    return QString(QObject::tr("Copyright 2012 by Till Harbaum"));
  
  return QVariant();
}

bool Oc::parseFloat(const QVariantMap &map, const QString &key, qreal &val) {

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

bool Oc::parseString(const QVariantMap &map, const QString &key, QString &str) {

  QVariantMap::const_iterator it = map.find(key);
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

bool Oc::parseDescription(const QVariantMap &map, 
		  const QString &key, Description &desc, bool html) {

  QString text;

  if(!parseString(map, key, text))
    return false;

  if(text.isEmpty())
    return false;

  desc.set(html, text);
  return true;
}

bool Oc::parseLocation(const QVariantMap &map, QGeoCoordinate &loc) {
  qreal lat, lon;
  if(parseFloat(map, "lat", lat))
    loc.setLatitude(lat);

  if(parseFloat(map, "lon", lon)) 
    loc.setLongitude(lon);
  
  return loc.isValid();
}

bool Oc::parsePerson(const QVariantMap &map, const QString &key, QString &person) {
  if(!map.contains(key)) return false;
  return parseString(map.value(key).toMap(), "name", person);
}

bool Oc::parseType(const QVariantMap &map, Cache::Type &type) {
  QString typeStr;
  if(!parseString(map, "type", typeStr))
    return false;

  int i;
  static const struct { Cache::Type type; QString str; } tags[] = {
    { Cache::TypeTraditional, "Traditional Cache"  }, 
    { Cache::TypeMulti,       "Multi-cache" },
    { Cache::TypeMystery,     "Unknown Cache" },
    { Cache::TypeVirtual,     "Virtual Cache" },
    { Cache::TypeUnknown,     "" }
  };

  for(i=0;(tags[i].type != Cache::TypeUnknown) && (tags[i].str.compare(typeStr) != 0);i++);
  type = tags[i].type;

  return true;
}

bool Oc::parseDate(const QVariantMap &map, const QString &key, QDate &date) {
  QString dateStr;

  if(!parseString(map, key, dateStr)) {
    date = QDate();
    return false;
  }

  dateStr.truncate(dateStr.length()-3);

  QDateTime dateTime;
  dateTime.setTime_t(dateStr.toInt());
  date = dateTime.date();

  return true;
}

bool Oc::parseSize(const QVariantMap &map, Container::Type &containerType) {
  // <= 1.6 = nano, 1.6-2.5 = micro, 2.6-3.5 = small
  // 3.6-4.5 = normal, > 4.5 = large

  qreal size;
  if(!parseFloat(map, "size", size))
    return false;

  if(size <= 2.5)
    containerType = Container::ContainerMicro;
  else if(size > 2.5 && size <= 3.5)
    containerType = Container::ContainerSmall;
  else if(size > 3.5 && size <= 4.5)
    containerType = Container::ContainerRegular;
  else if(size > 4.5)
    containerType = Container::ContainerLarge;
 
  return true;
}

bool Oc::parseLogType(const QMap<QString, QVariant> &map, 
		      Log::Type::Id &typeId) {

  if(!map.contains("type")) {
    typeId = Log::Type::Unknown;
    return false;
  }

  QString logTypeStr = map.value("type").toString();

  int i;
  static const struct { 
    Log::Type::Id type;
    QString str;
  } tags[] = {
    { Log::Type::Found,              "Found it" },
    { Log::Type::Found,              "Additional Find" },
    { Log::Type::NotFound,           "Didn't find it"},
    { Log::Type::WriteNote,          "Comment" },
    { Log::Type::Unknown,            "" }
  };

  typeId = Log::Type::Unknown;
  for(i=0;(tags[i].type != Log::Type::Unknown) && 
	(tags[i].str.compare(logTypeStr) != 0);i++);
  typeId = tags[i].type;

  return true;
}

bool Oc::parseLog(const QVariantMap &map, Cache &cache) {
  Log log;
  Description desc;
  if(parseDescription(map, "comment", desc, false)) 
    log.setDescription(desc);

  QString user;
  if(parsePerson(map, "user", user))
    log.setFinder(user);
  
  QDate log_time;
  if(parseDate(map, "log_time", log_time)) 
    log.setDate(log_time);
  
  Log::Type::Id typeId;
  if(parseLogType(map, typeId))
    log.setType(typeId);
  
  cache.appendLog(log);

  return true;
}

bool Oc::parseLogs(const QVariantMap &map, Cache &cache) {
  if(!map.contains("logs")) 
    return false;

  QVariantList list = map.value("logs").toList();

  for(QVariantList::const_iterator it = list.begin(); 
      it != list.end(); ++it) 
    parseLog((*it).toMap(), cache);

  return true;
}

bool Oc::parseImage(const QVariantMap &, Cache &) {
#if 0
  Image image;

  QString name;
  if(parseString(map, "Name", name)) image.setName(name);

  QString url;
  if(parseString(map, "Url", url)) image.setUrl(url);
  
  if(name.isEmpty() || url.isEmpty())
    return false;

  cache.appendImage(image);
#endif
  return true;
}

bool Oc::parseImages(const QVariantList &list, Cache &cache) {
  for(QVariantList::const_iterator it = list.begin(); 
      it != list.end(); ++it) 
    parseImage((*it).toMap(), cache);
  
  return true;
}

bool Oc::parseGeocache(const QVariantMap &map) {
  Cache cache;

  if(m_currentRequest != Overview) 
    qDebug() << __FUNCTION__ << map.keys();

  QGeoCoordinate coo;
  if(parseLocation(map.value("location").toMap(), coo))
    cache.setCoordinate(coo);
  
  QString name;
  if(parseString(map, "name", name))
    cache.setDescription(name);

  QString oxcode;
  if(parseString(map, "oxcode", oxcode))
    cache.setName(oxcode);

  Cache::Type type;
  if(parseType(map, type))
    cache.setType(type);

  QString hidden_by;
  if(parsePerson(map, "hidden_by", hidden_by))
    cache.setOwner(hidden_by);

  QDate hidden;
  if(parseDate(map, "hidden", hidden))
    cache.setDateOfPlacement(hidden);

  qreal difficulty, terrain;
  if(parseFloat(map, "difficulty", difficulty)) cache.setDifficulty(difficulty);
  if(parseFloat(map, "terrain", terrain)) cache.setTerrain(terrain);

  Container::Type containerType;
  if(parseSize(map, containerType))
    cache.setContainer(containerType);

  QString status;
  if(parseString(map, "status", status)) {
    if(status.compare("Active", Qt::CaseInsensitive) == 0) 
      cache.setAvailable(true);
    else
      cache.setAvailable(false);
  }

  Description desc;
  if(parseDescription(map, "description", desc, true)) 
    cache.setLongDescription(desc);

  Description hint;
  if(parseDescription(map, "hint", hint, false)) 
    cache.setHint(hint);

  // (spoiler) images
  if(map.contains("images"))
    parseImages(map.value("images").toList(), cache);

  parseLogs(map, cache);

  if(m_currentRequest == Overview) 
    m_cacheList.append(cache);
  else if(m_currentRequest == Info) 
    m_cache = cache;
  else
    qDebug() << __FUNCTION__ << "unexpected request";

  return true;
}

bool Oc::parseGeocaches(const QVariantList &list) {
  for(QVariantList::const_iterator it = list.begin(); 
      it != list.end(); ++it) 
    parseGeocache((*it).toMap());

  return true;
}

bool Oc::decodeJson(const QString &data) {
  bool ok;

  // json is parsing a QString containing the data to convert
  QVariant result = Json::parse(data, ok);
  if(!ok) {
    qDebug() << PLUGIN_NAME << __FUNCTION__ << "Json deconding failed.";
    return false;
  }

  if(m_currentRequest == Overview) {
    if(QVariant::List != result.type()) {
      qDebug() << PLUGIN_NAME << __FUNCTION__ << "Unexpected result type:" << result.type();
      return false;
    } 
    
    parseGeocaches(result.toList());
  }

  else if((m_currentRequest == Info) || (m_currentRequest == Detail)) {
    if(QVariant::Map != result.type()) {
      qDebug() << PLUGIN_NAME << __FUNCTION__ << "Unexpected result type:" << result.type();
      return false;
    } 
    
    parseGeocache(result.toMap());
  }

  else
    qDebug() << __FUNCTION__ << "don't know how to decode";

  return true;
}

void Oc::replyFinished(QNetworkReply *reply) {
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
    emit done();
    emit notifyBusy(false);
    
    QString allData = QString::fromUtf8(reply->readAll());
    //    qDebug() << __FUNCTION__ << allData;
    
    decodeJson(allData);

    Params params;
    switch(m_currentRequest) {
    case Overview:
      params.insert("Type", "Overview");
      emit replyCacheList(params, m_cacheList);
      break;

    case Info:
      params.insert("Type", "Info");
      emit replyCache(params, m_cache);
      break;

    case Detail:
      params.insert("Type", "Detail");
      emit replyCache(params, m_cache);
      break;

    default:
      break;
    }

    m_currentRequest = None;
    emit next();
  }
    
  reply->deleteLater();
}

Oc::Oc(): m_cacheList(PLUGIN_NAME), m_currentRequest(None) {  
  // connect network manager
  m_manager = new QNetworkAccessManager(this);
  connect(this->m_manager, SIGNAL(finished(QNetworkReply*)),
	  this, SLOT(replyFinished(QNetworkReply*)));
}

Oc::~Oc() {
}

void Oc::requestGet(const QString &cmd, const QMap<QString, QString> &items) {
  QNetworkRequest request;
  request.setRawHeader("User-Agent", "CacheMe " PLUGIN_NAME " Plugin");
  QUrl url(URL_API + cmd);

  foreach(QString key, items.keys()) 
    url.addQueryItem(key, items.value(key));

  url.addQueryItem("Authorization", KEY);

  //  qDebug() << __FUNCTION__ << url;

  request.setUrl(url);

  QNetworkReply *reply = this->m_manager->get(request);    
  if(reply->error())
    replyFinished(reply);
}

QObject *Oc::object() {
  return this;
}

void Oc::init(QWidget *, const QMap<QString, QVariant> &) {   
  qDebug() << PLUGIN_NAME << __FUNCTION__;
  
  emit reload();
}

bool Oc::busy() {
  return false;
}

void Oc::requestInfoDetail(const QString &name, bool info) {
  if(m_cache.name() == name) {
    Params params;
    params.insert("Type", info?"Info":"Detail");

    emit done();
    emit replyCache(params, m_cache);
    emit next();

    return;
  }

  emit notifyBusy(true);

  m_currentRequest = info?Info:Detail;
  m_cache = Cache();
  requestGet("geocache/" + name, QMap<QString, QString>() );
}

void Oc::requestOverview(const QGeoBoundingBox &bbox) {

  // too small bounding boxes cannot be handled
  if(bbox.bottomRight().distanceTo(bbox.topLeft()) < 10) {
    emit done();
    emit next();
    return;
  }

  QMap<QString, QString> items;

  items.insert("bbox",
 	       QString::number(bbox.bottomRight().latitude()) + "," +
	       QString::number(bbox.topLeft().longitude()) + "," +
	       QString::number(bbox.topLeft().latitude()) + "," +
	       QString::number(bbox.bottomRight().longitude()));

  items.insert("limit", "50"); 

  m_cacheList.clear();
  emit notifyBusy(true);

  m_currentRequest = Overview;
  requestGet("geocache", items);
}

void Oc::request(const Params &params) {
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

    requestOverview(bbox);
  } 

  else if(type.compare("Info") == 0) {
    QString name(params.value("Name").toString());
    requestInfoDetail(name, true);
  }
  
  else if(type.compare("Detail") == 0) {
    QString name(params.value("Name").toString());
    requestInfoDetail(name, false);
  }
  
  else {
    qDebug() << PLUGIN_NAME << __FUNCTION__ << "unexpected request" << type;
    emit done();
    emit next();
  }
}

// use this to react to settings changes, e.g. change of user name
void Oc::registerSettings(QDialog *) { }

// send an error message to the cache provider
void Oc::error(const QString &msg) {
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

Q_EXPORT_PLUGIN2(oc, Oc);
