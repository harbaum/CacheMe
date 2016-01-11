#include <QDebug>

#include <QVBoxLayout>
#include <QString>

#include "logwidget.h"
#include "htmlview.h"
#include "iconlabel.h"

#ifdef FLICKCHARM
#include "flickcharm.h"
#endif

LogWidget::LogWidget(IconLoader *loader, const Log &log,
		     QWidget *parent) : LogWidgetBase(parent) {
  QString foundByMessage("(" + log.type().toString() + ") " +
			 log.date().toString() + " " + tr("by") + " " + log.finder());
  
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins(0,0,0,0);
  
  layout->addWidget(new IconLabel(loader, log.type().iconFile(),
				  foundByMessage));
  
  layout->addWidget(new FixHtmlView(log.description(), this));
  //    layout->addWidget(new HtmlLabel(log.description(), this));
  setLayout(layout);
};
