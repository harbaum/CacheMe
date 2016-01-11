#ifdef BUILTINBROWSER

#include <QDebug>
#include <QPushButton>
#include <QComboBox>
#include <QWebView>
#include "browserwindow.h"

#ifdef FLICKCHARM
#include "flickcharm.h"
#endif

BrowserWindow::BrowserWindow(const QUrl &url, QWidget *parent) :
  CustomWindow("BrowserWindow", parent) {

  setWindowTitle(tr("Browser"));

  QWidget *vbox = new QWidget(this);
  QVBoxLayout *vboxLayout = new QVBoxLayout;
  vboxLayout->setContentsMargins(0,0,0,0);
  vbox->setLayout(vboxLayout);

  QWebView *view = new QWebView(parent);
#ifdef FLICKCHARM
  //  new FlickCharm(view, this);
#endif
  view->load(url);
  vboxLayout->addWidget(view, 1);

  addMenuEntry(tr("Back"), view, SLOT(back()));

  setCentralWidget(vbox);
}

#endif
