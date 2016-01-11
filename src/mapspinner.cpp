#include <math.h>

#include "mapwidget.h"
#include "mapspinner.h"

MapSpinner::MapSpinner(MapWidget *mapWidget) : m_mapWidget(mapWidget) {
  this->m_mapWidget = mapWidget;

  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(timerExpired()));
  m_timer->start(100);
  m_step = 0;
}

QRect MapSpinner::area() {
  // determine spinner size
  int size = m_mapWidget->size().width()/20;
  if(size > 16) size = 16;

  // size/4 is dot radius, size/8 is shadow offset
  return QRect(m_mapWidget->size().width()/2 - size - size/4,
	       m_mapWidget->size().height()/2 - size - size/4,
	       2*(size + size/4) + size/8, 2*(size + size/4) + size/8);
}

MapSpinner::~MapSpinner() {
  m_mapWidget->update(area());
  delete m_timer;
};

void MapSpinner::timerExpired() {
  if(++m_step == 8) m_step = 0;
  m_mapWidget->update(area());
}

void MapSpinner::paint(QPainter *painter) {
  // determine spinner size
  int size = m_mapWidget->size().width()/20;
  if(size > 16) size = 16;

  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen( Qt::transparent );

  QPointF center(m_mapWidget->size().width()/2,
		 m_mapWidget->size().height()/2);

  for(int i =0;i<8;i++) {
    float angle = (i+m_step)*M_PI/4;
    QPointF pos = center - QPointF(size * sin(angle),
				  size * cos(angle));
    
    painter->setBrush( QColor(0, 0, 0, 8*(i+1)) );
    painter->drawEllipse(pos + QPointF(size/8, size/8),
			 size/4, size/4);

    painter->setBrush( QColor(64, 64, 64, 32*(i+1)-1) );
    painter->drawEllipse(pos, size/4, size/4);
  }
}
