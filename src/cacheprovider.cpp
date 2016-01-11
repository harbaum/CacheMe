/*
  cacheprovider, class answering requests for caches

  The cacheprovider handles requests asynchronously and 
  internally uses a queue to store requests. It also drops
  unhandled pending requests if a newer request of the same
  time comes in
*/

#include <QDebug>

// to load plugins
#include <QApplication>
#include <QPluginLoader>
#include <QDir>
#include <QSettings>

#include "cacheproviderplugin.h"
#include "cacheprovider.h"
#include "filefinder.h"
#include "config.h"

CacheProvider::RequestEntry::RequestEntry(const RequestType &type, const QString &cache) {

  Q_ASSERT((type == Info) || (type == Detail));

  this->m_type = type;
  this->m_cache = cache;
}

CacheProvider::RequestEntry::RequestEntry(const RequestType &type, const QGeoBoundingBox &bbox, int zoom, int flags) {

  Q_ASSERT(type == Overview);

  this->m_type = type;
  this->m_bbox = bbox;
  this->m_zoom = zoom;
  this->m_flags = flags;
}

CacheProvider::RequestEntry::RequestEntry(const RequestType &type, const QMap<QString, QVariant> &params) {
  Q_ASSERT(type == Custom);

  this->m_type = type;
  this->m_params = params;
}

CacheProvider::RequestEntry::RequestType CacheProvider::RequestEntry::type() {
  return this->m_type;
}

QGeoBoundingBox CacheProvider::RequestEntry::bbox() {
  return this->m_bbox;
}

QMap<QString, QVariant> CacheProvider::RequestEntry::params() {
  return this->m_params;
}

int CacheProvider::RequestEntry::flags() {
  return this->m_flags;
}

int CacheProvider::RequestEntry::zoom() {
  return this->m_zoom;
}

QString CacheProvider::RequestEntry::cache() {
  return this->m_cache;
}

void CacheProvider::RequestEntry::set(const QString &cache) {
  this->m_cache = cache;
}

void CacheProvider::RequestEntry::set(const QGeoBoundingBox &bbox, const int zoom, const int flags) {
  this->m_bbox = bbox;  
  this->m_zoom = zoom;  
  this->m_flags = flags;  
}

CacheProvider::RequestQueue::RequestQueue(CacheProvider *cacheProvider) {
  this->m_cacheProvider = cacheProvider;
}

bool CacheProvider::RequestQueue::add(const RequestEntry::RequestType &type, const QString &cache) {

  // ignore detail requests if there's already one being processed. This
  // prevents multiple stacked windows from opening on maemo5
  if(size() && (type == RequestEntry::Detail) && 
     (head()->type() == RequestEntry::Detail)) {
    qDebug() << __FUNCTION__ << "supressing addional Detail request";
    return false;
  }

  // check if there's already the same type of request in
  // queue and overwrite that one
  if(size() > 1) {
    for(int i=1;i<size();i++) {
      if(at(i)->type() == type) {
	at(i)->set(cache);
	qDebug() << __FUNCTION__ << "updating request already present";
	return false;
      }      
    }
  }

  enqueue(new RequestEntry(type, cache));
  return true;
}

bool CacheProvider::RequestQueue::add(const RequestEntry::RequestType &type, const QGeoBoundingBox &bbox, const int zoom, const int flags) {

  // check if there's already the same type of request waiting in
  // the queue and overwrite that one
  if(size() > 1) {
    for(int i=1;i<size();i++) {
      if(at(i)->type() == type) {
	at(i)->set(bbox, zoom, flags);
	qDebug() << __FUNCTION__ << "updating request already present";
	return false;
      }      
    }
  }

  enqueue(new RequestEntry(type, bbox, zoom, flags));
  return true;
}

bool CacheProvider::RequestQueue::add(const RequestEntry::RequestType &type, const QMap<QString, QVariant> &params) {
  qDebug() << __FUNCTION__ << params;

  enqueue(new RequestEntry(type, params));
  return true;
}

CacheProvider::RequestEntry::RequestType CacheProvider::RequestQueue::type() {
  return head()->type();
}

void CacheProvider::RequestQueue::done() {
  qDebug() << __FUNCTION__;

  if(empty()) {
    qDebug() << __FUNCTION__ << "ARGHHH!! Queue empty!";
    return;
  }

  delete dequeue();
}

