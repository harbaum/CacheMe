#ifndef ATTRIBUTEWINDOW_H
#define ATTRIBUTEWINDOW_H

#include "customwindow.h"
#include "cache.h"
#include "iconloader.h"

class AttributeWindow : public CustomWindow {
  Q_OBJECT;

 public:
  AttributeWindow(const QList<Attribute> &, QWidget * = 0);
  ~AttributeWindow();

 private:
  QList<Attribute> m_attributes;
  IconLoader m_iconLoader;
};

#endif
