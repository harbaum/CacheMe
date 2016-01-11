#include <QDebug>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "cache.h"

void writeTextElement(QXmlStreamWriter *xmlWriter, const QString &name, const QString &text) {
  if(!text.isEmpty()) {
    xmlWriter->writeStartElement(name);
    xmlWriter->writeCharacters(text);
    xmlWriter->writeEndElement();
  }
}

void writeDateElement(QXmlStreamWriter *xmlWriter, const QString &name, const QDate &date) {
  if(date.isValid()) {
    xmlWriter->writeStartElement(name);
    xmlWriter->writeAttribute("y", QString::number(date.year()));
    xmlWriter->writeAttribute("m", QString::number(date.month()));
    xmlWriter->writeAttribute("d", QString::number(date.day()));
    xmlWriter->writeEndElement();
  }
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

bool loadDate(QXmlStreamReader &xmlReader, QDate &date) {
  int y,m,d;
  QXmlStreamAttributes attr = xmlReader.attributes();
  y = attr.value("y").toString().toInt();
  m = attr.value("m").toString().toInt();
  d = attr.value("d").toString().toInt();
  drop(xmlReader);

  date = QDate(y, m, d);
  return date.isValid();
}

//------------------- Image -------------------

Image::Image() { }

void Image::setName(const QString &name) {
  m_name = name;
}

void Image::setDescription(const QString &description) {
  m_description = description;
}

void Image::setUrl(const QString &url) {
  m_url = url;
}

QString Image::name() const {
  return m_name;
}

QString Image::description() const {
  return m_description;
}

QString Image::url() const {
  return m_url;
}

void Image::save(QXmlStreamWriter *xmlWriter) {
  xmlWriter->writeStartElement("image");
  writeTextElement(xmlWriter, "name", name());
  writeTextElement(xmlWriter, "desc", description());
  writeTextElement(xmlWriter, "url", url());
  xmlWriter->writeEndElement();
}

bool Image::load(QXmlStreamReader &xmlReader) {
  xmlReader.readNext();

  // parse contents
  while(xmlReader.tokenType() != QXmlStreamReader::EndElement &&
	!xmlReader.atEnd() && !xmlReader.hasError()) {

    if(xmlReader.tokenType() == QXmlStreamReader::StartElement) {
      if(xmlReader.name() == "name") {
	QString name(xmlReader.readElementText());
	setName(name);
      } else if(xmlReader.name() == "desc") {
	QString description(xmlReader.readElementText());
	setDescription(description);
      } else if(xmlReader.name() == "url") {
	QString url(xmlReader.readElementText());
	setUrl(url);
      } else {
	qDebug() << __FUNCTION__ << "Unexpected element: " << xmlReader.name();
	drop(xmlReader);
      }
    }
    xmlReader.readNext();
  }

  return !name().isEmpty() && !url().isEmpty();
}

QDebug operator<<(QDebug dbg, const Image &i) {
  dbg.nospace() << "Image " << i.name() << " " << 
    i.description() << " " << i.url();
  return dbg;
}

//------------------- Waypoint -------------------

Waypoint::Waypoint() {
  m_type = Unknown;
}

void Waypoint::setComment(const QString &cmt) {
  m_comment = cmt;
}

void Waypoint::setType(const Type &type) {
  m_type = type;
}

Waypoint::Type Waypoint::type() const {
  return m_type;
}

QString Waypoint::iconFile() const {
  int i;

  static const struct { Waypoint::Type type; QString str; } tags[] = {
    { Waypoint::Multistage,   "multistage" },
    { Waypoint::Parking,      "parking" },
    { Waypoint::Final,        "final" },
    { Waypoint::Question,     "question" },
    { Waypoint::Trailhead,    "trailhead" },
    { Waypoint::Refpoint,     "refpoint" },
    { Waypoint::Custom,       "custom" },
    { Waypoint::Unknown,      "<unknown>" }
  };

  for(i=0;(tags[i].type!=Waypoint::Unknown) && (tags[i].type != m_type);i++); 
     
  return "wpt_" + tags[i].str;
}

QString Waypoint::comment() const {
  return m_comment;
}

void Waypoint::save(QXmlStreamWriter *xmlWriter) {
  xmlWriter->writeStartElement("wpt");
  xmlWriter->writeAttribute("type", QString::number(type()));
  xmlWriter->writeAttribute("lat", QString::number(coordinate().latitude(), 'g', 9));
  xmlWriter->writeAttribute("lon", QString::number(coordinate().longitude(), 'g', 9));
  
  writeTextElement(xmlWriter, "name", name());
  writeTextElement(xmlWriter, "desc", description());
  writeTextElement(xmlWriter, "cmt", comment());
  
  xmlWriter->writeEndElement();
}

bool Waypoint::load(QXmlStreamReader &xmlReader) {
  bool retval = true;
 
  QXmlStreamAttributes attr = xmlReader.attributes();
  QGeoCoordinate coo;
  coo.setLatitude(attr.value("lat").toString().toFloat());
  coo.setLongitude(attr.value("lon").toString().toFloat());
  setCoordinate(coo);

  // previous versions didn't export the type of custom waypoints
  if(attr.hasAttribute("type")) 
    setType((Waypoint::Type)(attr.value("type").toString().toInt()));
  else
    setType(Waypoint::Custom);

  xmlReader.readNext();

  // parse contents
  while(xmlReader.tokenType() != QXmlStreamReader::EndElement &&
	!xmlReader.atEnd() && !xmlReader.hasError()) {

    if(xmlReader.tokenType() == QXmlStreamReader::StartElement) {
      if(xmlReader.name() == "name") {
	QString name(xmlReader.readElementText());
	setName(name);
      } else if(xmlReader.name() == "desc") {
	QString desc(xmlReader.readElementText());
	setDescription(desc);
      } else if(xmlReader.name() == "cmt") {
	QString comment(xmlReader.readElementText());
	setComment(comment);
      } else {
	qDebug() << __FUNCTION__ << "Unexpected element: " << xmlReader.name();
	drop(xmlReader);
      }
    }
    
    xmlReader.readNext();
  }

  return retval;
}

QDebug operator<<(QDebug dbg, const Waypoint &w) {
  dbg.nospace() << "Waypoint " << w.iconFile() << " " << w.name() << " " << w.coordinate() << "\n";
  dbg.nospace() << "    " << w.description() << " " << w.comment();
  return dbg;
}

//------------------- Attribute -------------------

Attribute::Attribute(const Id &id, bool yes) {
  m_id = id;
  m_yes = yes;
}

QString Attribute::toString() const {
  int i;

  static const struct { Attribute::Id type; QString strYes, strNo; } tags[] = {
     { Attribute::Dogs,
       QObject::tr("Dogs Allowed"),                  QObject::tr("No Dogs Allowed") },
     { Attribute::Fee,
       QObject::tr("Access or Parking Fee"),         QObject::tr("No Access or Parking Fee") },
     { Attribute::Rappelling,
       QObject::tr("Climbing Gear Required"),        QObject::tr("No Climbing Gear Required") },
     { Attribute::Boat,
       QObject::tr("Boat Required"),                 QObject::tr("No Boat Required") },
     { Attribute::Scuba,
       QObject::tr("Scuba Gear Required"),           QObject::tr("No Scuba Gear Required") },
     { Attribute::Kids,
       QObject::tr("Recommended for Kids"),          QObject::tr("Not Recommended for Kids") },
     { Attribute::OneHour,
       QObject::tr("Takes Less Than an Hour"),       QObject::tr("Takes More Than an Hour") },
     { Attribute::Scenic,
       QObject::tr("Scenic View"),                   QObject::tr("No Scenic View") },
     { Attribute::Hiking,
       QObject::tr("Hiking Required"),               QObject::tr("No Hiking Required") },
     { Attribute::Climbing,
       QObject::tr("Difficult Climbing"),            QObject::tr("No Difficult Climbing") },
     { Attribute::Wading,
       QObject::tr("May Require Wading"),            QObject::tr("Does not Require Wading") },
     { Attribute::Swimming,
       QObject::tr("May Require Swimming"),          QObject::tr("Does not Require Swimming") },
     { Attribute::Available,
       QObject::tr("Available at All Times"),        QObject::tr("Not Available at All Times") },
     { Attribute::Night,
       QObject::tr("Recommended at Night"),          QObject::tr("Not Recommended at Night") },
     { Attribute::Winter,
       QObject::tr("Available During Winter"),       QObject::tr("Not Available During Winter") },
     { Attribute::PoisonOak,
       QObject::tr("Poisonous Plants"),              QObject::tr("No Poisonous Plants") },
     { Attribute::Cactus,
       QObject::tr("Cactus"),                        QObject::tr("No Cactus") },
     { Attribute::Snakes,
       QObject::tr("Dangerous Animals"),             QObject::tr("No Dangerous Animals") },
     { Attribute::Ticks,
       QObject::tr("Ticks"),                         QObject::tr("No Ticks") },
     { Attribute::Mine,
       QObject::tr("Abandoned Mines"),               QObject::tr("No Abandoned Mines") },
     { Attribute::Cliff,
       QObject::tr("Cliff / Falling Rocks"),         QObject::tr("No Cliff / Falling Rocks") },
     { Attribute::Hunting,
       QObject::tr("Hunting"),                       QObject::tr("No Hunting") },
     { Attribute::Danger,
       QObject::tr("Dangerous Area"),                QObject::tr("No Dangerous Area") },
     { Attribute::WheelChair,
       QObject::tr("Wheelchair Accessible"),         QObject::tr("Not Wheelchair Accessible") },
     { Attribute::Parking,
       QObject::tr("Parking Available"),             QObject::tr("No Parking Available") },
     { Attribute::Public,
       QObject::tr("Public Transportation Nearby"),  QObject::tr("No Public Transportation Nearby") },
     { Attribute::Water,
       QObject::tr("Drinking Water Nearby"),         QObject::tr("No Drinking Water Nearby") },
     { Attribute::Restrooms,
       QObject::tr("Public Restrooms Nearby"),       QObject::tr("No Public Restrooms Nearby") },
     { Attribute::Phone,
       QObject::tr("Public Phone Nearby"),           QObject::tr("No Public Phone Nearby") },
     { Attribute::Picnic,
       QObject::tr("Picnic Tables Nearby"),          QObject::tr("No Picnic Tables Nearby") },
     { Attribute::Camping,
       QObject::tr("Camping Available"),             QObject::tr("Camping Not Available") },
     { Attribute::Bicycles,
       QObject::tr("Bicycles Allowed"),              QObject::tr("Bicycles Not Allowed") },
     { Attribute::Motorcycles,
       QObject::tr("Motorcycles Allowed"),           QObject::tr("Motorcycles Not Allowed") },
     { Attribute::Quads,
       QObject::tr("Quads Allowed"),                 QObject::tr("Quads Not Allowed") },
     { Attribute::Jeeps,
       QObject::tr("Off-Road Vehicles Allowed"),     QObject::tr("Off-Road Vehicles Not Allowed") },
     { Attribute::Snowmobiles,
       QObject::tr("Snowmobiles Allowed"),           QObject::tr("Snowmobiles Not Allowed") },
     { Attribute::Horses,
       QObject::tr("Horses Allowed"),                QObject::tr("Horses Not Allowed") },
     { Attribute::Campfires,
       QObject::tr("Campfires Allowed"),             QObject::tr("Campfires Not Allowed") },
     { Attribute::Thorn,
       QObject::tr("Thorns!"),                       QObject::tr("No Thorns") },
     { Attribute::Stealth,
       QObject::tr("Stealth Required"),              QObject::tr("No Stealth Required") },
     { Attribute::Stroller,
       QObject::tr("Stroller Accessible"),           QObject::tr("Not Stroller Accessible") },
     { Attribute::FirstAid,
       QObject::tr("Needs Maintenance"),             QObject::tr("Does Not Need Maintenance") },
     { Attribute::Cow,
       QObject::tr("Watch for Livestock"),           QObject::tr("Don't Watch for Livestock") },
     { Attribute::Flashlight,
       QObject::tr("Flashlight Required"),           QObject::tr("No Flashlight Required") },
     { Attribute::LandF,
       QObject::tr("Lost & Found Cache"),            QObject::tr("No Lost & Found Cache") },
     { Attribute::RV,
       QObject::tr("Truck Driver/RV Allowed"),       QObject::tr("Truck Driver/RV Not Allowed") },
     { Attribute::FieldPuzzle,
       QObject::tr("Field Puzzle"),                  QObject::tr("No Field Puzzle") },
     { Attribute::UV,
       QObject::tr("UV Light Required"),             QObject::tr("No UV Light Required") },
     { Attribute::Snowshoes,
       QObject::tr("Snowshoes Required"),            QObject::tr("No Snowshoes Required") },
     { Attribute::Skiis,
       QObject::tr("Cross Country Skis Required"),   QObject::tr("No Cross Country Skis Required") },
     { Attribute::STool,
       QObject::tr("Special Tool Required"),         QObject::tr("No Special Tool Required") },
     { Attribute::NightCache,
       QObject::tr("Night Cache"),                   QObject::tr("No Night Cache") },
     { Attribute::ParkNGrab,
       QObject::tr("Park And Grab"),                 QObject::tr("No Park And Grab") },
     { Attribute::AbandonedBuilding,
       QObject::tr("Abandoned Structure"),           QObject::tr("No Abandoned Structure") },
     { Attribute::HikeShort,
       QObject::tr("Hike Shorter than 1km"),         QObject::tr("Hike Longer than 1km") },
     { Attribute::HikeMed,
       QObject::tr("Hike Shorter than 10km"),        QObject::tr("Hike Longer than 10km") },
     { Attribute::HikeLong,
       QObject::tr("Hike Longer than 10km"),         QObject::tr("Hike Shorter than 10km") },
     { Attribute::Fuel,
       QObject::tr("Fuel Nearby"),                   QObject::tr("No Fuel Nearby") },
     { Attribute::Food,
       QObject::tr("Food Nearby"),                   QObject::tr("No Food Nearby") },
     { Attribute::WirelessBeacon,
       QObject::tr("Wireless Beacon"),               QObject::tr("No Wireless Beacon")  },
     { Attribute::PartnerShip,
       QObject::tr("Partnership Cache"),             QObject::tr("No Partnership Cache")  },
     { Attribute::Seasonal,
       QObject::tr("Seasonal Access"),               QObject::tr("Access at all Seasons")  },
     { Attribute::TouristOk,
       QObject::tr("Tourist Friendly"),              QObject::tr("Not Tourist Friendly")  },
     { Attribute::TreeClimbing,
       QObject::tr("Tree Climbing"),                 QObject::tr("No Tree Climbing")  },
     { Attribute::FrontYard,
       QObject::tr("Front Yard (Private Residence)"),QObject::tr("No Front Yard (No Private Residence)")  },
     { Attribute::TeamWork,
       QObject::tr("Teamwork Required"),             QObject::tr("No Teamwork Required")  },
     { Attribute::Unknown,
       QObject::tr("<unknown>"),                     QObject::tr("<not unknown>")  }
  };

  for(i=0;(tags[i].type != Attribute::Unknown) && (tags[i].type != m_id);i++); 
     
  return m_yes?tags[i].strYes:tags[i].strNo;
}

QStringList Attribute::iconFile() const {
  int i;

  static const struct { Attribute::Id type; QString str; } tags[] = {
    { Attribute::Dogs,              "dogs" },
    { Attribute::Fee,               "fee" },
    { Attribute::Rappelling,        "rappelling" },
    { Attribute::Boat,              "boat" },
    { Attribute::Scuba,             "scuba" },
    { Attribute::Kids,              "kids" },
    { Attribute::OneHour,           "onehour" },
    { Attribute::Scenic,            "scenic" },
    { Attribute::Hiking,            "hiking" },
    { Attribute::Climbing,          "climbing" },
    { Attribute::Wading,            "wading" },
    { Attribute::Swimming,          "swimming" },
    { Attribute::Available,         "available" },
    { Attribute::Night,             "night" },
    { Attribute::Winter,            "winter" },
    { Attribute::Cactus,            "cactus" },
    { Attribute::PoisonOak,         "poisonoak" },
    { Attribute::Snakes,            "snakes" },
    { Attribute::Ticks,             "ticks" },
    { Attribute::Mine,              "mine" },
    { Attribute::Cliff,             "cliff" },
    { Attribute::Hunting,           "hunting" },
    { Attribute::Danger,            "danger" },
    { Attribute::WheelChair,        "wheelchair" },
    { Attribute::Parking,           "parking" },
    { Attribute::Public,            "public" },
    { Attribute::Water,             "water" },
    { Attribute::Restrooms,         "restrooms" },
    { Attribute::Phone,             "phone" },
    { Attribute::Picnic,            "picnic" },
    { Attribute::Camping,           "camping" },
    { Attribute::Bicycles,          "bicycles" },
    { Attribute::Motorcycles,       "motorcycles" },
    { Attribute::Quads,             "quads" },
    { Attribute::Jeeps,             "jeeps" },
    { Attribute::Snowmobiles,       "snowmobiles" },
    { Attribute::Horses,            "horses" },
    { Attribute::Campfires,         "campfires" },
    { Attribute::Thorn,             "thorn" },
    { Attribute::Stealth,           "stealth" },
    { Attribute::Stroller,          "stroller" },
    { Attribute::FirstAid,          "firstaid" },
    { Attribute::Cow,               "cow" },
    { Attribute::Flashlight,        "flashlight" },
    { Attribute::LandF,             "landf" },
    { Attribute::RV,                "rv" },
    { Attribute::FieldPuzzle,       "field_puzzle" },
    { Attribute::UV,                "UV" },
    { Attribute::Snowshoes,         "snowshoes" },
    { Attribute::Skiis,             "skiis" },
    { Attribute::STool,             "s_tool" },
    { Attribute::NightCache,        "nightcache" },
    { Attribute::ParkNGrab,         "parkngrab" },
    { Attribute::AbandonedBuilding, "abandonedbuilding" },
    { Attribute::HikeShort,         "hike_short" },
    { Attribute::HikeMed,           "hike_med" },
    { Attribute::HikeLong,          "hike_long" },
    { Attribute::Fuel,              "fuel" },
    { Attribute::Food,              "food" },
    { Attribute::WirelessBeacon,    "wirelessbeacon" },
    { Attribute::PartnerShip,       "partnership" },
    { Attribute::Seasonal,          "seasonal" },
    { Attribute::TouristOk,         "touristok" },
    { Attribute::TreeClimbing,      "treeclimbing" },
    { Attribute::FrontYard,         "frontyard" },
    { Attribute::TeamWork,          "teamwork" },
    { Attribute::Unknown,           "<unknown>" }
  };
  
  for(i=0;(tags[i].type != Attribute::Unknown) && (tags[i].type != m_id);i++); 
  
  QStringList list("att_" + tags[i].str);
  if(!m_yes) list << "att_no";
  return list;
}

Attribute::Id Attribute::id() const {
  return m_id;
}

bool Attribute::isOn() const {
  return m_yes;
}

QDebug operator<<(QDebug dbg, const Attribute &a) {
  dbg.nospace() << "Attribute=" << a.toString();
  return dbg;
}

//------------------- Log -------------------

Log::Type::Type() {
  m_id = Log::Type::Unknown;  
}

void Log::Type::set(Log::Type::Id id) {
  m_id = id;
}

bool Log::Type::is(Log::Type::Id id) const {
  return m_id == id;
}

Log::Type::Id Log::Type::id() const {
  return m_id;
}

QString Log::Type::toString(Type::Id id) {
  int i;

  static const struct { Log::Type::Id type; QString str; } tags[] = {
    { Log::Type::Found,              QObject::tr("Found it") },
    { Log::Type::NotFound,           QObject::tr("Didn't find it") },
    { Log::Type::Maintenance,        QObject::tr("Owner Maintenance") },
    { Log::Type::WriteNote,          QObject::tr("Write Note") },
    { Log::Type::ReviewerNote,       QObject::tr("Post Reviewer Note") },
    { Log::Type::EnableListing,      QObject::tr("Enable Listing") },
    { Log::Type::PublishListing,     QObject::tr("Publish Listing") },
    { Log::Type::WillAttend,         QObject::tr("Will Attend") },
    { Log::Type::Attended,           QObject::tr("Attended") },
    { Log::Type::Photo,              QObject::tr("Webcam Photo taken") },
    { Log::Type::TempDisable,        QObject::tr("Temporarily Disable Listing") },
    { Log::Type::NeedsMaintenance,   QObject::tr("Needs Maintenance") },
    { Log::Type::UpdatedCoordinates, QObject::tr("Update Coordinates") },
    { Log::Type::Unarchive,          QObject::tr("Unarchive") },
    { Log::Type::NeedsArchived,      QObject::tr("Needs Archived") },
    { Log::Type::Archive,            QObject::tr("Archive") },
    { Log::Type::Unknown,            QObject::tr("<unknown>") }
  };

  for(i=0;(tags[i].type!=Log::Type::Unknown) && (tags[i].type != id);i++); 
     
  return tags[i].str;
}

QString Log::Type::toString() const {
  return toString(m_id);
}

QString Log::Type::iconFile(Type::Id id) {
  int i;

  static const struct { Log::Type::Id type; QString str; } tags[] = {
    { Log::Type::Found,              "smile" },
    { Log::Type::NotFound,           "sad" },
    { Log::Type::Maintenance,        "maint" },
    { Log::Type::WriteNote,          "note" },
    { Log::Type::ReviewerNote,       "big_smile" },
    { Log::Type::EnableListing,      "enabled" },
    { Log::Type::PublishListing,     "greenlight" },
    { Log::Type::WillAttend,         "rsvp" },
    { Log::Type::Attended,           "attended" },
    { Log::Type::Photo,              "camera" },
    { Log::Type::TempDisable,        "disabled" },
    { Log::Type::NeedsMaintenance,   "needsmaint" },
    { Log::Type::UpdatedCoordinates, "coord_update" },
    { Log::Type::Unarchive,          "traffic_cone" },
    { Log::Type::NeedsArchived,      "traffic_cone" },
    { Log::Type::Archive,            "traffic_cone" },
    { Log::Type::Unknown,            "<unknown>" }
  };

  for(i=0;(tags[i].type!=Log::Type::Unknown) && (tags[i].type != id);i++); 
     
  return "log_icon_" + tags[i].str;
}

QString Log::Type::iconFile() const {
  return iconFile(m_id);
}

Log::Log() {
}

void Log::setFinder(const QString &finder) {
  m_finder = finder;  
}

QString Log::finder() const {
  return m_finder;  
}

void Log::setType(const Log::Type &type) {
  m_type = type;  
}

void Log::setType(Log::Type::Id id) {
  m_type.set(id);  
}

Log::Type Log::type() const {
  return m_type;
}

void Log::setDate(const QDate &date) {
  m_date = date;
}

QDate Log::date() const {
  return m_date;
}

void Log::setDescription(const Description &description) {
  m_description = description;
}

Description Log::description() const {
  return m_description;
}

void Log::save(QXmlStreamWriter *xmlWriter) {
  xmlWriter->writeStartElement("log");
  xmlWriter->writeAttribute("type", QString::number(type().id()));
  
  writeTextElement(xmlWriter, "finder", finder());
  description().save(xmlWriter, "desc");
  writeDateElement(xmlWriter, "date", date());
  
  xmlWriter->writeEndElement();
}

bool Log::load(QXmlStreamReader &xmlReader) {
  bool retval = true;
 
  QXmlStreamAttributes attr = xmlReader.attributes();
  if(attr.hasAttribute("type")) 
    setType((Log::Type::Id)(attr.value("type").toString().toInt()));

  xmlReader.readNext();

  // parse contents
  while(xmlReader.tokenType() != QXmlStreamReader::EndElement &&
	!xmlReader.atEnd() && !xmlReader.hasError()) {

    if(xmlReader.tokenType() == QXmlStreamReader::StartElement) {
      if(xmlReader.name() == "finder") {
	QString finder(xmlReader.readElementText());
	setFinder(finder);
      } else if(xmlReader.name() == "desc")
	m_description.load(xmlReader);
      else if(xmlReader.name() == "date") {
	QDate date;
	if(loadDate(xmlReader, date))
	  setDate(date);
      } else {
	qDebug() << __FUNCTION__ << "Unexpected element: " << xmlReader.name();
	drop(xmlReader);
      }
    }
    
    xmlReader.readNext();
  }

  return retval;
}

QDebug operator<<(QDebug dbg, const Log &l) {
  dbg.nospace() << "Log type=" << l.type().toString();
  
  if(l.date().isValid())
    dbg.nospace() << " date=" << l.date().toString();

  if(!l.finder().isEmpty())
    dbg.nospace() << " finder=" << l.finder();

  if(l.description().isSet())
    dbg.nospace() << "\n  " << l.description();

  return dbg;
}

//------------------- Description -------------------

Description::Description() {
  this->m_isHtml = false;
}

void Description::set(bool isHtml, const QString &text) {
  this->m_text = text;
  this->m_isHtml = isHtml;
}

QString Description::text() const {
  return this->m_text;
}

bool Description::isHtml() const {
  return this->m_isHtml;
}

bool Description::isSet() const {
  return !this->m_text.isEmpty();
}

void Description::save(QXmlStreamWriter *xmlWriter, const QString &name) {
  if(isSet()) {
    xmlWriter->writeStartElement(name);
    xmlWriter->writeAttribute("html", isHtml()?"true":"false");
    xmlWriter->writeCharacters(m_text);
    xmlWriter->writeEndElement();
  }
}

void Description::load(QXmlStreamReader &xmlReader) {
  QXmlStreamAttributes attr = xmlReader.attributes();

  if(attr.hasAttribute("html")) 
    m_isHtml = (attr.value("html").toString().compare("true", Qt::CaseInsensitive) == 0);

  m_text = xmlReader.readElementText();
}

QDebug operator<<(QDebug dbg, const Description &d) {
  dbg.nospace() << d.text();
  dbg.nospace() << " (html=" << d.isHtml() << ")";
  return dbg;
}

//------------------- Tribool -------------------
Tribool::Tribool() {
  // no need to set m_value
  this->m_set = false;
}

void Tribool::set(bool val) {
  this->m_set = true;
  this->m_value = val;
}

bool Tribool::value() const {
  Q_ASSERT(this->m_set);
  return this->m_value;
}

bool Tribool::isSetAndTrue() const {
  if(!this->m_set) return false;
  return this->m_value;
}

bool Tribool::isSet() const {
  return this->m_set;
}

void Tribool::saveAttribute(QXmlStreamWriter *xmlWriter, const QString &name) {
  if(isSet())
    xmlWriter->writeAttribute(name, isSetAndTrue()?"true":"false");
}

void Tribool::loadAttribute(QXmlStreamAttributes &attr, const QString &name) {
  if(attr.hasAttribute(name)) 
    set(attr.value(name).toString().compare("true", Qt::CaseInsensitive) == 0);
}

QDebug operator<<(QDebug dbg, const Tribool &t) {
  if(!t.isSet()) dbg.nospace() << "?";
  else           dbg.nospace() << t.value();
  return dbg;
}

//------------------- Rating -------------------
Rating::Rating() {
  this->m_value = 0;
}

void Rating::set(qreal val) {
  this->m_value = val*2;
}

qreal Rating::value() const {
  Q_ASSERT(this->m_value);
  return this->m_value/2.0;
}

bool Rating::isSet() const {
  return(this->m_value != 0);
}

QString Rating::toString() const {
  QString ret;

  if(!isSet()) return("?");

  int i, j = 2*value();
  for(i=2;i<=10;i+=2) {
    if(j < i-1)    ret += "-";
    else if(j < i) ret += "+";
    else           ret += "*";
  }
  return ret;
}

void Rating::save(QXmlStreamWriter *xmlWriter, const QString &name) {
  if(isSet()) 
    writeTextElement(xmlWriter, name, QString::number(value(), 'g', 2));
}

void Rating::load(QXmlStreamReader &xmlReader) {
  QString str(xmlReader.readElementText());
  set(str.toFloat());
}

QDebug operator<<(QDebug dbg, const Rating &t) {
  dbg.nospace() << t.toString();
  return dbg;
}

//------------------- Container -------------------

Container::Container() : m_value(ContainerUnknown) {
}

void Container::set(Type type) {
  m_value = type;
}

Container::Type Container::get() const {
  return m_value;
}

QString Container::toString(Container::Type type) {
  int i;

  static const struct { Type type; QString str; } tags[] = {
    { ContainerRegular,      "Regular" }, { ContainerSmall,   "Small" },
    { ContainerMicro,          "Micro" }, { ContainerOther,   "Other" },
    { ContainerNotChosen, "Not Chosen" }, { ContainerLarge,   "Large" },
    { ContainerVirtual,      "Virtual" }, { ContainerUnknown,     "?" }
  };

  for(i=0;(tags[i].type!=ContainerUnknown) && (tags[i].type != type);i++); 
     
  return tags[i].str;
}

QString Container::toString() const {
  return toString(m_value);
}

QString Container::iconFile(Container::Type type, bool simple) {
  int i;

  static const struct { Type type; QString str; } tags[] = {
    { ContainerRegular,      "regular" }, { ContainerSmall,   "small" },
    { ContainerMicro,          "micro" }, { ContainerOther,   "other" },
    { ContainerNotChosen, "not_chosen" }, { ContainerLarge,   "large" },
    { ContainerVirtual,      "virtual" }, { ContainerUnknown, "other" }
  };

  for(i=0;(tags[i].type!=ContainerUnknown) && (tags[i].type != type);i++); 
     
  return "cache_size_" + QString(simple?"simple_":"") + tags[i].str;
}

QString Container::iconFile(bool simple) const {
  return iconFile(m_value, simple);
}

bool Container::isSet() const {
  return m_value != ContainerUnknown;
}

void Container::save(QXmlStreamWriter *xmlWriter, const QString &name) {
  if(isSet()) 
    writeTextElement(xmlWriter, name, QString::number(m_value));
}

void Container::load(QXmlStreamReader &xmlReader) {
  QString str(xmlReader.readElementText());
  set((Container::Type)(str.toInt()));
}

QDebug operator<<(QDebug dbg, const Container &c) {
  dbg.nospace() << c.toString();
  return dbg;
}

//------------------- Cache -------------------

Cache::Cache() :
  m_type(TypeUnknown),
  m_id(0) {
}

Cache::~Cache() {
}

// set "owned" attribute
void Cache::setOwned(bool owned) {
  this->m_owned.set(owned);
}

Tribool Cache::owned() const {
  return this->m_owned;
}

void Cache::setFound(bool found) {
  this->m_found.set(found);
}

Tribool Cache::found() const {
  return this->m_found;
}

void Cache::setAvailable(bool available) {
  this->m_available.set(available);
}

Tribool Cache::available() const {
  return this->m_available;
}

void Cache::setId(int id) {
  this->m_id = id;
}

int Cache::id() const {
  return this->m_id;
}

void Cache::setType(Type type) {
  this->m_type = type;
}

Cache::Type Cache::type() const {
  return this->m_type;
}

QString Cache::typeString(Type rtype) {
  int i;

  struct { Type type; QString str; } tags[] = {
    { TypeTraditional, "Traditional" },   { TypeMulti,       "Multi" },
    { TypeMystery,     "Mystery" },       { TypeVirtual,     "Virtual" },
    { TypeWebcam,      "Webcam" },        { TypeEvent,       "Event" },
    { TypeLetterbox,   "Letterbox" },     { TypeEarthcache,  "Earthcache" },
    { TypeWherigo,     "Wherigo" },       { TypeMegaEvent,   "Mega-Event" },
    { TypeCito,   "Cache In Trash Out" }, { TypeUnknown,     "?" }
  };

  for(i=0;(tags[i].type!=TypeUnknown) && (tags[i].type != rtype);i++); 
     
  return tags[i].str;
}


QString Cache::typeString() const {
  return typeString(m_type);
}

QString Cache::typeIconFile(Type rtype) {
  int i;

  static const struct { Type type; QString str; } tags[] = {
    { TypeTraditional, "traditional" }, { TypeMulti,       "multi" },
    { TypeMystery,     "mystery" },     { TypeVirtual,     "virtual" },
    { TypeWebcam,      "webcam" },      { TypeEvent,       "event" },
    { TypeLetterbox,   "letterbox" },   { TypeEarthcache,  "earthcache" },
    { TypeWherigo,     "wherigo" },     { TypeMegaEvent,   "megaevent" },
    { TypeCito,        "cito" },        { TypeUnknown,     "unknown" }
  };

  for(i=0;(tags[i].type!=TypeUnknown) && (tags[i].type != rtype);i++); 
     
  return "cache_type_" + tags[i].str;
}


QString Cache::typeIconFile() const {
  return typeIconFile(m_type);
}

QString Cache::overlayIconFile() const {
  if(this->m_available.isSet() && !this->m_available.value())
    return "cache_type_ovl_unavailable";
  
  if(this->m_owned.isSet() && this->m_owned.value())
    return "cache_type_ovl_mine";

  if(this->m_found.isSet() && this->m_found.value())
    return "cache_type_ovl_found";

  return NULL;
}


void Cache::setDifficulty(qreal val) {
  this->m_difficulty.set(val);
}

Rating Cache::difficulty() const {
  return this->m_difficulty;
}

void Cache::setTerrain(qreal val) {
  this->m_terrain.set(val);
}

Rating Cache::terrain() const {
  return this->m_terrain;
}

void Cache::setContainer(Container::Type val) {
  this->m_container.set(val);
}

Container Cache::container() const {
  return this->m_container;
}

void Cache::setGuid(const QString &guid) {
  this->m_guid = guid;
}

QString Cache::guid() const {
  return this->m_guid;
}

void Cache::setOwner(const QString &owner) {
  m_owner = owner;
}

QString Cache::owner() const {
  return m_owner;
}

void Cache::setDateOfPlacement(const QDate &date) {
  m_dateOfPlacement = date;
}

QDate Cache::dateOfPlacement() const {
  return m_dateOfPlacement;
}

QString Cache::dateOfPlacementString() const {
  if(m_dateOfPlacement.isNull() || !m_dateOfPlacement.isValid())
    return "?";

  return m_dateOfPlacement.toString();
}

void Cache::setShortDescription(const Description &shortDescription) {
  m_shortDescription = shortDescription;
}

Description Cache::shortDescription() const {
  return m_shortDescription;
}

void Cache::setLongDescription(const Description &longDescription) {
  m_longDescription = longDescription;
}

Description Cache::longDescription() const {
  return m_longDescription;
}

void Cache::setHint(const Description &hint) {
  m_hint = hint;
}

Description Cache::hint() const {
  return m_hint;
}

QList<Log> Cache::logs() const {
  return m_logs;
}

void Cache::appendLog(const Log &log) {
  m_logs.append(log);
}

void Cache::clearLogs() {
  m_logs.clear();
}

QList<Attribute> Cache::attributes() const {
  return m_attributes;
}

void Cache::appendAttribute(const Attribute &attribute) {
  m_attributes.append(attribute);
}

void Cache::clearAttributes() {
  m_attributes.clear();
}

QList<Waypoint> Cache::waypoints() const {
  return m_waypoints;
}

void Cache::appendWaypoint(const Waypoint &waypoint) {
  m_waypoints.append(waypoint);
}

void Cache::updateWaypoint(const Waypoint &waypoint) {
  QList<Waypoint>::iterator i;
  for( i = m_waypoints.begin(); i != m_waypoints.end(); ++i ) 
    if(i->name() == waypoint.name())
      *i = waypoint;
}

void Cache::clearWaypoints() {
  m_waypoints.clear();
}

QList<Image> Cache::images() const {
  return m_images;
}

void Cache::appendImage(const Image &image) {
  m_images.append(image);
}

void Cache::clearImages() {
  m_images.clear();
}

QDebug operator<<(QDebug debug, const Cache &c) {
  debug.nospace() << "Cache(" << c.name() << 
    ", " << c.coordinate() << ", " << c.description() << ",\n  ";
  debug.nospace() << "placed at " << c.dateOfPlacementString() << " by " << c.owner() << ",\n  ";
  if(c.m_shortDescription.isSet()) debug.nospace() << c.m_shortDescription  << ",\n  ";
  if(c.m_longDescription.isSet()) debug.nospace() << c.m_longDescription  << ",\n  ";
  if(c.url().isValid()) debug.nospace() << "url=" << c.url()  << ",\n  ";
  if(!c.guid().isEmpty()) debug.nospace() << "guid=" << c.guid() << ",\n  ";
  debug.nospace() << "diffculty=" << c.m_difficulty << 
    ", terrain=" << c.m_terrain << ", container=" << c.m_container << ",\n  ";
  debug.nospace() << 
    "type=" << c.typeString() << ", id=" << c.id() << 
    ", found=" << c.m_found << ", owned=" << c.m_owned << 
    ", available=" << c.m_available << ")\n";

  foreach(Log log, c.logs()) 
    debug.nospace() << log << endl;

  foreach(Attribute attribute, c.attributes()) 
    debug.nospace() << attribute << endl;

  foreach(Waypoint waypoint, c.waypoints()) 
    debug.nospace() << waypoint << endl;

  foreach(Image image, c.images()) 
    debug.nospace() << image << endl;

  return debug;
}

bool Cache::save(QXmlStreamWriter *xmlWriter) {
  xmlWriter->writeStartElement("cache");
  xmlWriter->writeAttribute("type", QString::number(type()));
  xmlWriter->writeAttribute("lat", QString::number(coordinate().latitude(), 'g', 9));
  xmlWriter->writeAttribute("lon", QString::number(coordinate().longitude(), 'g', 9));

  found().saveAttribute(xmlWriter, "found");
  owned().saveAttribute(xmlWriter, "owned");
  available().saveAttribute(xmlWriter, "available");

  writeTextElement(xmlWriter, "name", name());
  writeTextElement(xmlWriter, "desc", description());
  writeTextElement(xmlWriter, "owner", owner());
  writeTextElement(xmlWriter, "url", url().toString());

  m_difficulty.save(xmlWriter, "difficulty");
  m_terrain.save(xmlWriter, "terrain");
  m_container.save(xmlWriter, "container");

  writeDateElement(xmlWriter, "date", dateOfPlacement());

  writeTextElement(xmlWriter, "guid", guid());

  m_shortDescription.save(xmlWriter, "short_description");
  m_longDescription.save(xmlWriter, "long_description");
  m_hint.save(xmlWriter, "hint");

  // build list of attribute ids
  QStringList attIds;
  foreach(Attribute att, attributes())
    attIds << (att.isOn()?"":"n") + QString::number(att.id());
  writeTextElement(xmlWriter, "attributes", attIds.join(","));
  
  if(waypoints().size() > 0) {
    xmlWriter->writeStartElement("waypoints");
    foreach(Waypoint wpt, waypoints())
      wpt.save(xmlWriter);
    xmlWriter->writeEndElement();
  }

  if(logs().size() > 0) {
    xmlWriter->writeStartElement("logs");
    foreach(Log log, logs())
      log.save(xmlWriter);
    xmlWriter->writeEndElement();
  }

  if(images().size() > 0) {
    xmlWriter->writeStartElement("images");
    foreach(Image image, images())
      image.save(xmlWriter);
    xmlWriter->writeEndElement();
  }

  xmlWriter->writeEndElement();

  return true;
}

bool Cache::loadWaypoints(QXmlStreamReader &xmlReader) {
  bool retval = true;
  xmlReader.readNext();

  // parse contents
  while(xmlReader.tokenType() != QXmlStreamReader::EndElement &&
	!xmlReader.atEnd() && !xmlReader.hasError()) {

    if(xmlReader.tokenType() == QXmlStreamReader::StartElement) {
      if(xmlReader.name() == "wpt") {
	Waypoint wpt;
	if(wpt.load(xmlReader))
	  appendWaypoint(wpt);
      } else {
	qDebug() << __FUNCTION__ << "Unexpected element: " << xmlReader.name();
	drop(xmlReader);
      }
    }
    
    xmlReader.readNext();
  }

  return retval;
}

bool Cache::loadLogs(QXmlStreamReader &xmlReader) {
  bool retval = true;
  xmlReader.readNext();

  // parse contents
  while(xmlReader.tokenType() != QXmlStreamReader::EndElement &&
	!xmlReader.atEnd() && !xmlReader.hasError()) {

    if(xmlReader.tokenType() == QXmlStreamReader::StartElement) {
      if(xmlReader.name() == "log") {
	Log log;
	if(log.load(xmlReader))
	  appendLog(log);
      } else {
	qDebug() << __FUNCTION__ << "Unexpected element: " << xmlReader.name();
	drop(xmlReader);
      }
    }
    
    xmlReader.readNext();
  }

  return retval;
}

bool Cache::loadImages(QXmlStreamReader &xmlReader) {
  bool retval = true;
  xmlReader.readNext();

  // parse contents
  while(xmlReader.tokenType() != QXmlStreamReader::EndElement &&
	!xmlReader.atEnd() && !xmlReader.hasError()) {

    if(xmlReader.tokenType() == QXmlStreamReader::StartElement) {
      if(xmlReader.name() == "image") {
	Image image;
	if(image.load(xmlReader))
	  appendImage(image);
      } else {
	qDebug() << __FUNCTION__ << "Unexpected element: " << xmlReader.name();
	drop(xmlReader);
      }
    }
    
    xmlReader.readNext();
  }

  return retval;
}

bool Cache::load(QXmlStreamReader &xmlReader) {
  bool retval = true;

  QXmlStreamAttributes attr = xmlReader.attributes();
  QGeoCoordinate coo;
  coo.setLatitude(attr.value("lat").toString().toFloat());
  coo.setLongitude(attr.value("lon").toString().toFloat());
  setType((Cache::Type)(attr.value("type").toString().toInt()));
  m_found.loadAttribute(attr, "found");
  m_owned.loadAttribute(attr, "owned");
  m_available.loadAttribute(attr, "available");
  setCoordinate(coo);

  xmlReader.readNext();

  // parse contents
  while(xmlReader.tokenType() != QXmlStreamReader::EndElement &&
	!xmlReader.atEnd() && !xmlReader.hasError()) {

    if(xmlReader.tokenType() == QXmlStreamReader::StartElement) {
      
      if(xmlReader.name() == "name") {
	QString name(xmlReader.readElementText());
	setName(name);
      } else if(xmlReader.name() == "desc") {
	QString desc(xmlReader.readElementText());
	setDescription(desc);
      } else if(xmlReader.name() == "url") {
	QString url(xmlReader.readElementText());
	setUrl(url);
      } else if(xmlReader.name() == "owner") 
	m_owner = xmlReader.readElementText();
      else if(xmlReader.name() == "guid") 
	m_guid = xmlReader.readElementText();
      else if(xmlReader.name() == "difficulty") 
	m_difficulty.load(xmlReader);
      else if(xmlReader.name() == "terrain") 
	m_terrain.load(xmlReader);
      else if(xmlReader.name() == "container") 
	m_container.load(xmlReader);
      else if(xmlReader.name() == "short_description") 
	m_shortDescription.load(xmlReader);
      else if(xmlReader.name() == "long_description") 
	m_longDescription.load(xmlReader);
      else if(xmlReader.name() == "hint") 
	m_hint.load(xmlReader);
      else if(xmlReader.name() == "attributes") {
	QStringList atts(xmlReader.readElementText(). split(","));
	foreach(QString attStr, atts) {
	  bool isOn = true;
	  int id;

	  if(attStr[0] == 'n') {
	    isOn = false;
	    id = attStr.right(attStr.length()-1).toInt();
	  } else
	    id = attStr.toInt();

	  appendAttribute(Attribute((Attribute::Id)id, isOn));
	}
      } else if(xmlReader.name() == "waypoints") {
	retval &= loadWaypoints(xmlReader);
      } else if(xmlReader.name() == "logs") {
	retval &= loadLogs(xmlReader);
      } else if(xmlReader.name() == "images") {
	retval &= loadImages(xmlReader);
      } else if(xmlReader.name() == "date") {
	QDate date;
	if(loadDate(xmlReader, date))
	  setDateOfPlacement(date);
      } else {
	qDebug() << __FUNCTION__ << "Unexpected element: " << xmlReader.name();
	drop(xmlReader);
      }
    }
    
    xmlReader.readNext();
  }

  return retval;
}
