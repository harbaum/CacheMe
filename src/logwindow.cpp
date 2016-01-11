// Filename: logwindow.cpp

#include <QDebug>

#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>

#include "htmlview.h"
#include "logwindow.h"
#include "iconlabel.h"
#include "logwidget.h"
#include "logeditwindow.h"

#ifdef FLICKCHARM
#include "flickcharm.h"
#endif

#ifdef Q_OS_SYMBIAN
#define LogWidgetBase QWidget
#else
#include <QGroupBox>
#define LogWidgetBase QGroupBox
#endif

LogWindow::LogWindow(const Cache &cache, CacheProvider *cacheProvider, 
		     QWidget *parent) : 
  CustomWindow("LogWindow", parent),
  m_iconLoader(24), m_cache(cache), m_cacheProvider(cacheProvider) {

  setWindowTitle(tr("Logs"));

  QScrollArea *scrollArea = new QScrollArea(this);
  scrollArea->setWidgetResizable(true);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

#ifdef FLICKCHARM
  new FlickCharm(scrollArea, this);
#endif

  QWidget *scrollWidget = new QWidget;
  QVBoxLayout *scrollvbox = new QVBoxLayout;
  scrollvbox->setContentsMargins(0,0,0,0);

  if(m_cacheProvider) {
    if(m_cacheProvider->loggingIsSupported() ||
       !m_cacheProvider->getInfoString("LogUrl").isEmpty()) {
      QPushButton *button = new QPushButton(tr("Create new log"));
      connect(button, SIGNAL(clicked()), this, SLOT(createLog()));
      scrollvbox->addWidget(button,0,0);
    }
  }

  foreach(Log log, cache.logs()) {
    LogWidget *tlog = new LogWidget(&m_iconLoader, log, this);
    scrollvbox->addWidget(tlog,0,0);
  }

  scrollvbox->addStretch(1);
  scrollWidget->setLayout(scrollvbox);
  scrollArea->setWidget(scrollWidget);

  setCentralWidget(scrollArea);
}

LogWindow::~LogWindow() {
  qDebug() << __FUNCTION__;
}

void LogWindow::createLog() {
  qDebug() << __FUNCTION__;

  if(m_cacheProvider && m_cacheProvider->loggingIsSupported()) {
    LogEditWindow *logEditWindow = 
      new LogEditWindow(m_cache, m_cacheProvider, this);
    logEditWindow->show();

  } else {
    QUrl url(m_cacheProvider->getInfoString("LogUrl").arg(m_cache.name()));
    QDesktopServices::openUrl(url);
  }
}
