#ifndef MAP_BUTTON_H
#define MAP_BUTTON_H

#include <QPixmap>
#include <QPointF>
#include <QPainter>

#include "iconloader.h"

class MapButton : public QObject {
  Q_OBJECT

 public:
  enum State { Normal, Selected, Disabled, Hidden };
  MapButton(IconLoader *, const QString &, int, int, State);
  ~MapButton();
  void paint(const QSizeF &, QPainter *);
  QString name() const;
  bool setState(const State &);
  void change(IconLoader *, const QString &);
  void releaseImages();
  void reload(IconLoader *);

  bool isInside(const QSizeF &, const QPointF &) const;
  bool mousePress(const QSizeF &, const QPointF &);
  bool mouseMove(const QSizeF &, const QPointF &);
  bool mouseRelease(const QSizeF &, const QPointF &);

 signals:
  void clicked();

 private:
  QPoint screenPos(const QSizeF &) const;
  void renderNormal();
  void renderDisabled();
  void renderSelected();

  State m_state;
  QPixmap *m_pixmap;
  QPixmap *m_pixNormal, *m_pixDisabled, *m_pixSelected;
  QPoint m_p;
  int m_size;
  QString m_name;

  bool m_downOn;

  static const int SHADOW_OFFSET = 8;
};

#endif // MAP_BUTTON_H
