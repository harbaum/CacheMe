#include <QDebug>
#include <QGraphicsGeoMap>
#include <QPainter>

#include "mapbubble.h"
#include "config.h"

class Bubble : public QPixmap {
private:
  class BubblePath : public QPainterPath {
  public:
    BubblePath(bool topTip, const QRect &rect, qreal radius, qreal tipOffset) : 
      QPainterPath() {

      qreal x0 = rect.x();
      qreal x1 = x0 + radius;
      qreal x2 = x0 + rect.width() - radius;
      qreal x3 = x0 + rect.width();

      qreal px = rect.x() + rect.width()/2 + tipOffset;

      qreal y0 = rect.y() + (topTip?radius:0);
      qreal y1 = y0 + radius;
      qreal y2 = y0 + rect.height() - radius - radius;
      qreal y3 = y0 + rect.height() - radius;

      moveTo(x1, y0);
      if(topTip) {
	lineTo(px-radius/2, y0);
	lineTo(px, y0-radius);
	lineTo(px+radius/2, y0);
      }
      lineTo(x2, y0);            // top line
      quadTo(x3, y0, x3, y1);
      lineTo(x3, y2);
      quadTo(x3, y3, x2, y3);
      if(!topTip) {
	lineTo(px+radius/2, y3);
	lineTo(px, y3+radius);
	lineTo(px-radius/2, y3);
      }
      lineTo(x1, y3);            // bottom line
      quadTo(x0, y3, x0, y2);
      lineTo(x0, y1);
      quadTo(x0, y0, x1, y0);

      closeSubpath();
    }
  };
  
public:
  Bubble(bool topTip, const QSize &size, qreal radius, qreal tipOffset) : 
    QPixmap(size) {
    // fill with transparency
    fill(Qt::transparent);

    // leave one pixel border to avoid antialiasing issues
    QRect rectangle(1, 1, width()-2, height()-2);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // black outline and fill with slightly transparent white
    painter.setBrush( QColor(255, 255, 255, 224) );
    painter.setPen(QPen(QBrush(Qt::black), 2) );

    BubblePath bubblePath(topTip, rectangle, radius, tipOffset);
    painter.drawPath(bubblePath);
  };
};

class BubbleContent {
private:
  QFont getFont(int size = Config::MAP_BUBBLE_BASE_SIZE, int weight = 0) {
    QFont font("Arial");
    font.setPixelSize(size);
    font.setWeight(weight);
    return font;
  }

public: 
  void checkRating(const QString &msg, bool right = false) {
    QFontMetrics metrics(getFont());
    if(!(m_landscape && right)) {
      if(metrics.lineSpacing() > m_iconLoader->size())
	m_size.setHeight(m_size.height() + metrics.lineSpacing());
      else
	m_size.setHeight(m_size.height() + m_iconLoader->size());
    }

    int width = metrics.width(msg) + 5*m_iconLoader->size();
    if(width > m_size.width()) m_size.setWidth(width);
    if(width > m_colWidth[(m_landscape && right)?1:0]) 
      m_colWidth[(m_landscape && right)?1:0] = width;
  }

  void checkText(QPixmap *pix, const QString &msg, 
		 int fontSize = Config::MAP_BUBBLE_BASE_SIZE, 
		 int flags = 0) {
    QFontMetrics metrics(getFont(fontSize, flags));

    QRect rect = metrics.boundingRect(msg);

    int height = (pix && (pix->height() > rect.height()))?
      pix->height():rect.height();
    
    m_size += QSize(0, height);

    int width = metrics.width(msg) + (pix?pix->width()+fontSize:0);
    if(width > m_size.width()) m_size.setWidth(width);
  }

  void checkText(const QString &msg, 
		 int fontSize = Config::MAP_BUBBLE_BASE_SIZE, 
		 int flags = 0) {
    checkText(NULL, msg, fontSize, flags);
  }

