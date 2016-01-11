// Filename: attributewindow.cpp

#include <QDebug>

#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QFont>

#include "attributewindow.h"
#include "iconlabel.h"

#ifdef FLICKCHARM
#include "flickcharm.h"
#endif

AttributeWindow::AttributeWindow(const QList<Attribute> &attributes, 
				 QWidget *parent) : 
  CustomWindow("AttributeWindow", parent),  
  m_attributes(attributes), m_iconLoader(24) {

  setWindowTitle(tr("Attributes"));

  QScrollArea *scrollArea = new QScrollArea(this);
  scrollArea->setWidgetResizable(true);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

#ifdef FLICKCHARM
  new FlickCharm(scrollArea, this);
#endif
  
  QWidget *scrollWidget = new QWidget;
  QVBoxLayout *scrollvbox = new QVBoxLayout;
  scrollvbox->setContentsMargins(0,0,0,0);

  foreach(Attribute attribute, attributes)
    scrollvbox->addWidget(new IconLabel(&m_iconLoader, 
		attribute.iconFile(),
		attribute.toString(), 1.5, this), 0,0);

  scrollvbox->addStretch(1);
  scrollWidget->setLayout(scrollvbox);
  scrollArea->setWidget(scrollWidget);
  
  setCentralWidget(scrollArea);
}

AttributeWindow::~AttributeWindow() {
  qDebug() << __FUNCTION__;
}

