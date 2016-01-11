#include <QWidget>
#include <QComboBox>
#include "cache.h"
#include "customwindow.h"

class HintWindow : public CustomWindow {
  Q_OBJECT;

public:
  HintWindow(const Cache &, QWidget *);
  
public slots:
  void showImage();

 private:
  Cache m_cache;
  QComboBox *m_cBox;
};
