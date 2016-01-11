#ifndef PICKERMANAGER_H
#define PICKERMANAGER_H

#include <QWidget>
#include <QBoxLayout>
#include <QList>

#include <QGeoCoordinate>
QTM_USE_NAMESPACE

#include "pickerwidget.h"

class PickerManager : public QWidget {
  Q_OBJECT;

public:
  enum Mode { Latitude, Longitude };

  PickerManager(const QGeoCoordinate &, Mode mode, QWidget *parent = 0);
  void addWidgets(bool, QBoxLayout *);
  void createWidget(QBoxLayout *);
  bool canDecrease(int);
  bool canIncrease(int);
  void decrease(int);
  void increase(int);
  void evaluateAll();
  int upperBound(int);
  void evaluate();
  void updateWidgets();
  void setCoordinate(const QGeoCoordinate &);
  QGeoCoordinate coordinate();

private:
  Mode m_mode;
  QGeoCoordinate m_coo;
  QList <PickerWidget*> m_widget;
};

#endif // PICKERMANAGER_H
