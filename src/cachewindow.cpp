// Filename: cachewindow.cpp

#include <QDebug>

#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QKeyEvent>
#include <QPushButton>
#include <QFontInfo>

#include "htmlview.h"
#include "cachewindow.h"
#include "descriptionwindow.h"
#include "navigationwindow.h"
#include "logwindow.h"
#include "attributewindow.h"
#include "hintwindow.h"
#include "iconlabel.h"

#include "swapbox.h"

#ifdef FLICKCHARM
#include "flickcharm.h"
#endif

void CacheWindow::showHint() {
  HintWindow *hintWindow = new HintWindow(m_cache, this);
  hintWindow->show();
}

#define HACK

// a qpushbutton with a small icon on the right
class Button : public QPushButton {
public:
  Button(const QString &text, QWidget *parent = 0) : 
    QPushButton(text, parent), m_pixmap(NULL) {
  };

  void addIcon(IconLoader *loader, const QString &name) {
#ifndef HACK
    if(!m_pixmap) {
      QMargins margins = contentsMargins();
      qDebug() << __FUNCTION__ << 
	margins.left() << margins.right();
      margins.setRight(margins.right() + iconSize().width());
      margins.setLeft(0);
      setContentsMargins(margins);
      qDebug() << __FUNCTION__ << 
	margins.left() << margins.right();
    }
#endif
    m_pixmap = loader->load("button_" + name, iconSize().width());
#ifdef HACK
    // append some spaces to the text to shift it a little bit
    // to the left
    setText(text()+"           ");
#endif
  }

#ifndef HACK // now done by the additional spaces
  QSize sizeHint() const {
    QSize size = QPushButton::sizeHint();

    // request additional space for icon
    if(m_pixmap)
      size += QSize(m_pixmap->width(),0);

    return size;
  }
#endif

  void paintEvent(QPaintEvent *event) {
    // draw original button
    QPushButton::paintEvent(event);

    if(m_pixmap) {
      // add pixmap on top
      QRect rect = contentsRect();
      QPainter painter(this);
#ifdef HACK
      painter.drawPixmap(rect.x()+rect.width()-1.3*m_pixmap->width(), 
      			 rect.y()+(rect.height()-m_pixmap->height())/2, 
			 *m_pixmap);
#else
      painter.drawPixmap(rect.x(), 
			 rect.y()+(rect.height()-m_pixmap->height())/2, 
			 *m_pixmap);
      painter.drawPixmap(rect.x()+rect.width(), 
			 rect.y()+(rect.height()-m_pixmap->height())/2, 
			 *m_pixmap);
#endif
    }
  }

private:
  QPixmap *m_pixmap;
};

void CacheWindow::reload() {
  qDebug() << __FUNCTION__;

  // tell cache provider to forget about this cache
  QMap<QString, QVariant> params;
  params.insert("Type", "Drop"); 
  params.insert("Cache", m_cache.name());
  m_cacheProvider->requestCustom(params);

  // close this window ...
  close();

  // ... and cause it to re-open
  m_cacheProvider->requestDetail(m_cache.name());
}

CacheWindow::CacheWindow(const Cache &cache, 
	 LocationProvider *locationProvider, 
	 WaypointDb *wptDb, MapWidget *mapWidget, 
	 CacheProvider *cacheProvider, QWidget *parent) : 
  CustomWindow("CacheWindow", parent),  m_cache(cache), m_iconLoader(24),
  m_locationProvider(locationProvider), m_wptDb(wptDb), m_mapWidget(mapWidget),
  m_cacheProvider(cacheProvider) {

  setWindowTitle(cache.name());

  qDebug() << __FUNCTION__ << cache;

  addMenuEntry(tr("Reload"), this, SLOT(reload()));

  QWidget *vbox = new QWidget;
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(0);
  vbox->setLayout(layout);
  setCentralWidget(vbox);

  // ---------- general info on top -------------
  
  // ------------- title with icon -------------
  layout->addWidget(new IconLabel(&m_iconLoader, cache, 
				  cache.description(), 1.5));

  // if there's a short description, then use a swapbox to place
  // the description in landscape in the left window half

  // ------------- short description -------------
  SwapBox *sbox = NULL;
  HtmlView *htmlView = NULL;
  if(cache.shortDescription().isSet()) {
    sbox = new SwapBox();

    // html view below
    htmlView = new HtmlView(cache.shortDescription());
#ifdef FLICKCHARM
    new FlickCharm(htmlView, this);
#endif

    connect(this, SIGNAL(zoomInPressed()), htmlView, SLOT(zoomIn()));
    connect(this, SIGNAL(zoomOutPressed()), htmlView, SLOT(zoomOut()));

    sbox->addWidget(htmlView, 1);
    layout->addWidget(sbox);

    // create a new vbox for the right (button) half
    vbox = new QWidget;
    layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    vbox->setLayout(layout);
    sbox->addWidget(vbox);
  }

  // ------------- button to open long description -------------
  if(cache.longDescription().isSet()) {
    Button *button = new Button(tr("Description"));
    button->addIcon(&m_iconLoader, "new_window");    

    connect(button, SIGNAL(clicked()), this, SLOT(showDescription()));

    layout->addWidget(button);
  }

  // ------------- button to open navigation window -------------
  if(cache.coordinate().isValid()) {
    Button *button = new Button(tr("Navigation"));
    button->addIcon(&m_iconLoader, "new_window");    
    
    connect(button, SIGNAL(clicked()), this, SLOT(showNavigation()));
    
    layout->addWidget(button);
  }

  // ------------- button to display hint -------------
  if(cache.hint().isSet() || cache.images().size() > 0) {
    Button *button = new Button(tr("Hint"));
    button->addIcon(&m_iconLoader, "new_window");    

    connect(button, SIGNAL(clicked()), this, SLOT(showHint()));

    layout->addWidget(button);
  }

  // ------------- button to display logs -------------
  if(cache.logs().size()) {
    Button *button = new Button(tr("Logs"));
    button->addIcon(&m_iconLoader, "new_window");    

    connect(button, SIGNAL(clicked()), this, SLOT(showLogs()));

    layout->addWidget(button);
  }

  // ------------- button to display attributes -------------
  if(cache.attributes().size()) {
    Button *button = new Button(tr("Attributes"));
    button->addIcon(&m_iconLoader, "new_window");    

    connect(button, SIGNAL(clicked()), this, SLOT(showAttributes()));

    layout->addWidget(button);
  }

  QWidget *stretchWidget = new QWidget;
  if(sbox) sbox->setPortraitOnly(stretchWidget);
  layout->addWidget(stretchWidget, 1);
}

CacheWindow::~CacheWindow() {
  qDebug() << __FUNCTION__;
}

void CacheWindow::showDescription() {
  DescriptionWindow *descriptionWindow = 
    new DescriptionWindow(m_cache, this);

  descriptionWindow->show();
}

void CacheWindow::showNavigation() {
  NavigationWindow *navigationWindow = 
    new NavigationWindow(m_cache, m_locationProvider, m_wptDb, 
			 m_mapWidget, this);

  navigationWindow->show();
}

void CacheWindow::showLogs() {
  LogWindow *logWindow = 
    new LogWindow(m_cache, m_cacheProvider, this);

  logWindow->show();
}

void CacheWindow::showAttributes() {
  AttributeWindow *attributeWindow = 
    new AttributeWindow(m_cache.attributes(), this);

  attributeWindow->show();
}
