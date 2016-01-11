
#include "mapbutton.h"
#include "iconloader.h"

#include <QDebug>
#include <QPainter>

void MapButton::renderNormal() {
  // already rendered?
  if(this->m_pixNormal != NULL) return;

  int shadowOffset = this->m_pixmap->width()/SHADOW_OFFSET;
  int shadowTransparency = 64;

  this->m_pixNormal = new QPixmap(this->m_pixmap->width()+shadowOffset, 
				  this->m_pixmap->height()+shadowOffset);
  this->m_pixNormal->fill(Qt::transparent);

  QPainter painter(this->m_pixNormal);
  painter.setRenderHint(QPainter::Antialiasing, true);

  // create shadow
  painter.drawPixmap(shadowOffset, shadowOffset, *this->m_pixmap);

  // convert to black
  painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
  painter.fillRect(this->m_pixNormal->rect(), Qt::black);
  // and make it more transparent
  painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
  painter.fillRect(this->m_pixNormal->rect(), QColor(0, 0, 0, shadowTransparency));
  
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  painter.drawPixmap(0, 0, *this->m_pixmap);
}

void MapButton::renderSelected() {
  // already rendered?
  if(this->m_pixSelected != NULL) return;

  int shadowOffset = this->m_pixmap->width()/SHADOW_OFFSET - 
    this->m_pixmap->width()/(4*SHADOW_OFFSET);
  int imgOffset = this->m_pixmap->width()/(4*SHADOW_OFFSET);
  int shadowTransparency = 64;

  this->m_pixSelected = new QPixmap(this->m_pixmap->width()+shadowOffset, 
				  this->m_pixmap->height()+shadowOffset);
  this->m_pixSelected->fill(Qt::transparent);

  QPainter painter(this->m_pixSelected);
  painter.setRenderHint(QPainter::Antialiasing, true);

  // create shadow
  painter.drawPixmap(shadowOffset, shadowOffset, *this->m_pixmap);

  // convert to black
  painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
  painter.fillRect(this->m_pixNormal->rect(), Qt::black);
  // and make it more transparent
  painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
  painter.fillRect(this->m_pixNormal->rect(), QColor(0, 0, 0, shadowTransparency));
  
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  painter.drawPixmap(imgOffset, imgOffset, *this->m_pixmap);
}

void MapButton::renderDisabled() {
  // already rendered?
  if(!this->m_pixmap || this->m_pixDisabled != NULL) return;

  renderNormal();  // disabled bitmap derives from normal one

  this->m_pixDisabled = new QPixmap(*this->m_pixNormal);

  // make the entire button 50 (more) transparent if it's disabled
  QPainter painter(this->m_pixDisabled);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
  painter.fillRect(this->m_pixDisabled->rect(), QColor(0, 0, 0, 64));
}

MapButton::MapButton(IconLoader *loader,
     const QString &name, int x, int y, State state) :
  m_pixNormal(NULL), m_pixDisabled(NULL), m_pixSelected(NULL) {

  this->m_pixmap = loader->load("map_overlay_" + name);
  this->m_name = name;
  this->m_state = state;
  this->m_p.setX(x);
  this->m_p.setY(y);
  this->m_size = loader->size();
}

MapButton::~MapButton() {
  releaseImages();
}

void MapButton::reload(IconLoader *loader) {
  Q_ASSERT(this->m_size != loader->size());

  // ok, we need to resize, so release existing images
  releaseImages();
 
  this->m_pixmap = loader->load("map_overlay_" + this->m_name);
  this->m_size = loader->size();
}

void MapButton::releaseImages() {
  if(m_pixDisabled) {
    delete this->m_pixDisabled;
    m_pixDisabled = NULL;
  }

  if(m_pixNormal) {
    delete this->m_pixNormal;
    m_pixNormal = NULL;
  }

  if(m_pixSelected) {
    delete this->m_pixSelected;
    this->m_pixSelected = NULL;
  }
}

void MapButton::change(IconLoader *loader, const QString &name) {
  releaseImages();
  this->m_pixmap = loader->load("map_overlay_" + name);
}

QString MapButton::name() const {
  return m_name;
}

bool MapButton::setState(const State &state) {
  qDebug() << __FUNCTION__ << name();

  State prev = this->m_state;
  this->m_state = state;

  return prev != this->m_state;
}

QPoint MapButton::screenPos(const QSizeF &size) const {
  QPoint p;

  if(m_p.x() >= 0) 
    p.setX(m_p.x() *  m_pixmap->width()/4 + (m_p.x()-1) *  m_pixmap->width());
  else
    p.setX(size.width() - m_pixmap->width() +
	   m_p.x() *  m_pixmap->width()/4 + (m_p.x()+1) * m_pixmap->width());

  if(m_p.y() >= 0) 
    p.setY(m_p.y() * m_pixmap->height()/4 + (m_p.y()-1) * m_pixmap->height());
  else
    p.setY(size.height() - m_pixmap->height() +
	   m_p.y() * m_pixmap->height()/4 + (m_p.y()+1) * m_pixmap->height());

  return p;
}

void MapButton::paint(const QSizeF &size, QPainter *painter) {

  if(m_state != Hidden) {
    QPixmap *pix = NULL;

    switch(m_state) {
    case Normal:
      renderNormal();
      pix = m_pixNormal;
      break;
    case Disabled:
      renderDisabled();
      pix = m_pixDisabled;
      break;
    case Selected:
      renderSelected();
      pix = m_pixSelected;
      break;
    default:
      break;
    }
    
    painter->drawPixmap(screenPos(size), *pix);
  }
}

bool MapButton::isInside(const QSizeF &size, const QPointF &p) const {
  QPoint m = screenPos(size);

  return (  p.x() > m.x() &&
	    p.x() < m.x() + m_size &&
	    p.y() > m.y() &&
	    p.y() < m.y() + m_size);
}

bool MapButton::mousePress(const QSizeF &size, const QPointF &p) {
  if(m_state == Disabled) return false;

  if(m_state == Normal && isInside(size, p)) {
    m_state = Selected;
    m_downOn = true;
    return true;
  }

  m_downOn = false;
  return false;
}

bool MapButton::mouseMove(const QSizeF &size, const QPointF &p) {
  if(m_state == Disabled) return false;

  // mouse is not over this button anymore, release it
  if(m_state == Selected && !isInside(size, p)) {
    m_state = Normal;
    return true;
  }

  // mouse is again over this button anymore, activate it
  if(m_state == Normal && isInside(size, p)) {
    m_state = Selected;
    return true;
  }

  return false;
}

bool MapButton::mouseRelease(const QSizeF &size, const QPointF &p) {
  if(m_state == Disabled) return false;

  if(isInside(size, p) && m_state == Selected) {
    m_state = Normal;
    
    // if the user actually pressed this, emit click
    if(m_downOn)
      emit clicked();

    return true;
  }

  return false;
}
