#include <QHBoxLayout>

#include "infowidget.h"

// a hbox with two strings, one left aligned and one right aligned
InfoWidget::InfoWidget(const QString &leftStr,
	     const QString &rightStr,
	     QWidget *parent) : QWidget(parent) {

  QHBoxLayout *layout = new QHBoxLayout;
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(0);

  m_left = new QLabel(leftStr + ":");
  layout->addWidget(m_left);
  layout->addStretch();
  m_right = new QLabel("<b>" + rightStr + "</b>");
  layout->addWidget(m_right);
  
  setLayout(layout);
}

QLabel *InfoWidget::getLabel() {
  return m_right;
}

void InfoWidget::setInfo(const QString &info) {
  m_right->setText("<b>" + info + "</b>");
}

