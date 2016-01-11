// Filename: descriptionwindow.cpp

#include <QDebug>
#include <QDesktopServices>

#include "htmlview.h"
#include "descriptionwindow.h"

#ifdef FLICKCHARM
#include "flickcharm.h"
#endif

DescriptionWindow::DescriptionWindow(const Cache &cache, QWidget *parent) : 
  CustomWindow("DescriptionWindow", parent), m_cache(cache) {

  setWindowTitle(cache.description());

  HtmlView *htmlView = new HtmlView(cache.longDescription());
  connect(this, SIGNAL(zoomInPressed()), htmlView, SLOT(zoomIn()));
  connect(this, SIGNAL(zoomOutPressed()), htmlView, SLOT(zoomOut()));

  if(!m_cache.url().isEmpty())
    addMenuEntry(tr("Open in browser"), this, SLOT(openCacheUrl()));

#ifdef FLICKCHARM
  new FlickCharm(htmlView, this);
#endif

  setCentralWidget(htmlView);
}

DescriptionWindow::~DescriptionWindow() {
  qDebug() << __FUNCTION__;
}

void DescriptionWindow::openCacheUrl() {
  QDesktopServices::openUrl(m_cache.url());
}
