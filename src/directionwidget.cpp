/*
 *  This file is part of CacheMe
 *
 *  (c) 2011 by Till Harbaum <till@harbaum.org>
 */

#include <QHBoxLayout>
#include <QLabel>

#include "directionwidget.h"

void DirectionWidget::setValue(qreal value) {
  while(value < 0.0) value += 360.0;
  while(value > 360.0) value -= 360.0;

  m_value = value;
  updateLineEdit();
}

qreal DirectionWidget::value() {
  return m_value;
}

void DirectionWidget::textEdited(const QString &text) {

  // update value accordingly
  bool ok;
  qreal value = text.toFloat(&ok);   
  if(ok) {
    m_value = value;
    emit changed(m_value);
  }
}

// update line edit from stored value
void DirectionWidget::updateLineEdit() {
  m_lineEdit->setText(QString::number(m_value, 'f', 1));
}

DirectionWidget::DirectionWidget(QWidget *parent): 
  QWidget(parent), m_value(0) {

  // the distance widget is a hbox with a text entry and a unit selector
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(0);

  m_lineEdit = new QLineEdit(this);
  connect(m_lineEdit, SIGNAL(textEdited(const QString&)), 
	  this, SLOT(textEdited(const QString &)));
  layout->addWidget(m_lineEdit);

  updateLineEdit();
 
  layout->addWidget(new QLabel("\260"));

  setLayout(layout);
}
