#ifndef PICKERWIDGET_H
#define PICKERWIDGET_H

#include <QWidget>
#include <QStringList>
#include <QPushButton>
#include <QLabel>

class PickerManager;

class PickerWidget : public QWidget {
  Q_OBJECT;

public:
  PickerWidget(QWidget * = 0);
  void setManager(PickerManager *, int);
  void create();
  bool canDecrease();
  bool canIncrease();
  int value();
  void evaluate();
  void set(QChar);

public slots:
  void increase();
  void decrease();

private:
  QPushButton *m_btnDec, *m_btnInc;
  enum Mode { Number, LatChar, LonChar };
  QLabel *m_label;
  Mode m_mode;
  QChar m_chr;
  
  PickerManager *m_pickerManager;
  int m_id;
};

#endif // PICKERWIDGET_H
