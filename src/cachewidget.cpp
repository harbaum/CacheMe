#include "cachewidget.h"

#include <QLocale>
#include <QHBoxLayout>
#include <QPainter>
#include <QLabel>
#include <QSvgRenderer>

#include "filefinder.h"
#include "iconlabel.h"

QWidget *CacheWidget::directionWidget(qreal angle, qreal dist) {
  QWidget *vbox = new QWidget();
  QVBoxLayout *vBoxLayout = new QVBoxLayout;
  vBoxLayout->setContentsMargins(0,0,0,0);
  vbox->setLayout(vBoxLayout);

  QString filename = FileFinder::find("icons", "direction_arrow.svg");
  if(!filename.isNull()) {
    QPixmap *pix = new QPixmap(32, 32);
    pix->fill(Qt::transparent);

    QSvgRenderer renderer(filename);

    QPainter painter(pix);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.translate(pix->size().width()/2, pix->size().height()/2);
    painter.rotate(angle);
    painter.translate(-pix->size().width()/2, -pix->size().height()/2);
    renderer.render(&painter);  
    
    QLabel *label = new QLabel();
    label->setAlignment(Qt::AlignHCenter);
    label->setPixmap(*pix);

    vBoxLayout->addWidget(label);
  }

  QString str;
  QLocale locale;

  if(locale.measurementSystem() == QLocale::ImperialSystem) {
    /* 10 ft == 3.0480m / 1m == 3.28084 ft */ 
    /* 10 yd == 9.144m / 1m == 1.0936133 yd */ 
    /* 100 yd == 91.44m / 1m == 1.0936133 yd */ 
    /* 1 mi = 1609.344m */ 
    if(dist<3.048)         str = QString::number(dist * 3.28084, 'f', 2) + " ft";
    else if(dist<9.144)    str = QString::number(dist * 1.0936133, 'f', 2) + " yd";
    else if(dist<91.44)    str = QString::number(dist * 1.0936133, 'f', 1) + " yd";
    else if(dist<16093.44) str = QString::number(dist / 1609.344, 'f', 2) + " mi";
    else if(dist<160934.4) str = QString::number(dist / 1609.344, 'f', 1) + " mi";
    else                   str = QString::number(dist / 1609.344, 'f', 0) + " mi";
  } else {
    if(dist<10)          str = QString::number(dist, 'f', 2) + " m";
    else if(dist<100)    str = QString::number(dist, 'f', 1) + " m";
    else if(dist<1000)   str = QString::number(dist, 'f', 0) + " m";
    else if(dist<100000) str = QString::number(dist/1000, 'f', 1) + " km";
    else                 str = QString::number(dist/1000, 'f', 0) + " km";
  }

  QLabel *label = new QLabel(str);
  label->setAlignment(Qt::AlignHCenter);
  vBoxLayout->addWidget(label);

  return vbox;
}

// render five stars into one bitmap
QPixmap CacheWidget::renderRating(qreal rating, int size) {
  QPixmap pixmap(5*size, size);
  pixmap.fill(Qt::transparent);
  
  // create single star bitmaps if required
  if(!m_pixStar)      
    m_pixStar = m_iconLoader->load("star", size);
  
  if(!m_pixStarLight) 
    m_pixStarLight = m_iconLoader->load("star_light", size);
  
  if(!m_pixStarHalf) {
    m_pixStarHalf = new QPixmap(*m_pixStar);
    QPainter halfPainter(m_pixStarHalf);
    const QRectF half(size/2, 0, size/2, size);
    halfPainter.drawPixmap(half, *m_pixStarLight, half);
  }
  
  // draw all five stars
  QPainter painter(&pixmap);
  QPixmap *star;
  int i, j = 2*rating;
  for(i=0;i<5;i++) {
    if(j < 2*(i+1)-1)    star = m_pixStarLight;
    else if(j < 2*(i+1)) star = m_pixStarHalf;
    else                 star = m_pixStar;

    painter.drawPixmap(i*size, 0, *star);
  }
  
  return pixmap;
}

QWidget *CacheWidget::ratingWidget(const QString &name, const Rating &rating) {
  QWidget *widget = new QWidget(this);
  
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setContentsMargins(0,0,0,0);
  widget->setLayout(layout);
  
  QLabel *label = new QLabel(name, this);
  QFont font = label->font();
  
  font.setPointSize(font.pointSize()*1.0);
  label->setFont(font);
  label->setWordWrap(true);
  QFontInfo fontInfo(label->font());
  layout->addWidget(label, 1);
  
  label = new QLabel();
  label->setPixmap(renderRating(rating.value(),
				2*QFontMetrics(font).height()/3));
  layout->addWidget(label);
  
  return widget;
}

QString CacheWidget::name() const {
  return m_name;
}

QWidget *CacheWidget::containerWidget(const QString &name, const Container &container) {
  QWidget *widget = new QWidget(this);
  
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setContentsMargins(0,0,0,0);
  widget->setLayout(layout);
  
  QLabel *label = new QLabel(name, this);
  QFont font = label->font();
  
  font.setPointSize(font.pointSize()*1.0);
  label->setFont(font);
  label->setWordWrap(true);
  QFontInfo fontInfo(label->font());
  layout->addWidget(label, 1);
  
  layout->addWidget(m_iconLoader->newWidget(container.iconFile(),
					    2*QFontMetrics(font).height()/3));
  
  return widget;
}

CacheWidget::CacheWidget(IconLoader *iconLoader, const Cache &cache, 
			 QGeoCoordinate &coo, QWidget *parent) : 
  QWidget(parent) {
  m_pixStar = m_pixStarLight = m_pixStarHalf = NULL;
  m_iconLoader = iconLoader;
  m_name = cache.name();
  
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins(4,4,4,4);
  layout->setSpacing(0);

  // main top label
  layout->addWidget(new IconLabel(iconLoader, cache,
				  cache.description(), 1.5, true, this));

  QWidget *hbox = new QWidget();
  QHBoxLayout *hBoxLayout = new QHBoxLayout;
  hBoxLayout->setContentsMargins(0,0,0,0);
  hbox->setLayout(hBoxLayout);

  QWidget *vbox = new QWidget();
  QVBoxLayout *vBoxLayout = new QVBoxLayout;
  vBoxLayout->setContentsMargins(0,0,0,0);
  vbox->setLayout(vBoxLayout);
  
  // add rating stars
  vBoxLayout->addWidget(ratingWidget(tr("Difficulty")+":", cache.difficulty()));
  vBoxLayout->addWidget(ratingWidget(tr("Terrain")+":", cache.terrain()));
  vBoxLayout->addWidget(containerWidget(tr("Container")+":", cache.container()));

  hBoxLayout->addWidget(vbox);
  hBoxLayout->addWidget(directionWidget(coo.azimuthTo(cache.coordinate()),
					coo.distanceTo(cache.coordinate())));
  
  layout->addWidget(hbox);
  setLayout(layout);
}
