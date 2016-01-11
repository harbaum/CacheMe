// Filename: cachelistwindow.cpp
// #include <QAbstractKineticScroller>

#include <QDebug>

#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QListWidget>
#include <QScrollBar>
#include <QPushButton>

#include "cachelistwindow.h"
#include "cachewidget.h"
#include "cachewindow.h"

#ifdef FLICKCHARM
#include "flickcharm.h"
#endif

void CacheListWindow::handleReplyCache(const Params &params, const Cache &cache) {
  qDebug() << __FUNCTION__;

  if(isActiveWindow()) {
    QString type = params.value("Type").toString();
    
    if(type.compare("Detail") == 0) {
      CacheWindow *cacheWindow = 
	new CacheWindow(cache, m_locationProvider, m_wptDb, m_mapWidget, m_cacheProvider, this);
      cacheWindow->show();
    }
  }
}

void CacheListWindow::selected(QListWidgetItem *item) {
  CacheWidget *cw = static_cast<CacheWidget*>(m_listWidget->itemWidget(item));
  if(!cw) {
    qDebug() << __FUNCTION__ << "unable to find cache";
    return;
  }

  qDebug() << __FUNCTION__ << cw->name();

  // the result of this signal will cause the main window
  // to open a cache window
  emit entrySelected(cw->name());
}

void CacheListWindow::sort(SortOrder order) {
  if(order == SortByName)
    m_cacheList.sortByDescription();
  else // if(m_sortOrder == SortByMapDistance)
    m_cacheList.sortByDistance(m_mapCenter);
}

void CacheListWindow::showOnMap() {
  qDebug() << __FUNCTION__;

  close();

  emit requestShowOnMap();

  //  params.insert("Type", "MapSearch"); 
  //  m_cacheProvider->requestCustom(params);
}

CacheListWindow::CacheListWindow(const QGeoCoordinate &mapCenter,
		 const QString &name, const CacheList &cacheList, 
		 LocationProvider *locationProvider, WaypointDb *wptDb, 
                 MapWidget *mapWidget, CacheProvider *cacheProvider,
		 QWidget *parent) : CustomWindow(name + "Window", parent),
    	         m_cacheList(cacheList), m_iconLoader(24) {
  
  setWindowTitle(name);

  QWidget *mainWidget = new QWidget;
  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->setContentsMargins(0,0,0,0);
  

  m_mapCenter = mapCenter;

  // store all the pointers required to call the detail view
  m_locationProvider = locationProvider;
  m_wptDb = wptDb;
  m_mapWidget = mapWidget;
  m_cacheProvider = cacheProvider;

  // the cache list
  m_listWidget = new QListWidget();
  m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);

  connect(m_listWidget, SIGNAL(itemClicked(QListWidgetItem *)), 
	  this, SLOT(selected(QListWidgetItem *)));

  //  sort(SortByName);
  sort(SortByMapDistance);

  foreach(Cache cache, m_cacheList) {
    CacheWidget *cacheWidget = new CacheWidget(&m_iconLoader, cache, m_mapCenter);

    QListWidgetItem *item = new QListWidgetItem(m_listWidget, QListWidgetItem::UserType);
    item->setSizeHint(cacheWidget->sizeHint());
    m_listWidget->addItem(item);
    m_listWidget->setItemWidget(item, cacheWidget);
  }

#ifdef FLICKCHARM
  m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  new FlickCharm(m_listWidget, this);
#endif
  vbox->addWidget(m_listWidget);

  QPushButton *button = new QPushButton(tr("Display all on map"));
  connect(button, SIGNAL(clicked()), this, SLOT(showOnMap()));
  vbox->addWidget(button);

  mainWidget->setLayout(vbox);
  setCentralWidget(mainWidget);
}

CacheListWindow::~CacheListWindow() {
  qDebug() << __FUNCTION__;
}
