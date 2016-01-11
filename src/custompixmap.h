#ifndef CUSTOM_PIXMAP_H
#define CUSTOM_PIXMAP_H

#include <QPixmap>
#include <QSvgRenderer>
#include <QStringList>

class CustomPixmap: public QPixmap {
 public:
  enum State { Normal, Highlight, Greyscale };
  
  CustomPixmap(QSvgRenderer *, const QStringList &, int = 0, qreal = 0.0);
  CustomPixmap(const QStringList &, int = 0, State = Normal, qreal = 0.0);
  ~CustomPixmap();
  bool is(const QString &, State, qreal = 0.0);
  QPoint offset();
  static QSize prepare(QSvgRenderer *, const QString &, int);

 private:
  static bool load(QSvgRenderer *, const QString &);

  QString m_name;
  State m_state;
  QPoint m_offset;
  qreal m_angle;
};

#endif // CUSTOM_PIXMAP_H
