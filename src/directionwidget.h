/*
 *  This file is part of CacheMe
 *
 *  (c) 2011 by Till Harbaum <till@harbaum.org>
 */

#ifndef DIRECTION_WIDGET_H
#define DIRECTION_WIDGET_H

#include <QWidget>
#include <QLineEdit>

class DirectionWidget: public QWidget {
  Q_OBJECT;

 public:
  DirectionWidget(QWidget * = 0);
  void setValue(qreal);
  qreal value();

 private slots:
  void textEdited(const QString &);

 signals:
  void changed(qreal);

 private:
  void updateLineEdit();
  QLineEdit *m_lineEdit;
  qreal m_value;
};

#endif // DIRECTION_WIDGET_H
