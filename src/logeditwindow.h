#ifndef LOGEDITWINDOW_H
#define LOGEDITWINDOW_H

#include <QTextEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QCheckBox>

#include "customwindow.h"
#include "cacheprovider.h"
#include "cache.h"

class LogEditWindow : public CustomWindow {
  Q_OBJECT;

 public:
  LogEditWindow(const Cache &, CacheProvider *, QWidget * = 0);

 private slots:
  void submitLog();
  void handleReply(const Params &);

 private:
  Cache m_cache;
  CacheProvider *m_cacheProvider;

  QTextEdit *m_textEdit;
  QDateEdit *m_dateEdit;
  QComboBox *m_cBox;
  QCheckBox *m_favorite;
  QCheckBox *m_encrypt;
  QCheckBox *m_publish;
  QPushButton *m_submitButton;
};

#endif
