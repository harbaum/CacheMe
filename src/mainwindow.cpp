// Filename: mainwindow.cpp

#include <QDebug>
#include <QMessageBox>
#include <QPushButton>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QUrl>
#include <QXmlStreamReader>
#include <QDialogButtonBox>

#include "mainwindow.h"
#include "mapwidget.h"
#include "settingsdialog.h"
#include "iconloader.h"  
#include "cachewindow.h"
#include "cachelistwindow.h"
#include "navigationwindow.h"
#include "browserwindow.h"
#include "config.h"
#include "tool.h"
#include "htmlview.h"
#include "searchdialog.h"

#ifdef FLICKCHARM
#include "flickcharm.h"
#endif

#include <QIcon>
#include <QSettings>

#ifndef Q_WS_MAEMO_5
#include <QVBoxLayout>
#else
#include <QMaemo5InformationBox>
#endif

#include "tool_rot13.h"
#include "tool_projection.h"
#include "tool_formula.h"
#include "tool_roman.h"

void MainWindow::launchToolRot13() {
  ToolRot13 *tool = new ToolRot13(this);
  tool->show();
}

void MainWindow::launchToolRoman() {
  ToolRoman *tool = new ToolRoman(this);
  tool->show();
}

void MainWindow::launchToolFormula() {
  ToolFormula *tool = new ToolFormula(this);
  tool->show();
}

void MainWindow::launchToolProjection() {
  ToolProjection *tool = new ToolProjection(m_mapWidget, m_wptDb, this);
  tool->show();
}

void MainWindow::enableToolProjection(bool on) {
  qDebug() << __FUNCTION__ << on;

  if(m_ActionToolProjection)
    m_ActionToolProjection->setEnabled(on);
}

void MainWindow::enableToolSearch(bool on) {
  qDebug() << __FUNCTION__ << on;

  if(m_ActionToolSearch)
    m_ActionToolSearch->setEnabled(on);
}

void MainWindow::enableToolAuthorize(bool on) {
  qDebug() << __FUNCTION__ << on;

  if(m_ActionToolAuthorize)
    m_ActionToolAuthorize->setEnabled(on);
}

