#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QWidget>

#ifdef Q_OS_SYMBIAN
#define LogWidgetBase QWidget
#else
#include <QGroupBox>
#define LogWidgetBase QGroupBox
#endif

#include "cache.h"
#include "iconloader.h"

class LogWidget : public LogWidgetBase {
  Q_OBJECT;
 public:
  LogWidget(IconLoader *, const Log &, QWidget * = 0);
};

#endif // LOGWIDGET_H
