#include <QDebug>
#include <QLabel>
#include <math.h>

#include "coordinatetool.h"
#include "pickermanager.h"

// Allowed range:
// Latitude:   N/S  89° 59.999'  
// Longitude:  E/W 179° 59.999'

class CLabel : public QLabel {
public:
  CLabel(const QString &label, QWidget *parent = 0) : 
    QLabel(label, parent) {
    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  }
};

PickerManager::PickerManager(const QGeoCoordinate &coo, Mode mode, QWidget *parent) : 
  QWidget(parent), m_mode(mode), m_coo(coo) {
  qDebug() << __FUNCTION__;
}

void PickerManager::evaluate() {
  int id = 0;
  int sign = m_widget[id++]->value();
  int degrees = 0, minInt = 0, minFrac = 0;

  for(int i=0;i<((m_mode == Longitude)?3:2);i++) 
    degrees = 10*degrees + m_widget[id++]->value();

  for(int i=0;i<2;i++) 
    minInt = 10*minInt + m_widget[id++]->value();

  for(int i=0;i<3;i++) 
    minFrac = 10*minFrac + m_widget[id++]->value();

  if(m_mode == Longitude)
    m_coo.setLongitude(sign * (degrees + minInt/60.0 + minFrac/60000.0));
  else
    m_coo.setLatitude(sign * (degrees + minInt/60.0 + minFrac/60000.0));
}

void PickerManager::evaluateAll() {
  foreach(PickerWidget *widget, m_widget)
    widget->evaluate();
}

// return the current max value allowed for widget "id" taking the
// state of all other widgets into accound
int PickerManager::upperBound(int id) {
  Q_ASSERT(id > 0);

  if(m_mode == Latitude) {
    // 0 12° 34.567'
    if(id == 1) return 8;
    if(id == 3) return 5;
  } else {
    // 0 123° 45.678'
    if(id == 1) {
      // max 179 deg
      if(m_widget[2]->value() > 7) return 0;
      else	                   return 1;
    } else if(id == 2) {
      // max 179 deg
      if(m_widget[1]->value() == 1)
	return 7;
    }

    if(id == 4) return 5;
  }
  
  return 9;   // all other digits can run to 9
}

// check whether number widget with id can decrease
bool PickerManager::canDecrease(int id) {
  // a number widget can decrease unless previous numbers widgets are 0
  if(id == 1) return false;

  bool allZero = true;
  for(int i=1;i<id;i++)
    if(m_widget[i]->value() != 0)
      allZero = false;

  return !allZero;
}

// check whether number widget with id can increase
bool PickerManager::canIncrease(int id) {
  // a number widget can decrease unless previous numbers widgets are 
  // max'ed
  if(id == 1) return false;

  bool allMax = true;
  for(int i=1;i<id;i++)
    if(m_widget[i]->value() != upperBound(i))
      allMax = false;

  return !allMax;
}

void PickerManager::decrease(int id) {
  if(id > 0) 
    m_widget[id]->decrease();
}

void PickerManager::increase(int id) {
  if(id > 0) 
    m_widget[id]->increase();
}

void PickerManager::createWidget(QBoxLayout *layout) {
  PickerWidget *widget = new PickerWidget();
  layout->addWidget(widget);

  widget->setManager(this, m_widget.size());
  
  m_widget.append(widget);
}

void PickerManager::addWidgets(bool first, QBoxLayout *layout) {
  Q_ASSERT(m_coo.isValid());

  if(first) {
    if(m_mode == Latitude) 
      createWidget(layout);
    else {
      createWidget(layout);
      createWidget(layout);
    }

    createWidget(layout);
    createWidget(layout);
    layout->addWidget(new CLabel("\260"));
  } else {
    createWidget(layout);
    createWidget(layout);
    layout->addWidget(new CLabel("."));
    createWidget(layout);
    createWidget(layout);
    createWidget(layout);
    layout->addWidget(new CLabel("'"));
  }
}

void PickerManager::updateWidgets() {
  qreal value = (m_mode == Latitude)?m_coo.latitude():m_coo.longitude();

  /* convert value into string */
  qreal fractional;
  double integral;
  bool pos = true;

  if(value < 0) { value = fabs(value); pos = false; }

  fractional = modf(value, &integral);

  // convert into strings
  QString intStr(CoordinateTool::zeroCut(integral, 0, 3));
  QString fracStr(CoordinateTool::zeroCut(60*fractional, 3, 3));

  int id = 0;
  if(m_mode == Latitude)
    m_widget[id++]->set(pos?'N':'S');
  else {
    m_widget[id++]->set(pos?'E':'W');
    m_widget[id++]->set(intStr[0]);
  }

  m_widget[id++]->set(intStr[1]);
  m_widget[id++]->set(intStr[2]);

  m_widget[id++]->set(fracStr[0]);
  m_widget[id++]->set(fracStr[1]);

  m_widget[id++]->set(fracStr[3]);
  m_widget[id++]->set(fracStr[4]);
  m_widget[id++]->set(fracStr[5]);

  evaluateAll();
}

void PickerManager::setCoordinate(const QGeoCoordinate &coo) {
  m_coo = coo;
}

QGeoCoordinate PickerManager::coordinate() {
  return m_coo;
}
