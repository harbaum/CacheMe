#include <QDebug>
#include "swapbox.h"

SwapBox::SwapBox(QWidget *parent) : QWidget(parent) {
  // assume portrait for now. the actual formatting will be done
  // by a resize event afterwards
  bool landscape = false;
  
  m_layout = new QBoxLayout(landscape?
			    QBoxLayout::LeftToRight:
			    QBoxLayout::TopToBottom, this);
  
  m_layout->setContentsMargins(0,0,0,0);
  setLayout(m_layout);
}

void SwapBox::addWidget(QWidget *widget, int stretch, 
			Qt::Alignment alignment) {
  m_layout->addWidget(widget, stretch, alignment);
}

void SwapBox::setPortraitOnly(QWidget *widget) {
  m_portraitHide.append(widget);
}

void SwapBox::resizeEvent(QResizeEvent *) { 
  bool landscape = 
    window()->size().width() > window()->size().height();

  //  qDebug() << __FUNCTION__ << window()->size() << landscape;
  
  m_layout->setDirection(landscape?
			 QBoxLayout::LeftToRight:
			 QBoxLayout::TopToBottom);
  
  foreach(QWidget *widget, m_portraitHide)
    widget->setVisible(landscape);
}
