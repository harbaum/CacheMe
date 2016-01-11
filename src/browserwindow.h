#ifdef BUILTINBROWSER

#include <QWidget>
#include "customwindow.h"

class BrowserWindow : public CustomWindow {
  Q_OBJECT;

public:
  BrowserWindow(const QUrl &, QWidget *);
};

#endif
