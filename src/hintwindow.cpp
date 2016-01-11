#include <QDebug>
#include <QPushButton>
#include <QComboBox>
#include "hintwindow.h"
#include "htmlview.h"

#ifdef FLICKCHARM
#include "flickcharm.h"
#endif

void HintWindow::showImage() {

  foreach(Image image, m_cache.images()) {
    if(image.name().compare(m_cBox->currentText()) == 0) {

      // build a html page
      QString html = 
	"<html><body><center>"
	"<h1>" + image.name() + "</h1><br>";

      if(!image.description().isEmpty())
	html += image.description() + "<br>";

      html += "<img src=\"" + image.url() + "\">"
	"</center></body></html>";

      Description desc;
      desc.set(true, html);

      CustomWindow *window = new CustomWindow("Image", this);
      window->setWindowTitle(tr("Image"));
      HtmlView *htmlView = new HtmlView(desc, this);
#ifdef FLICKCHARM
      new FlickCharm(htmlView, this);
#endif
      window->setCentralWidget(htmlView);
      window->show();

      return;
    }
  }
}

HintWindow::HintWindow(const Cache &cache, QWidget *parent) :
  CustomWindow("HintWindow", parent), m_cache(cache) {
  setWindowTitle(tr("Hint"));

  QWidget *vbox = new QWidget(this);
  QVBoxLayout *vboxLayout = new QVBoxLayout;
  vboxLayout->setContentsMargins(0,0,0,0);
  vbox->setLayout(vboxLayout);

  HtmlView *htmlView = new HtmlView(cache.hint(), this);
#ifdef FLICKCHARM
  new FlickCharm(htmlView, this);
#endif
  vboxLayout->addWidget(htmlView, 1);

  if(cache.images().size() > 0) {
    vboxLayout->addWidget(new QLabel(tr("Images:")), 0);

    m_cBox = new QComboBox;

    foreach(Image image, cache.images()) 
      m_cBox->addItem(image.name());
    
    QWidget *hBox = new QWidget;
    QHBoxLayout *hboxLayout = new QHBoxLayout;
    hboxLayout->setContentsMargins(0,0,0,0);
    hBox->setLayout(hboxLayout);   

    hboxLayout->addWidget(m_cBox, 1);
    QPushButton *button = new QPushButton(tr("Show"));
    connect(button, SIGNAL(clicked()), this, SLOT(showImage()));
    hboxLayout->addWidget(button, 0);

    vboxLayout->addWidget(hBox, 0);
  }

  setCentralWidget(vbox);
}
