#ifndef FLICKCHARM_H
#define FLICKCHARM_H

#include <QWidget>
#include <QBasicTimer>
#include <QPoint>
#include <QTime>
#include <QScrollArea>

class FlickCharm: public QObject {
  Q_OBJECT
public:
  FlickCharm(QAbstractScrollArea *, QObject * = 0);
  ~FlickCharm();
  bool eventFilter(QObject *, QEvent *);
  void timerEvent(QTimerEvent *); 

private:
  void addFilter(QWidget *);
  void scrollTo(const QPoint &);

  bool m_steady;
  QAbstractScrollArea *m_scrollArea;
  QPoint m_lastPos, m_speed, m_initialPos;
  QTime m_timeStamp;
  QBasicTimer m_timer;
  int m_count;
  QPoint m_releasePos;
};

#endif // FLICKCHARM_H