  void recheckText(QPixmap *pix, const QString &msg, 
		   int fontSize = Config::MAP_BUBBLE_BASE_SIZE, int flags = 0) {
    QFontMetrics metrics(getFont(fontSize, flags));
    QRect rect = metrics.boundingRect(msg);
    int height = (pix && (pix->height() > rect.height()))?
      pix->height():rect.height();

    // now that we know the width, text may be broken down in several rows
    // and we have to take the vertical space required by this into account
    int xOffset = pix?pix->width()+fontSize:0;

    rect = metrics.boundingRect(QRect(m_pos + QPoint(xOffset,0), 
				      m_size - QSize(xOffset, 0)),
				      Qt::TextWordWrap, msg);

    // check if we need additional space
    if(rect.height() > height) 
      m_size += QSize(0, rect.height()-height);
  }

  void recheckText(const QString &msg, 
		   int fontSize = Config::MAP_BUBBLE_BASE_SIZE, 
		   int flags = 0) {
    recheckText(NULL, msg, fontSize, flags);
  }

  void drawText(QPainter &painter, QPixmap *pix, const QString &msg, 
		int fontSize = Config::MAP_BUBBLE_BASE_SIZE, 
		int flags = 0, bool swap = false) {
    painter.setFont(getFont(fontSize, flags));

    // calculate size and offset of text box
    QPoint textOffset((pix&&!swap)?pix->width()+fontSize:0, 0);
    QSize textSize(m_size - QSize(pix?pix->width():0, 0));
    
    QRect rect;
    painter.drawText(QRect(m_pos + textOffset, textSize),
		     Qt::TextWordWrap, msg, &rect);

    if(pix) {
      // move pix to the right if requested and always center vertically
      QPoint pixOffset(QPoint(swap?rect.width()+fontSize:0,
			      (rect.height()-pix->height())/2));

      painter.drawPixmap(m_pos + pixOffset, *pix);
    }
    
    // advance by text or pixmap height, whatever is bigger
    int height = (pix && (pix->height() > rect.height()))?
      pix->height():rect.height();

    m_pos += QPoint(0, height);
  }

  void drawText(QPainter &painter, const QString &msg, QPixmap *pix, 
		int fontSize = Config::MAP_BUBBLE_BASE_SIZE, int flags = 0) {
    drawText(painter, pix, msg, fontSize, flags, true); 
  }

  void drawText(QPainter &painter, const QString &msg, 
		int fontSize = Config::MAP_BUBBLE_BASE_SIZE, int flags = 0) {
    drawText(painter, NULL, msg, fontSize, flags);
  }

  // render five stars into one bitmap
  QPixmap renderRating(qreal rating) {
    QPixmap pixmap(5*m_iconLoader->size(), m_iconLoader->size());
    pixmap.fill(Qt::transparent);

    // create single star bitmaps if required
    if(!m_pixStar)      m_pixStar = m_iconLoader->load("star");
    if(!m_pixStarLight) {
      m_pixStarLight = m_iconLoader->load("star_light");

      // TODO: Generate greyscale star on demand from colored one
      // m_pixStarLight = new QPixmap(*m_pixStar);
    }
    if(!m_pixStarHalf) {
      m_pixStarHalf = new QPixmap(*m_pixStar);
      QPainter halfPainter(m_pixStarHalf);
      const QRectF half(m_iconLoader->size()/2, 0, 
		  m_iconLoader->size()/2, m_iconLoader->size());
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

      painter.drawPixmap(i*m_iconLoader->size(), 0, *star);
    }

    return pixmap;
  }

  void drawRating(QPainter &painter, const QString &msg, 
		  qreal rating, bool right = false) {
    int xOffset = (m_landscape && right)?m_colWidth[0]+
      Config::MAP_BUBBLE_BASE_SIZE:0;
    painter.setFont(getFont());

    int yOffset = (painter.fontMetrics().lineSpacing() > m_iconLoader->size())?
      painter.fontMetrics().lineSpacing():m_iconLoader->size();

    // move back up if the right column is to be drawn
    if(m_landscape && right) m_pos -= QPoint(0, yOffset);

    painter.drawText(m_pos + QPoint(xOffset, 
		    painter.fontMetrics().ascent()), msg);

    QPixmap pixmap = renderRating(rating);

    // right align bitmaps per column
    int pOffset = xOffset + m_colWidth[(m_landscape && right)?1:0] - 
      pixmap.width();

    painter.drawPixmap(m_pos + QPoint(pOffset, 0), pixmap);    

    m_pos += QPoint(0, yOffset);
  }

  // first row:  name + type icon
  // second row: description
  // third row:  date|owner
  // fourth row: difficulty|terrain
  // fifth row:  container

