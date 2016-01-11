#include "flickcharm.h"

#include <QApplication>
#include <QAbstractScrollArea>
#include <QMouseEvent>
#include <QScrollBar>

#include <QDebug>


#define MSEC 25
#define MULT 100

// http://doc.qt.nokia.com/stable/demos-embedded-flickable-flickable-cpp.html

FlickCharm::FlickCharm(QAbstractScrollArea *scrollArea, 
		       QObject *parent): QObject(parent) {

  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
  addFilter(scrollArea);

  m_scrollArea = scrollArea;
  m_steady = true;
}

FlickCharm::~FlickCharm() {
}

void FlickCharm::scrollTo(const QPoint &newPosition) {
  const QPoint delta = newPosition - m_lastPos;

  int elapsed = m_timeStamp.elapsed();

  if(elapsed > 5 && !m_steady) {
    QPoint speed = QPoint(MULT * MSEC * delta.x() / elapsed, MULT * MSEC * delta.y() / elapsed);
    m_speed = (3*m_speed/4)+(speed/4);

    // limit speed
    if(m_speed.manhattanLength() > 50*MULT)
      m_speed = m_speed * 50*MULT / m_speed.manhattanLength();
  }

  const int x = m_scrollArea->horizontalScrollBar()->value();
  const int y = m_scrollArea->verticalScrollBar()->value();
  m_scrollArea->horizontalScrollBar()->setValue(x - delta.x());
  m_scrollArea->verticalScrollBar()->setValue(y - delta.y());
}

void FlickCharm::addFilter(QWidget *widget) {
  widget->installEventFilter(this);

  const QObjectList& childList = widget->children();
  for(QObjectList::const_iterator it = childList.begin(); it != childList.end(); ++it) 
    if(*it && (*it)->isWidgetType()) 
      addFilter(static_cast<QWidget *>(*it));
}

void FlickCharm::timerEvent(QTimerEvent *event) {
  //  qDebug() << __FUNCTION__ << "timer" << m_count <<  m_speed << m_count * m_speed;
  m_count++;

  // 0..100 -> 0..1
  scrollTo(m_releasePos + m_speed / MULT);

  // create unit vector
  QPoint unit = MULT * m_speed / m_speed.manhattanLength();
  if(m_count >= 1000 || unit.manhattanLength() > m_speed.manhattanLength()) 
    m_timer.stop();
  else
    m_speed -= unit;
  
  QObject::timerEvent(event);
} 

bool FlickCharm::eventFilter(QObject *obj, QEvent *event) {
  QEvent::Type type = event->type();

  // make sure that all child widgets added afterwards also get
  // the event filter installed
  if(type == QEvent::ChildAdded) {
    QChildEvent *childEvent = static_cast<QChildEvent*>(event);
    QObject *child = childEvent->child();
    if(child->isWidgetType()) 
      addFilter(static_cast<QWidget *>(child));

    return false;
  }

  if(type == QEvent::MouseButtonDblClick)
    return true; 

  if((type != QEvent::MouseButtonPress) &&
     (type != QEvent::MouseButtonRelease) &&
     (type != QEvent::MouseMove))
    return false;
  
  QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
  if (type == QEvent::MouseMove && mouseEvent->buttons() != Qt::LeftButton)
    return false;
  
  if (mouseEvent->modifiers() != Qt::NoModifier)
    return false;

  const QPoint mousePos = mouseEvent->globalPos();
  if(mousePos.x() < 0 && mousePos.y() < 0)
    return false;

  bool consumed = false;
  if(m_steady) {
    if (type == QEvent::MouseButtonPress) {
      if(m_timer.isActive())
	m_timer.stop();

      m_speed = QPoint(0,0);
      m_initialPos = mousePos;
    }

    // just ignore press and release events in this state ...
    if (type == QEvent::MouseMove) {
      if ((mousePos - m_initialPos).manhattanLength() > 5) {
	// send fake release event at negative position, so whatever is selected
	// gets unselected
	QMouseEvent *fakeEvent = new QMouseEvent(QEvent::MouseButtonRelease,
			 QPoint(-100,-100), QPoint(-100,-100), Qt::LeftButton,
			 Qt::LeftButton, Qt::NoModifier);
	QApplication::postEvent(obj, fakeEvent);

	scrollTo(mousePos);
	m_steady = false;
      }
      consumed = true;
    }
  } else {
    if (type == QEvent::MouseMove) {
      scrollTo(mousePos);
      consumed = true;
    } else if (type == QEvent::MouseButtonRelease) {
      if(m_timeStamp.elapsed() < 100) {
	// start timer for kinetics ...
	m_steady = true;
	m_timer.start(MSEC, this);
	m_releasePos = mousePos;
	m_count = 0;
      }

      consumed = true;
    }
  }

  m_timeStamp = QTime::currentTime();
  m_lastPos = mousePos;
  return consumed;
}

