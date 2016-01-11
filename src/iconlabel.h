#include <QWidget>
#include <QString>
#include <QLabel>
#include "iconloader.h"

class IconLabel : public QWidget {
  Q_OBJECT;
 
 public:
  IconLabel(IconLoader *, const QStringList &,
	    const QString &, qreal=1.0, QWidget * = 0); 
  IconLabel(IconLoader *, const QString &,
	    const QString &, qreal=1.0, QWidget * = 0); 
  IconLabel(IconLoader *, const Cache &,
	    const QString &, qreal=1.0, bool=false, QWidget * = 0); 
  void resizeEvent(QResizeEvent *);

 private:
  int prepare(const QString &, qreal, bool);
  QString m_text;
  bool m_elided;
  QLabel *m_label;
};