  BubbleContent(const Cache *cache, bool landscape, const QSizeF &mapSize):
    m_pixStar(NULL), m_pixStarHalf(NULL), m_pixStarLight(NULL), m_fwd(NULL),
    m_landscape(landscape) {

    qDebug() << __FUNCTION__;

    this->m_cache = cache;
    this->m_wpt = NULL;

    this->m_difficultyStr = QString(QObject::tr("Difficulty") + ": ");
    this->m_terrainStr = QString(QObject::tr("Terrain") + ": ");
    if(!cache->owner().isEmpty()) {
      if(cache->dateOfPlacement().isValid())
	this->m_placedByStr = QString(QObject::tr("Placed on") + " " + 
			      cache->dateOfPlacementString() + " " +
			      QObject::tr("by") + " " + cache->owner());
      else
	this->m_placedByStr = QString(QObject::tr("Placed by") + 
				      " " + cache->owner());
    } else if(cache->dateOfPlacement().isValid())
      this->m_placedByStr = cache->dateOfPlacementString();
	      
    this->m_containerStr = QString(QObject::tr("Container") + ":");

    // iconloader to load the icons used inside the bubble, default size
    // is 16 (for the stars)
    this->m_iconLoader = new IconLoader(Config::MAP_BUBBLE_BASE_SIZE);
    
    // reset size calculations
    m_size = QSize(0,0);
    m_pos = QPoint(0,0);
    m_colWidth[0] = m_colWidth[1] = 0;

    checkText(m_iconLoader->load(*cache, 3*Config::MAP_BUBBLE_BASE_SIZE/2), 
	      cache->name(), Config::MAP_BUBBLE_BIG_SIZE, QFont::Bold);
    checkText(cache->description(), Config::MAP_BUBBLE_BASE_SIZE, QFont::Bold);

    if(!m_placedByStr.isEmpty())
      checkText(m_placedByStr);

    if(cache->difficulty().isSet()) {
      checkRating(m_difficultyStr);
      if(cache->terrain().isSet()) 
	checkRating(m_terrainStr, true);
    } else if(cache->terrain().isSet()) 
      checkRating(m_terrainStr);

    if(cache->container().isSet())
      checkText(m_iconLoader->load(cache->container()), m_containerStr);

    // try to limit bubble size to 70% of map width
    if(m_size.width()  > 0.7*mapSize.width()) 
      m_size.setWidth(0.7*mapSize.width());

    // but check that bubble is wide enough to accomodate two columens
    if(m_size.width() < 
       m_colWidth[0] + (m_colWidth[1]?Config::MAP_BUBBLE_BASE_SIZE:0) + 
       m_colWidth[1])
      m_size.setWidth(m_colWidth[0] + 
		      (m_colWidth[1]?Config::MAP_BUBBLE_BASE_SIZE:0) + 
		      m_colWidth[1]);

    // check if the adjusted width causes texts to wrap and take the
    // additional lines into account
    recheckText(m_iconLoader->load(*cache, Config::MAP_BUBBLE_BIG_SIZE), 
		cache->name(), Config::MAP_BUBBLE_BIG_SIZE, QFont::Bold);
    recheckText(cache->description(), Config::MAP_BUBBLE_BASE_SIZE, QFont::Bold);
    if(!m_placedByStr.isEmpty()) recheckText(m_placedByStr);
    if(cache->container().isSet())
      recheckText(m_iconLoader->load(cache->container()), m_containerStr);
  }