void CacheProvider::RequestQueue::next() {

  if(!empty()) {
    // process topmost entry
    switch(type()) {
    case RequestEntry::Overview:
      qDebug() << __FUNCTION__ << "Pending Overview request";
      m_cacheProvider->processRequestOverview(head()->bbox(), 
					      head()->zoom(), head()->flags()); 
      break;

    case RequestEntry::Info:
      qDebug() << __FUNCTION__ << "Pending Info request";
      m_cacheProvider->processRequestInfo(head()->cache()); 
      break; 

    case RequestEntry::Detail:
      qDebug() << __FUNCTION__ << "Pending Detail request";
      m_cacheProvider->processRequestDetail(head()->cache()); 
      break; 

    case RequestEntry::Custom:
      qDebug() << __FUNCTION__ << "Pending Custom request";
      m_cacheProvider->processRequestCustom(head()->params()); 
      break; 

    default:
      Q_ASSERT(0);
      break;
    }    
  } else
    qDebug() << __FUNCTION__ << "No pending request";
}

void CacheProvider::RequestQueue::restart() {
  // restart if queue holds one entry and if the cache provider
  // is not busy doing its own stuff (e.g. logging in)
  if((size() == 1) && !m_cacheProvider->busy()) 
    next();
}

void CacheProvider::loadPluginsInDir(QDir &pluginsDir) {

  if(!pluginsDir.cd("plugins")) return;
  if(!pluginsDir.cd("cacheprovider")) return;
  
  qDebug() << __FUNCTION__ << "Loading plugins from:" << pluginsDir;

  // load all available plugins
  foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
    if(QLibrary::isLibrary(fileName)) {
      qDebug() << __FUNCTION__ << fileName;
      QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
      QObject *plugin = loader.instance();
      if(!loader.isLoaded()) 
	qDebug() << __FUNCTION__ << loader.errorString();
      else if (plugin) {
	CacheProviderPlugin *cacheProviderPlugin = 
	  qobject_cast<CacheProviderPlugin *>(plugin);
	
	m_cacheProviderPluginList.append(cacheProviderPlugin);
	
	qDebug() << __FUNCTION__ << "Plugin name:" << 
	  getInfoString(cacheProviderPlugin, "Name");
      }
    }
  }
}
  
CacheProvider::CacheProvider(QWidget *parent) : 
  m_currentPlugin(NULL), m_parent(parent) {
  QDir pluginsDir = QDir(QApplication::applicationDirPath());

  // try to load plugins from local path first
#if defined(Q_OS_WIN)
  if (pluginsDir.dirName().toLower() == "debug" || 
      pluginsDir.dirName().toLower() == "release")
    pluginsDir.cdUp();
#endif

  loadPluginsInDir(pluginsDir);

#ifdef LIBDIR
  if(m_cacheProviderPluginList.isEmpty()) {
    // if no local plugins were found -> try the intallation path
    pluginsDir = QDir(LIBDIR);  
    loadPluginsInDir(pluginsDir);
  }
#endif

  this->m_pending = new RequestQueue(this);

  QSettings settings;
  settings.beginGroup("CacheProvider");
  QString name = settings.value("name").toString();
  settings.endGroup();

  // try to load preset plugin 
  foreach(CacheProviderPlugin *plugin, m_cacheProviderPluginList) {
    if(getInfoString(plugin, "Name") == name) {
      this->m_currentPlugin = plugin;
      return;
    }
  }
  
  // activate first default capable plugin if available
  foreach(CacheProviderPlugin *plugin, m_cacheProviderPluginList) {
    if(getInfoBool(plugin, "CanBeDefault")) {
      this->m_currentPlugin = plugin;
      return;
    }
  }
}

CacheProvider::~CacheProvider() {
  qDebug() << __FUNCTION__;

  delete this->m_pending;
}

bool CacheProvider::getInfoBool(const CacheProviderPlugin *plugin, const QString &name) {
  QVariant variant = plugin->getInfo(name);

  if(variant.isNull())
    return false;

  if(variant.type() != QVariant::Bool) {
    qDebug() << __FUNCTION__ << "expected type bool for" << name;
    return false;
  }

  return variant.toBool();
}

QString CacheProvider::getInfoString(const CacheProviderPlugin *plugin, const QString &name) {
  QVariant variant = plugin->getInfo(name);

  if(variant.type() != QVariant::String) {
    qDebug() << __FUNCTION__ << "expected type string for" << name;
    return QString();
  }

  return variant.toString();
}

