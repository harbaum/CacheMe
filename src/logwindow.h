#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include "customwindow.h"
#include "cache.h"
#include "cacheprovider.h"
#include "iconloader.h"

class LogWindow : public CustomWindow {
  Q_OBJECT;

 public:
  LogWindow(const Cache &, CacheProvider *, QWidget * = 0);
  ~LogWindow();

 private slots:
  void createLog();

 private:
  IconLoader m_iconLoader;
  Cache m_cache;
  CacheProvider *m_cacheProvider;
};

#endif
