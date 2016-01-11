// a SwapBox is a widget with a boxlayout that changes orientation

#ifndef SWAPBOX_H
#define SWAPBOX_H

#include <QWidget>
#include <QResizeEvent>
#include <QBoxLayout>
#include <QList>

class SwapBox : public QWidget {
public:
  SwapBox(QWidget * = 0);
  void addWidget(QWidget *, int = 0, Qt::Alignment = 0 );
  void setPortraitOnly(QWidget *widget);
  void resizeEvent(QResizeEvent *);
  
private:
  QList <QWidget*> m_portraitHide;
  QBoxLayout *m_layout;
};

#endif // SWAPBOX_H
