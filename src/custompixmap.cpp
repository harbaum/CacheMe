#include "custompixmap.h"
#include "filefinder.h"

#include <QDebug>
#include <QPainter>

bool CustomPixmap::load(QSvgRenderer *renderer, const QString &name) {
  QString filename = FileFinder::find("icons", name);
  if(filename.isNull()) return false;

  renderer->load(filename);

  return true;
}

// load file into renderer and return resulting size
QSize CustomPixmap::prepare(QSvgRenderer *renderer, const QString &name, int size) {
  if(CustomPixmap::load(renderer, name + ".svg")) {
    // if no explicit size is given, then use default size of first image
    if(size > 0) 
      return QSize(renderer->defaultSize().width()*size/
		   renderer->defaultSize().height(), size);
    else 
      return(renderer->defaultSize());
  }

  return QSize(size, size);
}

// create empty pixmap for graphic effects
CustomPixmap::CustomPixmap(const QStringList &name, int size, 
			   State state, qreal angle) :
  QPixmap(size, size) {

  m_offset -= QPoint(width()/2, height()/2);

  // fill with transparency and saved icon name
  fill(Qt::transparent);
  this->m_name = name.join(",");
  this->m_state = state;  
  this->m_angle = angle;  
}

// create a Pixmap from svg file
CustomPixmap::CustomPixmap(QSvgRenderer *renderer, 
			   const QStringList &name, int size, qreal angle) :
  QPixmap(prepare(renderer, name[0], size)) {

  // try to find hotspot object, otherwise just center
  if(renderer->elementExists("hotspot")) {
    m_offset = QPoint(0,0) - 
      renderer->boundsOnElement("hotspot").center().toPoint();

    // adjust offset by file size/display size ratio if required 
    if(renderer->defaultSize() != this->size()) {
      qreal ratio = (qreal)this->width()/(qreal)renderer->defaultSize().width();
      m_offset *= ratio;
    }

  } else
    m_offset -= QPoint(width()/2, height()/2);

  // fill with transparency and saved icon name
  fill(Qt::transparent);
  this->m_name = name.join(",");
  this->m_state = CustomPixmap::Normal;
  this->m_angle = angle;
  
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  // do we have to rotate?
  if(angle != 0.0) {
    painter.translate( -m_offset );
    painter.rotate(m_angle);
    painter.translate( m_offset );
  }

  // draw all given images on top of each other
  QStringList::const_iterator i;
  for(i=name.begin();i!=name.end();i++) {
    // check if renderer already successfully loaded a file
    if((i == name.begin()) && renderer->isValid())
      renderer->render(&painter);
    else {
      if(load(renderer, *i + ".svg"))
	renderer->render(&painter);
      else {
	// unable to load image, draw some kind of "broken" icon
	painter.setPen(QPen(QBrush(Qt::red), size/5) );
	painter.drawEllipse(QRect(QPoint(size/8,size/8), this->size() - 
				  QSize(size/4, size/4)));
      }
    }
  }
}

CustomPixmap::~CustomPixmap() {
}

bool CustomPixmap::is(const QString &name, State state, qreal angle) {
  return m_name == name && m_state == state && m_angle == angle;
}

QPoint CustomPixmap::offset() {
  return m_offset;
}
