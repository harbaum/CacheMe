#include <QDebug>

#include "mapwidget.h"
#include "mapbuttonmanager.h"
#include "config.h"

MapButtonManager::MapButtonManager(MapWidget *mapWidget) : QList<MapButton*>(), m_iconLoader(NULL) {
  this->m_mapWidget = mapWidget;

  if(this->m_mapWidget->hiRez())
    this->m_iconLoader = new IconLoader(Config::MAP_BUTTON_MAX_SIZE_HIREZ);
  else
    this->m_iconLoader = new IconLoader(Config::MAP_BUTTON_MAX_SIZE);
}

MapButtonManager::~MapButtonManager() {
  
  // destroy all Buttons inside
  QList<MapButton *>::const_iterator i;
  for( i = begin(); i != end(); ++i )
    delete *i;
  
  delete m_iconLoader;
}

MapButton* MapButtonManager::create(const QString &name, int x, int y, MapButton::State state) {
  MapButton *but = new MapButton(m_iconLoader, name, x, y, state);
  append(but);
  return but;
}

void MapButtonManager::setState(const QString &name, MapButton::State state) {
  QList<MapButton *>::const_iterator i;
  for( i = begin(); i != end(); ++i )
    if((*i)->name() == name)
      (*i)->setState(state);
}

void MapButtonManager::change(const QString &name, const QString &newIcon) {
  QList<MapButton *>::const_iterator i;
  for( i = begin(); i != end(); ++i )
    if((*i)->name() == name)
      (*i)->change(m_iconLoader, newIcon);
}


void MapButtonManager::paint(QPainter *painter) {

  // check of buttons size is still appropriate for screen
  // three buttons need 3*size + 4 * border (==size/4) = 4*size
  int bsize = Config::MAP_BUTTON_MAX_SIZE;
  if(this->m_mapWidget->hiRez())
    bsize = Config::MAP_BUTTON_MAX_SIZE_HIREZ;

  if(bsize > m_mapWidget->size().height()/4)
    bsize = m_mapWidget->size().height()/4;

  if(bsize != m_iconLoader->size()) {
    // create a new iconLoader reflecting the new size
    delete m_iconLoader;
    this->m_iconLoader = new IconLoader(bsize);

    // force reload of all buttons to get them their new size
    QList<MapButton *>::const_iterator i;
    for( i = begin(); i != end(); ++i ) 
      (*i)->reload(m_iconLoader);
  }

  QList<MapButton *>::const_iterator i;
  for( i = begin(); i != end(); ++i )
    (*i)->paint(m_mapWidget->size(), painter);
}

bool MapButtonManager::mousePress(const QPointF &p) {
  bool change = false;

  // check if the mouse press event occured over one of the buttons  
  QList<MapButton *>::const_iterator i;
  for( i = begin(); i != end(); ++i ) 
    change |= (*i)->mousePress(m_mapWidget->size(), p);

  return change;
}

bool MapButtonManager::mouseMove(const QPointF &p) {
  bool change = false;

  QList<MapButton *>::const_iterator i;
  for( i = begin(); i != end(); ++i ) 
    change |= (*i)->mouseMove(m_mapWidget->size(), p);

  return change;
}

bool MapButtonManager::mouseRelease(const QPointF &p) {
  bool change = false;

  QList<MapButton *>::const_iterator i;
  for( i = begin(); i != end(); ++i ) 
    change |= (*i)->mouseRelease(m_mapWidget->size(), p);

  return change;
}

bool MapButtonManager::isInside(const QPointF &p) const {
  bool inside = false;

  QList<MapButton *>::const_iterator i;
  for( i = begin(); i != end(); ++i ) 
    inside |= (*i)->isInside(m_mapWidget->size(), p);

  return inside;
}
