#ifndef DESCRIPTIONWINDOW_H
#define DESCRIPTIONWINDOW_H

#include "customwindow.h"
#include "cache.h"
#include "htmlview.h"

class DescriptionWindow : public CustomWindow {
  Q_OBJECT;

 public:
  DescriptionWindow(const Cache &, QWidget * = 0);
  ~DescriptionWindow();

 private slots:
  void openCacheUrl();

 private:
  Cache m_cache;
};

#endif