  BubbleContent(const Waypoint *wpt, bool landscape, const QSizeF &mapSize):
    m_pixStar(NULL), m_pixStarHalf(NULL), m_pixStarLight(NULL), m_fwd(NULL),
    m_landscape(landscape) {

    qDebug() << __FUNCTION__;
    this->m_wpt = wpt;
    this->m_cache = NULL;

    this->m_iconLoader = new IconLoader(Config::MAP_BUBBLE_BASE_SIZE);

    m_size = QSize(0,0);
    m_pos = QPoint(0,0);
    m_colWidth[0] = m_colWidth[1] = 0;

    checkText(m_iconLoader->load(wpt->iconFile(), 
				 3*Config::MAP_BUBBLE_BASE_SIZE/2), 
	      wpt->name(), Config::MAP_BUBBLE_BIG_SIZE, QFont::Bold);
    if(!wpt->description().isEmpty())
      checkText(wpt->description(), Config::MAP_BUBBLE_BASE_SIZE, QFont::Bold);
    if(!wpt->comment().isEmpty())
      checkText(wpt->comment());

    // try to limit bubble size to 70% of map width
    if(m_size.width()  > 0.7*mapSize.width()) 
      m_size.setWidth(0.7*mapSize.width());

    recheckText(m_iconLoader->load(wpt->iconFile(), 
				 3*Config::MAP_BUBBLE_BASE_SIZE/2), 
	      wpt->name(), Config::MAP_BUBBLE_BIG_SIZE, QFont::Bold);
    if(!wpt->description().isEmpty())
      recheckText(wpt->description(), Config::MAP_BUBBLE_BASE_SIZE, 
		  QFont::Bold);
    if(!wpt->comment().isEmpty())
      recheckText(wpt->comment());
  }

  ~BubbleContent() {
    // the other pixmaps (Star and StarLight) are owned by the
    // imageLoader and deleted by that
    if(m_pixStarHalf)  delete m_pixStarHalf;

    delete m_iconLoader;
  }

  QSize size() const {
    return m_size;
  }

  void paint(const QRect &contentArea, QPixmap *pix) {
    QPainter painter(pix);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // limit painter to contentArea
    painter.setClipRect(contentArea);
    painter.translate(contentArea.topLeft() );

    m_pos = QPoint(0,0);  // reset painter position

    if(m_cache) {
      /* render cache bubble */

      painter.setPen( QPen(Qt::darkRed));
      drawText(painter, m_iconLoader->load(*m_cache), 
	       m_cache->name(), Config::MAP_BUBBLE_BIG_SIZE, QFont::Bold);
      painter.setPen( QPen(Qt::black));
      drawText(painter, m_cache->description(), Config::MAP_BUBBLE_BASE_SIZE, 
	       QFont::Bold);

      if(!m_placedByStr.isEmpty())
	drawText(painter, m_placedByStr);

      if(m_cache->difficulty().isSet()) {
	drawRating(painter, m_difficultyStr, m_cache->difficulty().value());
	if(m_cache->terrain().isSet()) 
	  drawRating(painter, m_terrainStr, m_cache->terrain().value(), true);
      } else if(m_cache->terrain().isSet()) 
	drawRating(painter, m_terrainStr, m_cache->terrain().value());
    
      if(m_cache->container().isSet())
	drawText(painter, m_containerStr, 
		 m_iconLoader->load(m_cache->container()));
    } else {
      qDebug() << __FUNCTION__ << contentArea;

      /* render waypoint bubble */
      painter.setPen( QPen(Qt::darkRed));
      drawText(painter, m_iconLoader->load(m_wpt->iconFile()), 
	       m_wpt->name(), Config::MAP_BUBBLE_BIG_SIZE, QFont::Bold);

      painter.setPen( QPen(Qt::black));
      if(!m_wpt->description().isEmpty())
	drawText(painter, m_wpt->description(), Config::MAP_BUBBLE_BASE_SIZE, 
		 QFont::Bold);

      if(!m_wpt->comment().isEmpty())
	drawText(painter, m_wpt->comment());
    }

    // paint the forward button over everything
    if(!m_fwd)
      m_fwd = m_iconLoader->load("map_overlay_forward", 2 * Config::MAP_BUBBLE_BASE_SIZE);

    painter.drawPixmap(QPoint(contentArea.width()-m_fwd->width(),0), *m_fwd);
  }

private:
  QString m_difficultyStr, m_terrainStr, m_placedByStr, m_containerStr;

  QSize m_size;
  QPoint m_pos;
  const Cache *m_cache;
  const Waypoint *m_wpt;
  IconLoader *m_iconLoader;
  QPixmap *m_pixStar, *m_pixStarHalf, *m_pixStarLight;
  CustomPixmap *m_fwd;
  bool m_landscape;
  int m_colWidth[2];
};