void CacheProvider::requestOverview(const QGeoBoundingBox &area, 
				    int zoom, int flags) {
  qDebug() << __PRETTY_FUNCTION__ << "from" << area.topLeft() << 
    " to " << area.bottomRight() << "zoom" << zoom;

  if(m_pending->add(RequestEntry::Overview, area, zoom, flags))
    m_pending->restart();
}

void CacheProvider::requestInfo(const QString &cache) {
  //  qDebug() << __PRETTY_FUNCTION__ << cache;

  if(m_pending->add(RequestEntry::Info, cache))
    m_pending->restart();
}

void CacheProvider::requestDetail(const QString &cache) {
  //  qDebug() << __PRETTY_FUNCTION__ << cache;

  if(m_pending->add(RequestEntry::Detail, cache))
    m_pending->restart();
}

void CacheProvider::requestCustom(const QMap<QString, QVariant> &parms) {
  //  qDebug() << __PRETTY_FUNCTION__ << parms;  

  if(m_pending->add(RequestEntry::Custom, parms))
    m_pending->restart();
}

void CacheProvider::next() {
  this->m_pending->next();
}

void CacheProvider::done() {
  this->m_pending->done();
}

CacheProvider::RequestEntry::RequestType CacheProvider::type() {
  return this->m_pending->type();
}

// the following functions are called by the plugin in order to 
// let the CacheProvider emit a signal
void CacheProvider::emitReply(const Params &params) {
  emit reply(params);
}

void CacheProvider::emitReplyCache(const Params &params, const Cache &cache) {
  emit replyCache(params, cache);
}

void CacheProvider::emitReplyCacheList(const Params &params, 
				       const CacheList &cacheList) {
  emit replyCacheList(params, cacheList);
}

void CacheProvider::emitReload() {
  emit reload();
}

void CacheProvider::emitNotifyBusy(bool on) {
  emit notifyBusy(on);
}

QString CacheProvider::name() {
  if(!m_currentPlugin) return("<none>");
  return getInfoString(m_currentPlugin, "Name");
}

bool CacheProvider::busy() {
  if(!m_currentPlugin) return(true);
  return m_currentPlugin->busy();
}
 
void CacheProvider::processRequestOverview(const QGeoBoundingBox &bbox, const int zoom, const int flags) {
  qDebug() << __FUNCTION__;

  if(m_currentPlugin) {
    Params params;

    // assemble overview request
    params.insert("Type", "Overview");
    params.insert("Zoom", zoom);
    params.insert("Flags", flags);
    
    params.insert("TLLat", bbox.topLeft().latitude());
    params.insert("TLLon", bbox.topLeft().longitude());
    params.insert("BRLat", bbox.bottomRight().latitude());
    params.insert("BRLon", bbox.bottomRight().longitude());
    
    m_currentPlugin->request(params);
  }
}

void CacheProvider::processRequestInfo(const QString &id) {
  if(m_currentPlugin) {
    Params params;

    // assemble info request
    params.insert("Type", "Info");
    params.insert("Name", id);
    m_currentPlugin->request(params);
  }
}

void CacheProvider::processRequestDetail(const QString &id) {
  if(m_currentPlugin) {
    Params params;

    // assemble detail request
    params.insert("Type", "Detail");
    params.insert("Name", id);
    m_currentPlugin->request(params);
  }
}

void CacheProvider::processRequestCustom(const QMap<QString, QVariant>  &params) {
  if(m_currentPlugin)
    m_currentPlugin->request(params);
}

void CacheProvider::start(QWidget *parent) {
  foreach(CacheProviderPlugin *cplugin, m_cacheProviderPluginList) {
    QObject *plugin = cplugin->object();

    // connect to plugin
    connect(plugin, SIGNAL(reply(const Params &)), 
	    this, SLOT(emitReply(const Params &)));

    connect(plugin, SIGNAL(replyCache(const Params &, const Cache &)), 
	    this, SLOT(emitReplyCache(const Params &, const Cache &)));

    connect(plugin, SIGNAL(replyCacheList(const Params &, const CacheList &)), 
	    this, SLOT(emitReplyCacheList(const Params &, const CacheList &)));

    connect(plugin, SIGNAL(reload()), 
	    this, SLOT(emitReload()));
    
    connect(plugin, SIGNAL(notifyBusy(bool)), 
	    this, SLOT(emitNotifyBusy(bool)));
    
    connect(plugin, SIGNAL(done()), this, SLOT(done()));
    
    connect(plugin, SIGNAL(next()), this, SLOT(next()));
  }

  // call active plugins init function
  if(this->m_currentPlugin) {
    this->m_currentPlugin->init(parent, getParams());

    // en/disable update button
    emit manualUpdateRequired(getInfoBool(m_currentPlugin, "ManualUpdate"));
    emit searchSupported(getInfoBool(m_currentPlugin, "SupportsSearch"));
    emit authorizationSupported(getInfoBool(m_currentPlugin, "SupportsAuthorization"));
  }
}

