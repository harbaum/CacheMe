#ifndef ICON_LOADER_H
#define ICON_LOADER_H

#include <QList>
#include <QPixmap>
#include <QString>
#include <QStringList>
#include <QSvgRenderer>

#include "cache.h"
#include "custompixmap.h"

class IconLoader {
public:
  IconLoader(int);
  ~IconLoader();
  CustomPixmap *load(const QString &, int = 0, qreal = 0.0);
  CustomPixmap *loadHighlight(const QString &, int = 0);
  CustomPixmap *load(const QStringList &, int = 0, qreal = 0.0);
  CustomPixmap *loadHighlight(const QStringList &, int = 0);
  CustomPixmap *load(const Cache &, int = 0);
  CustomPixmap *loadHighlight(const Cache &, int = 0);
  CustomPixmap *load(const Container &, int = 0);
  int size() const;
  QWidget *newWidget(const QString &, int = 0);
  QWidget *newWidget(const QStringList &, int = 0);
  QWidget *newWidget(const Cache &, int = 0);

private:
  QSvgRenderer *m_renderer;
  int m_size;
  
  QList <CustomPixmap*> m_pixmapList;
};

#endif // ICON_LOADER_H
