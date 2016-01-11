#include <QDebug>

#include <QPainter>
#include <QColor>
#include "compass.h"
#include "filefinder.h"

Compass::Compass(const Cache &cache, QWidget *parent) : 
  QWidget(parent), m_hasCompass(false), m_timer(NULL), 
  m_bgPix(NULL), m_calPix(NULL), m_cache(cache),
  m_target(cache.coordinate()) {
  
  init();
  load(m_cacheRenderer, cache.typeIconFile());
}

Compass::Compass(const Waypoint &wpt, QWidget *parent) : 
  QWidget(parent), m_hasCompass(false), m_timer(NULL), 
  m_bgPix(NULL), m_calPix(NULL), m_target(wpt.coordinate()) {

  init();
  load(m_cacheRenderer, wpt.iconFile());
}

void Compass::init() {
  m_roseRenderer = new QSvgRenderer();
  load(m_roseRenderer, "compass_rose");
  m_needleRenderer = new QSvgRenderer();
  load(m_needleRenderer, "compass_needle");
  m_backgroundRenderer = new QSvgRenderer();
  load(m_backgroundRenderer, "compass_background");
  m_cacheRenderer = new QSvgRenderer();

  m_roseAngle = -1.0;
  m_needleAngle = -1.0;   // angle between rose and needle
  m_roseSpeed = 0;
  m_calibration = -1.0;

  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

Compass::~Compass() {
  qDebug() << __FUNCTION__;
  
  delete m_roseRenderer;
  delete m_needleRenderer;
  delete m_backgroundRenderer;
  if(m_timer) delete m_timer;
  if(m_bgPix) delete m_bgPix;
  if(m_calPix) delete m_calPix;
}

void Compass::setAzimuth(qreal azimuth) {
  bool start = m_roseAngle < 0.0; 
  
  m_roseAngle = -azimuth;
	
  // make sure angle is in 0..359.99 range
  while(m_roseAngle >=  360.0) m_roseAngle -= 360.0;
  while(m_roseAngle <     0.0) m_roseAngle += 360.0;
	
  if(start) {
    m_roseVAngle = m_roseAngle;
	  
    //  a timer for smooth animations
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timerExpired()));
    m_timer->start(1000/RATE);

    update();
  }
}

void Compass::setTarget(const QGeoCoordinate &pos) {
  if(pos.isValid() && m_lastPos.isValid()) {
    m_target = pos;

    m_needleAngle = m_lastPos.azimuthTo(m_target);
    update();
  }
}

void Compass::setPosition(const QGeoPositionInfo &pos) {
  if(pos.isValid() && pos.coordinate().isValid()) {
    m_lastPos = pos.coordinate();
    
    if(!m_hasCompass) {
      if(pos.hasAttribute(QGeoPositionInfo::Direction) ||
	 m_roseAngle < 0.0) {

	if(pos.hasAttribute(QGeoPositionInfo::Direction))
	  setAzimuth(pos.attribute(QGeoPositionInfo::Direction));
	else 
	  setAzimuth(0);

      }
    }
    
    m_needleAngle = pos.coordinate().azimuthTo(m_target);
    update();
  }
}

void Compass::setPosition(const QCompassReading *reading) {
  setAzimuth(reading->azimuth());
  m_calibration = reading->calibrationLevel();

  m_hasCompass = true;
}

void Compass::timerExpired() {
  // generate rosevangle from roseangle and rosespeed
  qreal diff = (m_roseAngle - m_roseVAngle);
  while(diff >= 180.0) diff -= 360.0;
  while(diff < -180.0) diff += 360.0;

  m_roseSpeed += diff/5;
  m_roseSpeed *= 0.3;

  if(m_roseSpeed > 0.1 || m_roseSpeed < -0.1) {
    m_roseVAngle += m_roseSpeed;
    update();
  }
}

bool Compass::load(QSvgRenderer *renderer, const QString &name) {
  QString filename = FileFinder::find("icons", name + ".svg");
  if(filename.isNull()) return false;

  renderer->load(filename);
  return true;
}

void Compass::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  // determine square size of compass
  qreal s = size().width()>size().height()?size().height():size().width();

  // check if we have a matching buffered background, delete it if
  // size doesn't match
  if(m_bgPix && (m_bgPix->size() != QSize(s,s))) {
    delete m_bgPix;
    m_bgPix = NULL;

    if(m_calPix) {
      delete m_calPix;
      m_calPix = NULL;
    }
  }

  // create background pixmap if we don't have one
  if(!m_bgPix) {
    m_bgPix = new QPixmap(s, s);
    m_bgPix->fill(Qt::transparent);

    QPainter bgPainter(m_bgPix);
    bgPainter.setRenderHint(QPainter::Antialiasing, true);
    m_backgroundRenderer->render(&bgPainter);
  }
  
  // draw background
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.drawPixmap(QPoint((size().width()-s)/2,
			     (size().height()-s)/2), *m_bgPix);

  // draw calibration pie
  if(m_calibration >= 0.0) {
    // size of calibration indicator
    qreal sc = s/12;
    
    if(!m_calPix || m_calibration != m_calibrationV) {
      if(m_calPix) delete m_calPix;

      QRectF rect(2, 2, sc, sc);

      m_calPix = new QPixmap(sc+4, sc+4);
      m_calPix->fill(Qt::transparent);
      QPainter calPainter(m_calPix);
      calPainter.setRenderHint(QPainter::Antialiasing, true);

      calPainter.setBrush( QColor(Qt::darkGray) );
      calPainter.setPen(QPen(QBrush(Qt::transparent), 2) );
      calPainter.drawEllipse(rect);
      
      calPainter.setBrush( QColor(Qt::green) );
      calPainter.drawPie(rect, 16*90, -16 * 360 * m_calibration);
      
      calPainter.setPen(QPen(QBrush(Qt::white), 2) );
      calPainter.setBrush( QColor(Qt::transparent) );
      calPainter.drawEllipse(rect);

      m_calibrationV = m_calibration;
    }
      
    painter.drawPixmap(QPoint((size().width()+s)/2-sc-4-sc/4,
			      (size().height()+s)/2-sc-4-sc/4), *m_calPix);
  }
  
  if(m_roseAngle < 0.0) return;

  // rotate painter around center
  painter.translate( size().width()/2, size().height()/2 );
  painter.rotate(m_roseVAngle);
  painter.translate( -size().width()/2, -size().height()/2 );
  
  m_roseRenderer->render(&painter, QRect(QPoint((size().width()-s)/2,
					       (size().height()-s)/2), 
					QSize(s,s)));

  if(m_needleAngle < -0.0) return;

  painter.translate( size().width()/2, size().height()/2 );
  painter.rotate(m_needleAngle);
  painter.translate( -size().width()/2, -size().height()/2 );
  
  m_needleRenderer->render(&painter, QRect(QPoint((size().width()-s)/2,
					       (size().height()-s)/2), 
					QSize(s,s)));

  m_cacheRenderer->render(&painter, QRect(QPoint((size().width()-s/5)/2,
						(size().height()-10*s/14)/2),
			  QSize(s/5, s/5)));  
}

void Compass::setType() {
  load(m_cacheRenderer, m_cache.typeIconFile());
  update();
}

void Compass::setType(const Waypoint &wpt) {
  load(m_cacheRenderer, wpt.iconFile());
  update();
}

int Compass::heightForWidth(int w) const {
  return w;
}
