#ifndef TOOL_H
#define TOOL_H

#include <QMenu>
#include <QAction>
#include "customwindow.h"

class Tool: public CustomWindow {
  Q_OBJECT;

public:
  Tool(const QString &, QWidget * = 0);
  ~Tool();

  static QAction *registerMenu(const QString &, QMenu *, QObject *, const char *);

private:
};

#endif // TOOL_H
