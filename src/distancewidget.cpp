/*
 *  This file is part of CacheMe
 *
 *  (c) 2011 by Till Harbaum <till@harbaum.org>
 */

#include <QHBoxLayout>
#include <QLocale>

#include "distancewidget.h"

void DistanceWidget::selectUnit(const QString &str) {
  const Unit units[] = { Meter, Kilometer, Foot, Yard, Mile, Unknown };
  int i;

  // search for unit
  for(i=0;(units[i] != Unknown) && (unitName(units[i]) != str);i++); 
  
  m_unit = units[i];
  updateLineEdit();
}

void DistanceWidget::setValue(qreal value) {
  m_value = value;
  updateLineEdit();
}

void DistanceWidget::setUnit(Unit unit) {
  m_unit = unit;
  m_cBox->setCurrentIndex((int)m_unit);
  updateLineEdit();
}

qreal DistanceWidget::value() {
  return m_value;
}

DistanceWidget::Unit DistanceWidget::unit() {
  return m_unit;
}

void DistanceWidget::textEdited(const QString &text) {

  // update value accordingly
  bool ok;
  qreal value = text.toFloat(&ok);   
  if(ok) {
    m_value = value / unitFactor();
    emit changed(m_value);
  }
}

QString DistanceWidget::unitName(const Unit unit) {
  Unit lunit = (unit != Unknown)?unit:m_unit;
  int i;

  const struct { Unit unit; QString str; } units[] = {
    { Meter,         tr("m") },
    { Kilometer,     tr("km") },
    { Foot,          tr("ft") },
    { Yard,          tr("yd") },
    { Mile,          tr("mi") },
    { Unknown,       "--" }
  };

  for(i=0;(units[i].unit != Unknown) && (units[i].unit != lunit);i++); 

  return units[i].str;
}

// returns the factor the value in meter needs to be multiplied with
// to be displayed in this unit
qreal DistanceWidget::unitFactor(const Unit unit) {
  Unit lunit = (unit != Unknown)?unit:m_unit;
  int i;

  const struct { Unit unit; qreal factor; } units[] = {
    { Meter,         1.0f },
    { Kilometer,     0.001f },
    { Foot,          3.28084f },
    { Yard,          1.0936133f },
    { Mile,          0.000621371192f },
    { Unknown,       0.0f }
  };

  for(i=0;(units[i].unit != Unknown) && (units[i].unit != lunit);i++); 

  return units[i].factor;
}

// update line edit from stored value
void DistanceWidget::updateLineEdit() {
  qreal valueUnit = m_value * unitFactor();

  m_lineEdit->setText(QString::number(valueUnit, 'f', 3));
}

DistanceWidget::DistanceWidget(QWidget *parent): QWidget(parent), m_value(0) {

  // set a sane default unit
  QLocale locale;
  if(locale.measurementSystem() == QLocale::ImperialSystem) m_unit = Yard;
  else                                                      m_unit = Meter;

  // the distance widget is a hbox with a text entry and a unit selector
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(0);

  m_lineEdit = new QLineEdit(this);
  connect(m_lineEdit, SIGNAL(textEdited(const QString&)), 
	  this, SLOT(textEdited(const QString &)));
  layout->addWidget(m_lineEdit);

  updateLineEdit();
 
  m_cBox = new QComboBox(this);
  connect(m_cBox, SIGNAL(activated(const QString &)), 
  	  this, SLOT(selectUnit(const QString &)));

  for(Unit u=Meter;u!=Unknown;u=(Unit)(u+1) ) {
    m_cBox->addItem(unitName(u));
    if(u == m_unit)
      m_cBox->setCurrentIndex(m_cBox->count()-1);
  }
  
  layout->addWidget(m_cBox);

  setLayout(layout);
}
