#ifndef HTMLVIEW_H
#define HTMLVIEW_H

#include <QTextBrowser>
#include <QNetworkReply>
#include <QLabel>
#include <QMap>

#include "cache.h"

class HtmlView : public QTextBrowser {
  Q_OBJECT;

 public:
  HtmlView(const Description &, QWidget * = 0);
  HtmlView(const QUrl &, QWidget * = 0);

 public slots:
  void replyFinished(QNetworkReply*);

 private:
  QString getHashedName(const QUrl &);
  void writeToCache(const QUrl &, const QByteArray &);
  QByteArray readFromCache(const QUrl &);
  QVariant loadResource(int, const QUrl &);
  QNetworkAccessManager *m_manager;

  QMap<QUrl, QPixmap> m_images;
};

class FixHtmlView : public HtmlView {
  Q_OBJECT;

 public:
  FixHtmlView(const Description &, QWidget *);

 private:
  bool eventFilter(QObject *, QEvent *);
  void adjust();
  void showEvent(QShowEvent *);
  void resizeEvent(QResizeEvent *);

  int m_width;
};

class HtmlLabel : public QLabel {
 public:
  HtmlLabel(const Description &, QWidget *);
};

#endif // HTMLVIEW_H
