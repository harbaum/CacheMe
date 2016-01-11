#include "iconloader.h"
#include "filefinder.h"

#include <QDebug>
#include <QPainter>
#include <QLabel>
#include <QImage>

IconLoader::IconLoader(int size) {
  m_renderer = new QSvgRenderer();
  this->m_size = size;
}

IconLoader::~IconLoader() {
  qDeleteAll(m_pixmapList);
  delete m_renderer;
}

CustomPixmap *IconLoader::load(const QStringList &name, int size, qreal angle) {
  QString joinedName = name.join(",");

  // check if there's already a matching pixmap in the list
  QList<CustomPixmap*>::const_iterator i;
  for(i = m_pixmapList.constBegin(); i != m_pixmapList.constEnd(); ++i) 
    if( (*i)->is(joinedName, CustomPixmap::Normal, angle) )
      return *i;
  
  // nothing matching in pixmap list, create a new entry
  CustomPixmap *pix = new CustomPixmap(m_renderer, name, size?size:m_size, angle); 
  m_pixmapList.append(pix);
  
  return pix;
}

// todo: cache resulting pixmap!
CustomPixmap *IconLoader::loadHighlight(const QStringList &name, int size) {
  QString joinedName = name.join(",");

  // check if there's already a matching pixmap in the list
  QList<CustomPixmap*>::const_iterator i;
  for(i = m_pixmapList.constBegin(); i != m_pixmapList.constEnd(); ++i) 
    if( (*i)->is(joinedName, CustomPixmap::Highlight) )
      return *i;

  // load non-highlighted icon
  const int BORDER = 8;
  CustomPixmap *pix = load(name, size);

  // create temporary pixmap with extra space for border ...
  QPixmap tmpPix(pix->width() + 2*BORDER, pix->height() + 2*BORDER);

  // ... and fill with transparency ...
  tmpPix.fill(Qt::transparent);

  // ... and draw icon into the center
  QPainter tmpPainter(&tmpPix);
  tmpPainter.drawPixmap(QPointF(BORDER, BORDER), *pix);

  // convert source to qimage to be able to access individual pixels
  QImage srcImage = tmpPix.toImage();

  // and create a destination image to paint on
  QImage image(srcImage.size(), QImage::Format_ARGB32);
  image.fill(Qt::transparent);
  
  // create hihglight shadow in "image"
  int w = image.width(), h = image.height();
  for(int y = 0; y < h; y++) {
    for( int x = 0; x < w; x++) {
      QRgb pix = srcImage.pixel(x, y);

      // target pixel is at least partially transparent and will 
      // let the highlight shine through
      if(qAlpha(pix) < 0xff) {
	int sum = 0;

	int sx = x-BORDER;   if(sx<0)   sx = 0;
	int ex = x+BORDER+1; if(ex>w-1) ex = w-1;
	int sy = y-BORDER;   if(sy<0)   sy = 0;
	int ey = y+BORDER+1; if(ey>w-1) ey = h-1;

	for(int ay=sy;ay<ey;ay++)
	  for(int ax=sx;ax<ex;ax++)
	    sum += qAlpha(srcImage.pixel(ax, ay));

	sum /= ((2*BORDER+1)*(2*BORDER+1))/4;

	if(sum > 255) sum = 255;
	QColor color(0xff,0xc0,0,sum);
	image.setPixel(x, y, color.rgba());
      }
    }
  }

  // finally combine everything into a new pixmap
  CustomPixmap *hlPix = 
    new CustomPixmap(name, tmpPix.width(), CustomPixmap::Highlight);

  QPainter hlPainter(hlPix);
  // first the highlight ...
  hlPainter.drawImage(QPointF(0,0), image);
  // ... then the icon itself on top
  hlPainter.drawPixmap(QPointF(BORDER,BORDER), *pix);

  // finally add pixmap to cache
  m_pixmapList.append(hlPix);

  return hlPix;
}

CustomPixmap *IconLoader::loadHighlight(const Cache &cache, int size) {
  QStringList names;
  names << cache.typeIconFile();
  QString overlay = cache.overlayIconFile();
  if(!overlay.isNull())
    names << overlay;
  
  return loadHighlight(names, size);
}

CustomPixmap *IconLoader::load(const QString &name, int size, qreal angle) {
  return load(QStringList(name), size, angle);
}

CustomPixmap *IconLoader::loadHighlight(const QString &name, int size) {
  return loadHighlight(QStringList(name), size);
}

CustomPixmap *IconLoader::load(const Cache &cache, int size) {
  QStringList names;
  names << cache.typeIconFile();
  QString overlay = cache.overlayIconFile();
  if(!overlay.isNull())
    names << overlay;
  
  return load(names, size);
}

CustomPixmap *IconLoader::load(const Container &container, int size) {
  return load(container.iconFile(), size);
}

int IconLoader::size() const {
  return m_size;
}

QWidget *IconLoader::newWidget(const QString &name, int size) {
  QLabel *widget = new QLabel;
  widget->setPixmap(*load(name, size));
  return widget;
}

QWidget *IconLoader::newWidget(const QStringList &names, int size) {
  QLabel *widget = new QLabel;
  widget->setPixmap(*load(names, size));
  return widget;
}

QWidget *IconLoader::newWidget(const Cache &cache, int size) {
  QLabel *widget = new QLabel;
  widget->setPixmap(*load(cache, size));
  return widget;
}