// horizontally we need to know whether we have to move the bubble off-center
qreal MapBubble::calculateTipOffset(const QGraphicsGeoMap *map, 
				    const QGeoCoordinate &coo,
				    const QSize &bsize) {
  qreal hstep = map->coordinateToScreenPosition(coo).x() - 
    map->size().width()/2;

  qreal tipOffset = hstep;
  if(tipOffset > (bsize.width()-3*Config::MAP_BUBBLE_RADIUS)/2) 
    tipOffset = (bsize.width()-3*Config::MAP_BUBBLE_RADIUS)/2;
  if(tipOffset < (3*Config::MAP_BUBBLE_RADIUS-bsize.width())/2) 
    tipOffset = (3*Config::MAP_BUBBLE_RADIUS-bsize.width())/2;

  return tipOffset;
} 

MapBubble::MapBubble(const QGraphicsGeoMap *map, bool landscape, 
        const Waypoint &wpt) : QGeoMapPixmapObject(wpt.coordinate()) {
  qDebug() << __FUNCTION__ << wpt.coordinate() << wpt.name();

  // create local copy of waypoint
  this->m_wpt = wpt;
  this->m_cache.setCoordinate(QGeoCoordinate());

  // vertically we only need to know whether the bubble should display
  // above or below the icon
  bool topTip = map->coordinateToScreenPosition(wpt.coordinate()).y()
    < map->size().height()/2;

  // create bubblecontent to be able to determine the necessary bubble size
  BubbleContent bubbleContent(&wpt, landscape, map->size());

  // calculate total bubble size
  QSize bsize(bubbleContent.size() + 
	      QSize(Config::MAP_BUBBLE_RADIUS+2,2*Config::MAP_BUBBLE_RADIUS+2));

  int tipOffset = calculateTipOffset(map, wpt.coordinate(), bsize);

  // create empty bubble bitmap
  Bubble bubble(topTip, bsize, Config::MAP_BUBBLE_RADIUS, tipOffset);

  // fill with content
  // and fill content area
  bubbleContent.paint(QRect(QPoint(Config::MAP_BUBBLE_RADIUS/2, 
	   Config::MAP_BUBBLE_RADIUS/2 + (topTip?Config::MAP_BUBBLE_RADIUS:0)),
	    bubbleContent.size()), &bubble);

  setPixmap(bubble);

  // move bubble relative to map to make bubble tip point to target coordinate
  setOffset(QPoint(-bsize.width()/2 - tipOffset, topTip?0:-bsize.height()));

  setObjectName("bubble");
  setZValue(4);  // above gps marker, caches and waypoints
  setProperty("name", wpt.name());
}

MapBubble::MapBubble(const QGraphicsGeoMap *map, bool landscape, 
        const Cache &cache) : QGeoMapPixmapObject(cache.coordinate()) {
  qDebug() << __FUNCTION__ << cache.coordinate() << cache.name();

  // create local copy of cache
  this->m_cache = cache;
  this->m_wpt.setCoordinate(QGeoCoordinate());

  // vertically we only need to know whether the bubble should display
  // above or below the icon
  bool topTip = map->coordinateToScreenPosition(cache.coordinate()).y()
    < map->size().height()/2;

  // create bubblecontent to be able to determine the necessary bubble size
  BubbleContent bubbleContent(&cache, landscape, map->size());

  // calculate total bubble size
  QSize bsize(bubbleContent.size() + 
	      QSize(Config::MAP_BUBBLE_RADIUS+2,2*Config::MAP_BUBBLE_RADIUS+2));

  int tipOffset = calculateTipOffset(map, cache.coordinate(), bsize);

  // create empty bubble bitmap
  Bubble bubble(topTip, bsize, Config::MAP_BUBBLE_RADIUS, tipOffset);

  // and fill content area
  bubbleContent.paint(QRect(QPoint(Config::MAP_BUBBLE_RADIUS/2, 
	   Config::MAP_BUBBLE_RADIUS/2 + (topTip?Config::MAP_BUBBLE_RADIUS:0)),
	    bubbleContent.size()), &bubble);

  setPixmap(bubble);

  // move bubble relative to map to make bubble tip point to target coordinate
  setOffset(QPoint(-bsize.width()/2 - tipOffset, topTip?0:-bsize.height()));

  setObjectName("bubble");
  setZValue(4);  // above gps marker, caches and waypoints
  setProperty("name", cache.name());
}

Cache MapBubble::cache() const {
  return m_cache;
} 

Waypoint MapBubble::wpt() const {
  return m_wpt;
} 
