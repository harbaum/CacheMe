#include <QDebug>
#include <QDir>

#include "waypointdb.h"
#include "filefinder.h"

WaypointDb::WaypointDb() {
  qDebug() << __FUNCTION__;

  read();
  qDebug() << __FUNCTION__ << "restored waypoints: " << m_wpts.size();
}

WaypointDb::~WaypointDb() {
  qDebug() << __FUNCTION__;
}

QString WaypointDb::fileName() {
  QString dataDir = FileFinder::dataPath();
  if(!QDir().mkpath(dataDir)) {
    qDebug() << __FUNCTION__ << "cannot create data dir" << dataDir;
    return NULL;
  }

  return QString(dataDir + "/waypoints.gpx");
}

bool WaypointDb::write() {
  QString gpxFileName(fileName());
  qDebug() << __FUNCTION__ << gpxFileName;
  if(gpxFileName.isNull()) return false;

  QFile gpxFile(gpxFileName);
  if(!gpxFile.open(QIODevice::WriteOnly | QFile::Text)) {
    qDebug() << __FUNCTION__ << "unable to open waypoint database for writing";
    return false;
  }

  QXmlStreamWriter xmlWriter(&gpxFile);
  xmlWriter.setAutoFormatting(true);
  xmlWriter.writeStartDocument();
  xmlWriter.writeStartElement("gpx");
  foreach(Waypoint wpt, m_wpts)
    wpt.save(&xmlWriter);

  xmlWriter.writeEndDocument();
 
  gpxFile.close();
  
  return true;
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

void WaypointDb::readGpx(QXmlStreamReader &xml) {
  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "wpt") {
	Waypoint wpt;
	if(wpt.load(xml))
	  m_wpts.append(wpt);
      } else {
	qDebug() << __FUNCTION__ << "Unexpected gpx element: " << xml.name();
	drop(xml);
      }
    }
    xml.readNext();
  }
}

bool WaypointDb::read() {
  QString gpxFileName(fileName());
  qDebug() << __FUNCTION__ << gpxFileName;

  if(gpxFileName.isNull()) return false;

  QFile gpxFile(gpxFileName);
  if(!gpxFile.open(QIODevice::ReadOnly | QFile::Text)) {
    qDebug() << __FUNCTION__ << "unable to open waypoint database for reading";
    return false;
  }

  m_wpts.clear();

  QXmlStreamReader xml(&gpxFile);

  while(!xml.atEnd() && !xml.hasError()) {
    xml.readNext();

    if(xml.tokenType() == QXmlStreamReader::StartDocument) 
      continue;

    if(xml.tokenType() == QXmlStreamReader::StartElement) {
      if(xml.name() == "gpx") 
	readGpx(xml);
      else {
	qDebug() << __FUNCTION__ << "Unexpected element: " << xml.name();
	drop(xml);
      }
    }
  }
  
  /* Error handling. */
  if(xml.hasError()) 
    qDebug() << __FUNCTION__ << QString("Parse error at line %1, column %2:\n%3")
      .arg(xml.lineNumber()).arg(xml.columnNumber()).arg(xml.errorString());

  xml.clear();
  gpxFile.close();
  
  return true;
}

static QString idNum(int id) {
  return QString("00").append(QString::number(id)).right(2);
}

QString WaypointDb::getUnusedId(const Cache &cache) {
  int index = 0;
  bool exists;
  QString newId;

  do {
    newId = cache.name() + "-" + idNum(index);

    QList<Waypoint>::const_iterator i;
    exists = false;
    for(i = m_wpts.constBegin(); i != m_wpts.constEnd() && !exists; ++i)
      if(i->name() == newId)
	exists = true;

    index++;
  } while(exists && index < 100);
  
  if(exists) return NULL;
  return newId;
}

QString WaypointDb::getUnusedName(const Cache &cache) {
  int index = 0;
  bool exists;
  QString newName;

  do {
    newName = cache.description() + " #" + idNum(index);

    QList<Waypoint>::const_iterator i;
    exists = false;
    for(i = m_wpts.constBegin(); i != m_wpts.constEnd() && !exists; ++i)
      if(i->description() == newName)
	exists = true;

    index++;
  } while(exists && index < 100);
  
  // we might indeed return an existing name which is no fundamental problem as
  // the ids are unique
  return newName;
}

QList<Waypoint> WaypointDb::get(const QString &name) {
  QList<Waypoint> result;

  QList<Waypoint>::const_iterator i;
  for(i = m_wpts.constBegin(); i != m_wpts.constEnd(); ++i)
    if(i->name().left(name.length()) == name)
      result.append(*i);

  return result;
}

void WaypointDb::append(Waypoint &wpt) {
  qDebug() << __FUNCTION__;

  wpt.setType(Waypoint::Custom);

  if(wpt.coordinate().isValid() && !wpt.name().isEmpty())
    m_wpts.append(wpt);

  write();
  emit waypointsChanged();
}

QList<Waypoint> WaypointDb::get(const Cache &cache) {
  return get(cache.name());
}

void WaypointDb::update(Waypoint &wpt) {
  qDebug() << __FUNCTION__;

  if(wpt.coordinate().isValid() && !wpt.name().isEmpty()) {
    bool found = false;

    for(int i=0;i<m_wpts.size();i++) {
      if(m_wpts[i].name() == wpt.name()) {
	m_wpts[i] = wpt;
	found = true;
      }
    }

    if(!found)
      append(wpt);
    else {
      write();
      emit waypointsChanged();
    }
  }
}

void WaypointDb::removeByName(const QString &name) {
  qDebug() << __FUNCTION__ << name;

  for(int i=0;i<m_wpts.size();i++) {
    if(m_wpts[i].description() == name) {
      m_wpts.removeAt(i);
      write();

      emit waypointsChanged();
      return;
    }
  }
  qDebug() << __FUNCTION__ << "Oops, not found!";
}