QString CacheProvider::getInfoString(const QString &name) {
  if(!this->m_currentPlugin) 
    return NULL;

  return getInfoString(this->m_currentPlugin, name);
}

bool CacheProvider::getInfoBool(const QString &name) {
  if(!this->m_currentPlugin) 
    return false;

  return getInfoBool(this->m_currentPlugin, name);
}

bool CacheProvider::loggingIsSupported() {
  if(!this->m_currentPlugin) 
    return false;

  return getInfoBool(m_currentPlugin, "SupportsLogging");
}

const QMap<QString, QVariant> CacheProvider::getParams() {
  QString name = getInfoString(m_currentPlugin, "Name");

  qDebug() << __FUNCTION__ << name;

  // forward secret keys from main app since the plugin is open source
  QMap<QString, QVariant> params;
  if(name.compare("GcLive", Qt::CaseInsensitive) == 0) {
#warning "You need to obtain a ConsumerKey/ConsumerSecretKey pair from geocaching.com for this to work"

#ifdef STAGING
    params.insert("ConsumerKey", "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx");
    params.insert("ConsumerSecretKey", "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx");
#else
    params.insert("ConsumerKey", "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx");
    params.insert("ConsumerSecretKey", "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx");
#endif
  }

  // all plugins get the location of the datadir where they may create a cache file
  params.insert("DataDir", FileFinder::dataPath());

  return params;
}

void CacheProvider::providerSelected(QAction *action) {
  CacheProviderPlugin *plugin = NULL;

  // figure out which entry was selected
  foreach(CacheProviderPlugin *cp, m_cacheProviderPluginList) 
    if(getInfoString(cp, "Name") == action->text())
      plugin = cp;

  Q_ASSERT(plugin);

  QString name = getInfoString(plugin, "Name");
  qDebug() << __FUNCTION__ << name;  

  this->m_currentPlugin = plugin;
  this->m_currentPlugin->init(this->m_parent, getParams());
  emit manualUpdateRequired(getInfoBool(m_currentPlugin, "ManualUpdate"));
  emit searchSupported(getInfoBool(m_currentPlugin, "SupportsSearch"));
  emit authorizationSupported(getInfoBool(m_currentPlugin, "SupportsAuthorization"));

  QSettings settings;
  settings.beginGroup("CacheProvider");
  settings.setValue("name", name);
  settings.endGroup();
}

void CacheProvider::createMenu(QMenuBar *menuBar) {

  QMenu *cacheProvider = menuBar->addMenu(tr("&Cache Provider"));
  QActionGroup *filterGroup = new QActionGroup(this);
  filterGroup->setExclusive(true);

  foreach(CacheProviderPlugin *plugin, m_cacheProviderPluginList) {
    QAction *action = new QAction(getInfoString(plugin, "Name"), filterGroup);
    action->setCheckable(true);

    if(m_currentPlugin == plugin)
      action->setChecked(true);
  }

  connect(filterGroup, SIGNAL(triggered(QAction *)), 
	  this, SLOT(providerSelected(QAction *)));
  
  cacheProvider->addActions(filterGroup->actions());        
}

QStringList CacheProvider::names() {
  QStringList retval;

  foreach(CacheProviderPlugin *plugin, m_cacheProviderPluginList) 
    retval << getInfoString(plugin, "Name");

  return retval;
}

QStringList CacheProvider::licenses() {
  QStringList retval;

  foreach(CacheProviderPlugin *plugin, m_cacheProviderPluginList) 
    retval << getInfoString(plugin, "License");

  return retval;
}

void CacheProvider::registerSettings(QDialog *parent) {
  foreach(CacheProviderPlugin *plugin, m_cacheProviderPluginList)
    plugin->registerSettings(parent);
}

