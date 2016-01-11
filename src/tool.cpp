#include <QDebug>
#include "tool.h"

Tool::Tool(const QString &name, QWidget *parent): 
  CustomWindow(name, parent) {
  qDebug() << __FUNCTION__ << name;
}

Tool::~Tool() {
  qDebug() << __FUNCTION__;

}

QAction *Tool::registerMenu(const QString &name, QMenu *menu, QObject *target, const char *slot) {
  qDebug() << __FUNCTION__;

  QAction *action = new QAction(name, menu);
  connect(action, SIGNAL(triggered()), target, slot);
  menu->addAction(action);

  return action;
}