void MainWindow::createMenu() {

  addMenuEntry(tr("&About"), this, SLOT(about()));

  addMenuEntry(tr("&Help"), this, SLOT(help()));

  addMenuEntry(tr("&Settings"), this, SLOT(settingsdialog()));

  /* create a submenu for the plugin selection */  
  m_cacheProvider->createMenu(menuBar());

  /* add tools to the tools menu */
  QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
  ToolRot13::registerMenu(toolsMenu, this, SLOT(launchToolRot13()));
  m_ActionToolProjection = 
    ToolProjection::registerMenu(toolsMenu, this, SLOT(launchToolProjection()));
  m_ActionToolProjection->setEnabled(false);
  ToolFormula::registerMenu(toolsMenu, this, SLOT(launchToolFormula()));
  ToolRoman::registerMenu(toolsMenu, this, SLOT(launchToolRoman()));

  m_motdAction = addMenuEntry(tr("&MotD"), this, SLOT(motd()), toolsMenu);
  m_motdAction->setEnabled(false);

  m_ActionToolSearch = addMenuEntry(tr("&Search"), this, SLOT(search()), toolsMenu);
  m_ActionToolSearch->setEnabled(false);

  m_ActionToolAuthorize = addMenuEntry(tr("&Authorize"), this, SLOT(authorize()), toolsMenu);
  m_ActionToolAuthorize->setEnabled(false);

#if (defined(Q_OS_SYMBIAN) && (QT_VERSION >= 0x040704)) && !defined(Q_WS_SIMULATOR)
  // symbian anna uses 4.7.4, symbian 1 uses 4.7.3
  // on anna/belle add an exit menu entry
  addMenuEntry(tr("E&xit"), this, SLOT(close()));
#endif
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

void MainWindow::replyFinished(QNetworkReply *reply) {
  // invoke appropriate decoder
  if(reply->isFinished()) {
    QString allData = QString::fromUtf8(reply->readAll());

    // send reply through xml parser to extract message and version
    QXmlStreamReader xml(allData);
    while(!xml.atEnd() && !xml.hasError()) {
      xml.readNext();
      
      if(xml.tokenType() == QXmlStreamReader::StartDocument) 
	continue;
      
      if(xml.tokenType() == QXmlStreamReader::StartElement) {
	if(xml.name() == "motd") {

	  int version = xml.attributes().value("version").toString().toInt();
	  m_motd = xml.readElementText().simplified();	  

	  /* check if this is a new message */
	  if(version > m_motdVersion) {
	    motd();
	    m_motdVersion = version;
	  }

	  if(!m_motd.isEmpty()) 
	    m_motdAction->setEnabled(true);
	} else 
	  drop(xml);
      }
    }
      
      /* Error handling. */
    if(xml.hasError()) 
      qDebug() << __FUNCTION__ << QString("Parse error at line %1, column %2:\n%3")
	.arg(xml.lineNumber()).arg(xml.columnNumber()).arg(xml.errorString());
    
    xml.clear();
  }

  reply->deleteLater();
}

void MainWindow::settingsdialog() {
  qDebug() << __FUNCTION__;

  SettingsDialog dialog(m_mapWidget, m_cacheProvider, this);
  dialog.exec(); 
}

void MainWindow::help() {
  openUrl(QUrl("http://www.harbaum.org/till/cacheme/help.html"));
}

void MainWindow::aboutBox(QWidget *parent, const QString &title, const QString &text) {
  QMessageBox *msgBox = 
    new QMessageBox(title, text, QMessageBox::Information, 0, 0, 0, parent);
  msgBox->setAttribute(Qt::WA_DeleteOnClose);

  msgBox->addButton(QMessageBox::Ok);

#if defined(Q_OS_SYMBIAN) || defined(Q_WS_SIMULATOR)
  QAction *aboutPluginsAction = new QAction( tr("Plugins"), this );
  aboutPluginsAction->setSoftKeyRole( QAction::NegativeSoftKey );
  connect(aboutPluginsAction, SIGNAL(triggered()), this, SLOT(aboutPlugins()));
  msgBox->addAction(aboutPluginsAction );
#else
  QPushButton *aboutPluginsButton = msgBox->addButton(tr("Plugins"), QMessageBox::HelpRole);
  connect(aboutPluginsButton, SIGNAL(clicked()), this, SLOT(aboutPlugins()));
#endif

  QIcon icon = msgBox->windowIcon();
  QSize size = icon.actualSize(QSize(64, 64));
  msgBox->setIconPixmap(icon.pixmap(size));

  msgBox->exec();
}

void MainWindow::about() {
  aboutBox(this, tr("About CacheMe"),
	   tr("CacheMe") + " " + tr("Version") + " " + VERSION + "\n" + 
	   tr("Build date:") + " " + __DATE__ + "\n" +
           tr("(c) 2011/2012 by Till Harbaum <till@harbaum.org>") + "\n" +
           tr("Some tools (c) 2012 by George Ruinelli") + "\n" +
           tr("Exparser (c) 2011 by Justin L.") + "\n\n" +
           tr("CacheMe is an online geocaching application.")
	   );
}

void MainWindow::aboutPlugins() {
  Description text;
  QString message;

  QStringList names = m_cacheProvider->names();
  QStringList licenses = m_cacheProvider->licenses();

  message = "<html><body>";
  
  message += "<hr><h3>" + tr("Translations") + "</h3>";
  message += tr("German by Till Harbaum") + "<br>";
  message += tr("Slowak by Edward") + "<br>";
  message += tr("Finnish by Henri Manninen") + "<br>";
  message += tr("Czech by Karel Farsk&yacute;") + "<br>";
  message += tr("Swedish by Joakim Carlsson") + "<br>";

  int i;
  for(i=0;i<names.size();i++) {
    message += "<hr><h3>" + tr("Plugin") + ": " + names[i] +"</h3>";
    message += "<p><b>" + tr("License") + ":</b><br>" + licenses[i] +"</p>";
  }
  message += "</body></html>";

  text.set(true, message);

  QDialog dialog(this);
  dialog.setWindowTitle(tr("Plugins"));

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(0,0,0,0);
  mainLayout->setSpacing(0);

  HtmlView *htmlView = new HtmlView(text, &dialog);
#ifdef FLICKCHARM
  new FlickCharm(htmlView, this);
#endif
  
  mainLayout->addWidget(htmlView);
  QDialogButtonBox *buttonBox = 
    new QDialogButtonBox(QDialogButtonBox::Ok);

  connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
  mainLayout->addWidget(buttonBox);

  dialog.setLayout(mainLayout);

  dialog.exec(); 
}

void MainWindow::search() {
  SearchDialog dialog(m_cacheProvider->getInfoBool("SearchSupportsContainerSelection"), this);
  if(dialog.exec() == QDialog::Accepted) {
    QMap<QString, QVariant> params;
    params.insert("Type", "Search"); 
    params.insert("Latitude", m_mapWidget->center().latitude());
    params.insert("Longitude", m_mapWidget->center().longitude());
    dialog.params(params);
    m_cacheProvider->requestCustom(params);
  }
}

void MainWindow::authorize() {
  QMap<QString, QVariant> params;
  params.insert("Type", "Authorize"); 
  m_cacheProvider->requestCustom(params);
}

void MainWindow::motd() {
  if(!m_motd.isEmpty()) {
    Description motd;
    motd.set(true, m_motd);
    
    CustomWindow *motdWin = new CustomWindow("MotdWindow", this);
    motdWin->setWindowTitle(tr("Message of the day"));
    HtmlView *htmlView = new HtmlView(motd, motdWin);
#ifdef FLICKCHARM
    new FlickCharm(htmlView, this);
#endif
    
    motdWin->setCentralWidget(htmlView);
    motdWin->show();
  }
}

void MainWindow::handleReply(const Params &params) {
  QString type = params.value("Type").toString();

  if(type.compare("OpenUrl") == 0)
    openUrl(QUrl(params.value("Url").toString()));
  if(type.compare("Error") == 0)
    displayError(params.value("Message").toString());
  else
    qDebug() << __FUNCTION__ << "Unknown type" << type;
}

void MainWindow::handleReplyCache(const Params &params, const Cache &cache) {

  // only handle replys if the window is actually the focussed one
  if(isActiveWindow()) {
    QString type = params.value("Type").toString();

    if(type.compare("Info") == 0)
      m_mapWidget->showBubble(cache);
    else if(type.compare("Detail") == 0)
      showDetail(cache);
    else
      qDebug() << __FUNCTION__ << "Unknown type" << type;
  }
}

void MainWindow::handleReplyCacheList(const Params &params, const CacheList &cacheList) {
  // only handle replys if the window is actually the focussed one
  QString type = params.value("Type").toString();
    
  if(type.compare("Overview") == 0)
    updateCaches(cacheList);
  else if(type.compare("Search") == 0)
    showCacheList(cacheList);
  else
    qDebug() << __FUNCTION__ << "Unknown type" << type;
}

MainWindow::MainWindow(LocationProvider *locationProvider, QWidget *parent) : 
  CustomWindow("MainWindow", parent), m_locationProvider(locationProvider),
  m_lastFlags(0), m_ActionToolProjection(NULL), m_motdVersion(0) {

  qDebug() << __FUNCTION__;
  m_cacheProvider = new CacheProvider(this);

  setWindowTitle(tr("CacheMe"));

  IconLoader iconLoader(48);
  QPixmap *icon = iconLoader.load(APPNAME);
  if(icon) setWindowIcon(QIcon(*icon));

  createMenu();

  // search for service providers
  QStringList list = QGeoServiceProvider::availableServiceProviders();
  for(int i=0;i<list.size();i++)
    qDebug() << __FUNCTION__ << "Service provider: " << list[i];

  // there needs to be at least one geo service provider
  Q_ASSERT(list.size() > 0);

  QSettings settings;
  settings.beginGroup("Map");
  QString serviceProviderStr = settings.value("Service", "nokia").toString();
  settings.endGroup();
  m_motdVersion = settings.value("motdVersion", 0).toInt();

  // give app id key to service provider
  QMap<QString, QVariant> params;
  params.insert("mapping.app_id", "athGTWaNTNPAgYYwbBLh");
  
  this->m_serviceProvider = new QGeoServiceProvider(serviceProviderStr, params);
  if(!this->m_serviceProvider->mappingManager())
    this->m_serviceProvider = new QGeoServiceProvider("nokia", params);

  this->m_mapWidget = new MapWidget(this->m_serviceProvider->mappingManager());

  m_wptDb = new WaypointDb();
  this->m_mapWidget->setWaypointDb(m_wptDb);

  // make sure cache updates are processed
  QObject::connect(this->m_locationProvider, SIGNAL(positionUpdated(const QGeoPositionInfo &)), 
		   this->m_mapWidget, SLOT(positionUpdated(const QGeoPositionInfo &)));
  QObject::connect(this->m_locationProvider, SIGNAL(positionUpdated(const QCompassReading *)), 
		   this->m_mapWidget, SLOT(positionUpdated(const QCompassReading *)));

  QObject::connect(this->m_cacheProvider, SIGNAL(reply(const Params &)), 
  		   this, SLOT(handleReply(const Params &)));

  QObject::connect(this->m_cacheProvider, SIGNAL(requestOpenUrl(const QUrl &)), 
  		   this, SLOT(openUrl(const QUrl &)));

  QObject::connect(this->m_cacheProvider, SIGNAL(replyCache(const Params &, const Cache &)), 
  		   this, SLOT(handleReplyCache(const Params &, const Cache &)));

  QObject::connect(this->m_cacheProvider, SIGNAL(replyCacheList(const Params &, const CacheList &)), 
  		   this, SLOT(handleReplyCacheList(const Params &, const CacheList &)));

  QObject::connect(this->m_cacheProvider, SIGNAL(reload()),
		   this, SLOT(reloadCaches()));

  QObject::connect(this->m_cacheProvider, SIGNAL(notifyBusy(bool)),
		   this, SLOT(setBusy(bool)));

  QObject::connect(this->m_cacheProvider, SIGNAL(manualUpdateRequired(bool)),
		   this->m_mapWidget, SLOT(enableUpdateButton(bool)));

  QObject::connect(this->m_mapWidget, SIGNAL(reload(const int)),
		   this, SLOT(reloadCaches(const int)));

  QObject::connect(this->m_mapWidget, SIGNAL(manualUpdateClicked(const int)),
		   this, SLOT(reloadCaches(const int)));

  QObject::connect(this->m_mapWidget, SIGNAL(showMessage(const QString &)),
 		   this, SLOT(displayError(const QString &)));

  QObject::connect(this->m_mapWidget, SIGNAL(cacheIsSelected(bool)),
		   this, SLOT(enableToolProjection(bool)));

  QObject::connect(this->m_cacheProvider, SIGNAL(searchSupported(bool)),
		   this, SLOT(enableToolSearch(bool)));

  QObject::connect(this->m_cacheProvider, SIGNAL(authorizationSupported(bool)),
		   this, SLOT(enableToolAuthorize(bool)));

  QObject::connect(this, SIGNAL(zoomInPressed()),
		   this->m_mapWidget, SLOT(zoomIn()));

  QObject::connect(this, SIGNAL(zoomOutPressed()),
		   this->m_mapWidget, SLOT(zoomOut()));

  QObject::connect(this->m_mapWidget, SIGNAL(fullscreen()),
		   this, SLOT(toggleFullscreen()));

  // connect to map to handle clicks on cache icons ...
  QObject::connect(this->m_mapWidget, SIGNAL(cacheClicked(const QString &)),
		   this, SLOT(selectCache(const QString &)));

  // ... and the detail button
  QObject::connect(this->m_mapWidget, SIGNAL(detailClicked(const QString &)),
		   this, SLOT(detailCache(const QString &)));
  QObject::connect(this->m_mapWidget, SIGNAL(wptDetailClicked(const Waypoint &)),
		   this, SLOT(detailWaypoint(const Waypoint &)));

  // also tell map about window activation and deactivation
  QObject::connect(this, SIGNAL(activated(bool)),
		   this->m_mapWidget, SLOT(windowActivated(bool)));

  // build a graphics scene based on the mapwidget
  QGraphicsScene *scene = new QGraphicsScene(this);
  m_view = new QGraphicsView( scene );
  m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_view->setVisible(true);
  m_view->setInteractive(true);
  scene->addItem(m_mapWidget);
  setCentralWidget(m_view);

  m_cacheProvider->start();

#ifdef EXPIRATION_DATE
  // create expiration timer if required
  if(QDate::fromString(EXPIRATION_DATE, "ddMMyyyy") <=
     QDate::currentDate()) {
    statusMessage(tr("This copy of CacheMe has expired and will close in one minute!"));
    QTimer::singleShot(60000, this, SLOT(close()));
  } else {
    statusMessage(tr("This copy of CacheMe will expire in %1 days!").
		  arg(QDate::currentDate().daysTo(
		  QDate::fromString(EXPIRATION_DATE, "ddMMyyyy"))));
  }
#endif  

  // fetch "message of the day" from harbaum.org

  // setup network manager and listen for its replies
  this->m_manager = new QNetworkAccessManager(this);
  connect(this->m_manager, SIGNAL(finished(QNetworkReply*)),
	  this, SLOT(replyFinished(QNetworkReply*)));

  QNetworkRequest request;
  request.setRawHeader("User-Agent", "CacheMe " VERSION);
  request.setUrl(QUrl("http://www.harbaum.org/till/cacheme/motd.xml"));
  this->m_manager->get(request);
}

MainWindow::~MainWindow() {
  qDebug() << __FUNCTION__;

  // save motd version
  QSettings settings;
  settings.setValue("motdVersion", m_motdVersion);

  delete m_manager;
  delete m_wptDb;
  delete m_cacheProvider;
  delete m_serviceProvider;
}

void MainWindow::resizeEvent(QResizeEvent*) {
  m_view->setSceneRect(QRect(QPoint(0,0), m_view->contentsRect().size()));
  m_mapWidget->resize(m_view->contentsRect().size());
}

void MainWindow::selectCache(const QString &name) {
  // ask CacheProvider for info about this cache
  this->m_cacheProvider->requestInfo(name);
}

void MainWindow::detailWaypoint(const Waypoint &wpt) {
  // if we can click in waypoints we sure have the details already
  // as the waypoints are part of the details
  qDebug() << __FUNCTION__ << "waypoint details for" << wpt.name();
  
  NavigationWindow *navigationWindow = 
    new NavigationWindow(wpt, m_locationProvider, m_mapWidget, this);
  
  navigationWindow->show();  
}

void MainWindow::detailCache(const QString &name) {
  // ask CacheProvider for details about this cache
  this->m_cacheProvider->requestDetail(name);
}

void MainWindow::reloadCaches() {
  qDebug() << __FUNCTION__;

  // don't send a request if the map is already preparing a reload
  if(m_mapWidget->preparingReload())
    return;

  // request new caches for the maps new viewport
  m_cacheProvider->requestOverview(this->m_mapWidget->viewport(), 
				   this->m_mapWidget->zoomLevel(), m_lastFlags);
}

void MainWindow::reloadCaches(const int flags) {
  qDebug() << __FUNCTION__ << flags;

  m_lastFlags = flags;

  // don't send a request if the map is already preparing a reload
  if(m_mapWidget->preparingReload())
    return;

  // request new caches for the maps new viewport
  m_cacheProvider->requestOverview(this->m_mapWidget->viewport(), 
				   this->m_mapWidget->zoomLevel(), flags);
}

void MainWindow::updateCaches(const CacheList &cacheList) {
  //  m_message = "";
  this->m_mapWidget->updateCaches(cacheList);
}

void MainWindow::statusMessage(const QString &message) {
#ifdef Q_WS_MAEMO_5
  QMaemo5InformationBox::information(this, message);
#else
  this->m_mapWidget->addBanner(message);
#endif
}

void MainWindow::displayError(const QString &message) {
  qDebug() << __FUNCTION__ << message;

  if(m_message != message) {
    statusMessage(message);
    m_message = message;
  }
}

void MainWindow::showDetail(const Cache &cache) {
  qDebug() << __FUNCTION__;

  CacheWindow *cacheWindow = 
    new CacheWindow(cache, m_locationProvider, m_wptDb, m_mapWidget, m_cacheProvider, this);
  cacheWindow->show();
}

void MainWindow::showSearchResultsOnMap() {
  QMap<QString, QVariant> params;
  params.insert("Type", "MapSearchResults"); 
  m_cacheProvider->requestCustom(params);

  reloadCaches();
}

void MainWindow::showCacheList(const CacheList &cacheList) {
  qDebug() << __FUNCTION__;

  CacheListWindow *cacheListWindow = 
    new CacheListWindow(m_mapWidget->center(), tr("Search Results"), cacheList, 
		m_locationProvider, m_wptDb, m_mapWidget, m_cacheProvider, this);

  connect(this->m_cacheProvider, SIGNAL(replyCache(const Params &, const Cache &)), 
	  cacheListWindow, SLOT(handleReplyCache(const Params &, const Cache &)));

  connect(cacheListWindow, SIGNAL(entrySelected(const QString &)),
	  this, SLOT(detailCache(const QString &)));

  connect(cacheListWindow, SIGNAL(requestShowOnMap()),
	  this, SLOT(showSearchResultsOnMap()));

  cacheListWindow->show();
}

void MainWindow::setBusy(bool on) {
  static int busyCount = 0;

  if((!on && busyCount == 1) || ( on && busyCount <= 0)) {
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, on);
#else
    m_mapWidget->setBusy(on);
#endif
  }

  if(on)                        busyCount++;
  else if(!on && busyCount > 0) busyCount--;
}

bool MainWindow::event(QEvent *event) {
  if(event->type() == QEvent::WindowActivate)
    emit activated(true);

  if(event->type() == QEvent::WindowDeactivate)
    emit activated(false);

  return CustomWindow::event(event);
}  

void MainWindow::openUrl(const QUrl &url) {
#ifdef BUILTINBROWSER
  if(!SettingsDialog::useBuiltInBrowser()) 
#endif
    QDesktopServices::openUrl(url);
#ifdef BUILTINBROWSER
  else {
    BrowserWindow *browserWindow = new BrowserWindow(url, this);
    browserWindow->show();  
  }
#endif
}
