/*
 *  This file is part of CacheMe
 *
 *  (c) 2011 by Till Harbaum <till@harbaum.org>
 */

#ifndef DISTANCE_WIDGET_H
#define DISTANCE_WIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>

class DistanceWidget: public QWidget {
  Q_OBJECT;

 public:
  enum Unit { Meter, Kilometer, Foot, Yard, Mile, Unknown };

  DistanceWidget(QWidget * = 0);
  void setValue(qreal);
  void setUnit(Unit);
  qreal value();
  Unit unit();

 private slots:
  void selectUnit(const QString &);
  void textEdited(const QString &);

 signals:
  void changed(qreal);

 private:
  void updateLineEdit();
  QString unitName(const Unit = Unknown);
  qreal unitFactor(const Unit = Unknown);

  QComboBox *m_cBox;
  QLineEdit *m_lineEdit;

  Unit m_unit;
  qreal m_value;
};

#endif // DISTANCE_WIDGET_H
