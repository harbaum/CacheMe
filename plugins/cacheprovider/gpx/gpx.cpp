#include <QDebug>
#include <QtPlugin>
#include <QFileDialog>
#include <QSettings>
#include <QGroupBox>
#include <QLabel>
#include <QThread>

#include "gpx.h"

#include "gpxfileparser.h"
#include "cacheprovider.h"

#define PLUGIN_NAME "Gpx"

const QVariant Gpx::getInfo(const QString &name) const {
  if(name.compare("Name") == 0)
    return PLUGIN_NAME;

  if(name.compare("CanBeDefault") == 0)
    return true;

  if(name.compare("LogUrl") == 0)
    return QString("http://www.geocaching.com/seek/log.aspx?wp=%1");

  if(name.compare("License") == 0)
    return QObject::tr("GPL version 2<br>"
		       "Copyright 2011 by Till Harbaum<br>"
		       "Quazip parts Copyright 2005 by Sergey A. Tachenov");

  return QVariant();
}

Gpx::Gpx() : m_cacheList(PLUGIN_NAME), m_fileParser(NULL), m_thread(NULL) {
  qDebug() << PLUGIN_NAME << __FUNCTION__;

  // try to get credentials from qsettings
  QSettings settings;
  settings.beginGroup("Account");
  m_name = settings.value("Name", "").toString();
  settings.endGroup();
}

Gpx::~Gpx() {
  qDebug() << PLUGIN_NAME << __FUNCTION__;

  // clean up
  if(m_fileParser) delete m_fileParser;
  if(m_thread) delete m_thread;
}

QObject *Gpx::object() {
  return this;
}

void Gpx::fileParserSucceeded(const CacheList &cacheList) {
  qDebug() << PLUGIN_NAME << __FUNCTION__ << cacheList.size();
  m_cacheList = cacheList;

  emit notifyBusy(false);

  // request map to reload caches
  emit reload();

  // check if parsing actually got us some results and only save
  // the file location then
  if(m_cacheList.size() > 0) {
    // save filename in settings
    QSettings settings;
    settings.beginGroup(PLUGIN_NAME);
    settings.setValue("File", m_fileName);
    settings.endGroup();
  } else
    error(tr("File %1 does not contain any geocaches").arg(m_fileName));
}

void Gpx::fileParserFailed(const QString &reason) {
  qDebug() << PLUGIN_NAME << __FUNCTION__ << reason;  
  error(reason);

  emit notifyBusy(false);

  // this will sure cause no caches to be reloaded, but it also removes
  // all leftovers from a previous cache provider
  emit reload();
}

void Gpx::init(QWidget *parent, const QMap<QString, QVariant> &) {
  qDebug() << PLUGIN_NAME << __FUNCTION__;

  // if there's a valid parent, then the UI is up and we can actually
  // use the file selector, otherwise this is happening automatically
  // in the startup phase

  QSettings settings;
  settings.beginGroup(PLUGIN_NAME);
  m_fileName = settings.value("File").toString();
  settings.endGroup();
  
  // Only open file selector when being interactive
  if(parent) 
    m_fileName = QFileDialog::getOpenFileName(parent,
		  tr("Open Pocket Query (GPX, LOC or ZIP)"), m_fileName, 
		  tr("Pocket Queries (*.gpx *.loc *.zip)"));

  if(m_fileName.isNull() || m_fileName.isEmpty()) {
    emit reload();
    return;
  }
    
  QFile file(m_fileName);
  if(!file.exists()) {
    error(tr("File %1 not found").arg(m_fileName));
    emit reload();
    return;
  }

  if(!m_fileParser) {
    m_fileParser = new GpxFileParser;
    if(!m_name.isEmpty())
      m_fileParser->setName(m_name);
  }

  if(!m_thread) {
    m_thread = new QThread;
   
    m_fileParser->moveToThread(m_thread);
    qRegisterMetaType<CacheList>("CacheList");
    connect(this, SIGNAL(fileParse(const QString &)),
	    m_fileParser, SLOT(parse(const QString &)));
    connect(m_fileParser, SIGNAL(succeeded(const CacheList &)),
	    this, SLOT(fileParserSucceeded(const CacheList &)));
    connect(m_fileParser, SIGNAL(failed(const QString &)),
	    this, SLOT(fileParserFailed(const QString &)));
	  
    m_thread->start();
  }

  emit notifyBusy(true);
  emit fileParse(m_fileName);
}

bool Gpx::busy() {
  qDebug() << PLUGIN_NAME << __FUNCTION__;
  return(false);
}

void Gpx::requestOverview(const QGeoBoundingBox &area, const int flags) {
  Params params;
  params.insert("Type", "Overview");

  CacheList retval(m_cacheList.name());
  retval.setDate(m_cacheList.date());

  // create list of matching caches
  foreach(Cache cache, m_cacheList) {
    if(area.contains(cache.coordinate())) 
      // ignore owned/found caches if we are not supposed to return them
      if(((!(flags && CacheProviderPlugin::NoFound)) ||
	  !cache.found().isSetAndTrue()) &&
     ((!(flags && CacheProviderPlugin::NoOwned)) ||
	  !cache.owned().isSetAndTrue()))
	retval.append(cache);
  }
  
  emit done();
  emit replyCacheList(params, retval);
  emit next();
}

void Gpx::requestInfo(const QString &name) {
  emit done();

  // find this cache
  foreach(Cache cache, m_cacheList) {
    if(cache.name() == name) {
      Params params;
      params.insert("Type", "Info");
      emit replyCache(params, cache);
      emit next();
      return;
    }
  }
      
  error(tr("Unable to get cache info"));
  emit next();
}

void Gpx::requestDetail(const QString &name) {
  emit done();

  // find this cache
  foreach(Cache cache, m_cacheList) {
    if(cache.name() == name) {
      Params params;
      params.insert("Type", "Detail");
      emit replyCache(params, cache);
      emit next();
      return;
    }
  }
      
  error(tr("Unable to get cache detail"));
  emit next();
}

void Gpx::request(const Params &params) {
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
    requestInfo(name);
  }

  else if(type.compare("Detail") == 0) {
    QString name(params.value("Name").toString());
    requestDetail(name);
  }

  else {
    qDebug() << __FUNCTION__ << "unexpected request" << type;
    emit done();
    emit next();
  }
}

void Gpx::applyChanges() {
  qDebug() << PLUGIN_NAME << __FUNCTION__;

  // settings have been saved by settings dialog,
  // so we can already read them again

  QSettings settings;
  settings.beginGroup("Account");
  QString name = settings.value("Name", "").toString();
  settings.endGroup();

  if(name != m_name) {
    m_name = name;

    /* re-evaluate current file with respect to the name */
    GpxParser::verifyName(m_cacheList, m_name);
    emit reload();
  }
}

void Gpx::registerSettings(QDialog *parent) {
  connect(parent, SIGNAL(accepted()), this, SLOT(applyChanges()));
}

// send an error message to the cache provider
void Gpx::error(const QString &msg) {
  Params params;
  params.insert("Type", "Error");

  if(!msg.isEmpty())
    params.insert("Message", tr(PLUGIN_NAME) + ": " + msg);
  
  emit reply(params);
}

Q_EXPORT_PLUGIN2(gpx, Gpx);
