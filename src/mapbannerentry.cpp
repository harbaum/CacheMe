#include <QDebug>

#include "mapwidget.h"
#include "mapbannerentry.h"
#include "config.h"

QRect MapBannerEntry::area() {
  QFont font("Arial");
  font.setPixelSize(Config::MAP_BANNER_FONT_SIZE);
  font.setWeight(QFont::Bold);

  QRect rect = QFontMetrics(font).
    boundingRect(QRect(QPoint(0,0), m_mapWidget->size().toSize()), 
		 Qt::TextWordWrap | Qt::AlignHCenter, m_message);

  rect.setX(0);
  rect.setWidth(m_mapWidget->size().width());
  rect.setHeight(rect.height() + 2*Config::MAP_BANNER_BORDER);

  return rect;
}

MapBannerEntry::MapBannerEntry(MapWidget *mapWidget, const QString &message) : 
  m_state(Waiting), m_counter(STEPS), m_mapWidget(mapWidget), m_important(false) { 

  this->m_message = message;
  
  // handle "very important" messages
  if((this->m_message.length() > 1) && (this->m_message.at(0) == QChar('!'))) {
    this->m_message.remove(0, 1);
    this->m_important = true;
  }

  this->m_timer = new QTimer(this);
  connect( this->m_timer, SIGNAL(timeout()), this, SLOT(timeout()) );

  // while the banner appears, events are generated at 20Hz
  m_timer->start(APPEAR / STEPS);
}

MapBannerEntry::~MapBannerEntry() { 
  qDebug() << __FUNCTION__; 
}

void MapBannerEntry::timeout() {

  switch(m_state) {
  case Waiting:
    break;

  case Appearing:
    if(--m_counter == 0) {
      m_state = Visible;
      m_timer->setSingleShot(true);

      m_timer->setInterval(m_important?5*STAY:STAY);
    }
   break;
    
  case Visible:
    m_state = Disappearing;
    m_timer->setSingleShot(false);
    m_timer->start(APPEAR / STEPS);
    break;
    
  case Disappearing:
    if(++m_counter == STEPS) {
      m_state = Done;

      // cleanup ...
      delete m_timer;
      m_timer = NULL;

      emit done(this);
    }
    break;

  case Done:
    break;
  }

  // force map update
  m_mapWidget->update(area());
}

bool MapBannerEntry::isDisappearing() const {
  return m_state == Disappearing;
}

void MapBannerEntry::paint(QPainter *painter, bool first) {
  if(m_state == Waiting) {
    if(!first) {
      return;
    } else {
      m_state = Appearing;
    }
  }

  QFont font("Arial");
  font.setPixelSize(Config::MAP_BANNER_FONT_SIZE);
  font.setWeight(QFont::Bold);

  painter->setFont(font);
  QRect rect = painter->fontMetrics().
    boundingRect(QRect(QPoint(0,0), m_mapWidget->size().toSize()), 
		 Qt::TextWordWrap | Qt::AlignHCenter, m_message);

  rect.setHeight(rect.height() + 2*Config::MAP_BANNER_BORDER);

  rect.setX(0);
  rect.setWidth(m_mapWidget->size().width());
  rect.moveTop(-rect.height()*m_counter/STEPS);
    
  // no outline and fill with slightly transparent yellow
  painter->setBrush( QColor(255, 200, 0, 224) );
  painter->setPen( Qt::transparent );
  painter->drawRect ( rect );
  
  painter->setPen( QPen(Qt::black));
  rect.setTop(rect.top() + Config::MAP_BANNER_BORDER);
  rect.setHeight(rect.height() - Config::MAP_BANNER_BORDER);
  painter->drawText(rect, Qt::TextWordWrap | Qt::AlignHCenter, m_message);
}

MapBannerEntryList::MapBannerEntryList() { 
  qDebug() << __FUNCTION__;
}

MapBannerEntryList::~MapBannerEntryList() { 
  qDebug() << __FUNCTION__ << size();
  while(size()) delete takeFirst(); 
}

QList<MapBannerEntry*>::const_iterator MapBannerEntryList::begin() const {
  return QList<MapBannerEntry*>::begin();
}

QList<MapBannerEntry*>::const_iterator MapBannerEntryList::end() const {
  return QList<MapBannerEntry*>::end();
}
