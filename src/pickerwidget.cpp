#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>

#include "pickermanager.h"
#include "pickerwidget.h"

void PickerWidget::setManager(PickerManager *manager, int id) {
  m_pickerManager = manager;
  m_id = id;
}

int PickerWidget::value() {
  if(!m_chr.isDigit())
    return(m_chr == 'S' || m_chr == 'W')?-1:1;
  
  return m_chr.digitValue();
}

bool PickerWidget::canDecrease() {
  // any number widget can decrease unless all number widgets
  // have reached 0

  if((m_chr == 'S') || (m_chr == 'W'))  return false;
  if((m_chr == 'N') || (m_chr == 'E'))  return true;

  Q_ASSERT(m_chr.isDigit());
  if(m_chr.digitValue() > 0) return true;

  Q_ASSERT(m_pickerManager);
  return m_pickerManager->canDecrease(m_id);
}

bool PickerWidget::canIncrease() {
  if((m_chr == 'S') || (m_chr == 'W'))  return true;
  if((m_chr == 'N') || (m_chr == 'E'))  return false;

  Q_ASSERT(m_chr.isDigit());
  Q_ASSERT(m_pickerManager);

  if(m_chr.digitValue() < m_pickerManager->upperBound(m_id)) 
    return true;

  return m_pickerManager->canIncrease(m_id);
}

void PickerWidget::increase() {
  if(m_chr == 'S') m_chr = 'N';
  else if(m_chr == 'W') m_chr = 'E';
  else if(m_chr.isDigit()) {
    int val = m_chr.digitValue();
    int max = m_pickerManager->upperBound(m_id);

    if(val >= 0 && val < max) m_chr = QChar('0'+val+1);
    else if(val == max) {
      m_pickerManager->increase(m_id-1);
      m_chr = '0';
    }
  }

  m_label->setText(m_chr);
  m_pickerManager->evaluateAll();
}

void PickerWidget::decrease() {
  if(m_chr == 'N') m_chr = 'S';
  else if(m_chr == 'E') m_chr = 'W';
  else if(m_chr.isDigit()) {
    int val = m_chr.digitValue();
    int max = m_pickerManager->upperBound(m_id);

    if(val > 0 && val <= max) m_chr = QChar('0'+val-1);
    else if(val == 0) {
      // underflow, decrease previous digit
      m_pickerManager->decrease(m_id-1);
      m_chr = QChar('0'+max);
    }
  }

  m_label->setText(m_chr);
  m_pickerManager->evaluateAll();
}

void PickerWidget::create() {
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(0);

  m_btnInc = new QPushButton("+");
  m_btnInc->setEnabled(false);
  layout->addWidget(m_btnInc);
  connect(m_btnInc, SIGNAL(clicked()), this, SLOT(increase()));

  m_label = new QLabel();
  m_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  layout->addWidget(m_label);
  
  m_btnDec = new QPushButton("-");
  m_btnDec->setEnabled(false);
  layout->addWidget(m_btnDec);
  connect(m_btnDec, SIGNAL(clicked()), this, SLOT(decrease()));
  
  setLayout(layout);
}

void PickerWidget::set(QChar chr) {
  m_chr = chr;
  m_label->setText(chr);
}

void PickerWidget::evaluate() {
  m_btnInc->setEnabled(canIncrease());
  m_btnDec->setEnabled(canDecrease());
}

PickerWidget::PickerWidget(QWidget *parent) : 
  QWidget(parent), m_chr(' '), m_pickerManager(NULL) {
  create();
}
