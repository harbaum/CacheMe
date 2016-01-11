#include <QDebug>
#include "htmlview.h"

#include <QTextDocument>
#include <QNetworkRequest>
#include <QCryptographicHash>
#include <QDesktopServices>

#include <QEvent>
#include <QBuffer>
#include <QHttp>
#include <QDir>

void HtmlView::writeToCache(const QUrl &url, const QByteArray &data) {
  QString cacheDir = 
    QDesktopServices::storageLocation(QDesktopServices::CacheLocation) + "/images";
  
  if(!QDir().mkpath(cacheDir)) {
    qDebug() << __FUNCTION__ << "cannot create cache dir" << cacheDir;
    return;
  }

  QString fullName = cacheDir + "/" + getHashedName(url);
  QFile file(fullName);
  if(!file.open(QIODevice::WriteOnly)) {
    qDebug() << __FUNCTION__ << "unable to open for writing:" << fullName;
    return;
  }

  file.write(data);
  file.close();

  qDebug() << __FUNCTION__ << "wrote: " << fullName;
}

QByteArray HtmlView::readFromCache(const QUrl &url) {
  QString fullName = QDesktopServices::storageLocation(QDesktopServices::CacheLocation) + "/images/" + getHashedName(url);

  QFile file(fullName);
  if(!file.open(QIODevice::ReadOnly)) {
    qDebug() << __FUNCTION__ << "unable to open for reading:" << fullName;
    return NULL;
  }

  QByteArray data = file.readAll();
  file.close();

  qDebug() << __FUNCTION__ << "read: " << fullName;
  return data;
}

QString HtmlView::getHashedName(const QUrl &url) {
  return QCryptographicHash::hash(url.toString().toLatin1(), 
		  QCryptographicHash::Md5).toHex();
}

HtmlView::HtmlView(const Description &desc, QWidget *parent) : QTextBrowser(parent) {
 
  // setup network manager and listen for its replies
  this->m_manager = new QNetworkAccessManager(this);

  connect(this->m_manager, SIGNAL(finished(QNetworkReply*)),
	  this, SLOT(replyFinished(QNetworkReply*)));

  QTextDocument *doc = new QTextDocument(this);
  if(desc.isHtml()) doc->setHtml(desc.text());
  else              doc->setPlainText(desc.text());
  setDocument(doc);

  // open links in external browser
  setOpenExternalLinks(true);

  setReadOnly(true);
}

HtmlView::HtmlView(const QUrl &url, QWidget *parent) : QTextBrowser(parent) {
 
  // setup network manager and listen for its replies
  this->m_manager = new QNetworkAccessManager(this);

  connect(this->m_manager, SIGNAL(finished(QNetworkReply*)),
	  this, SLOT(replyFinished(QNetworkReply*)));

  setSource(url);
  setReadOnly(true);
}
 
QVariant HtmlView::loadResource(int type, const QUrl &url) {
  QUrl localUrl(url); 

  // don't handle data resources
  if(url.toString().startsWith("data"))
    return QVariant();
  
  if(localUrl.host().isNull()) {
    localUrl.setScheme("http");
    localUrl.setHost("www.geocaching.com");
  }

  qDebug() << __FUNCTION__ << localUrl;

  // get from local ram cache
  if(m_images.contains(localUrl)) 
    return m_images.value(localUrl);
    
  // then try disk cache
  QByteArray data = readFromCache(localUrl);
  if(!data.isNull()) {
    QPixmap pix;
    if(pix.loadFromData(data)) {
      // store in local ram cache
      m_images.insert(localUrl, pix);
      return m_images.value(localUrl);
    }
  }

  qDebug() << __FUNCTION__ << "creating network request for" << localUrl;

  // and finally request download of image
  QNetworkRequest request;
  request.setUrl(localUrl);
  QNetworkReply *reply = this->m_manager->get(request);
  if(reply->error())
    replyFinished(reply);

  if(type == QTextDocument::ImageResource)
    m_images.insert(localUrl, QPixmap());

  //  return QVariant();
  return QTextBrowser::loadResource(type, url);
}

void HtmlView::replyFinished(QNetworkReply *reply) {
  qDebug() << __FUNCTION__;

  // whatever happened, we are now idle again
  if(reply->error() != QNetworkReply::NoError) {
    qDebug() << __FUNCTION__ << "Reply error:" << reply->errorString();
    return;
  }

  if(reply->isFinished()) {
    QPixmap pix;
    QByteArray data = reply->readAll();

    qDebug() << __FUNCTION__ << "rcvd bytes" << data.size();

    if(pix.loadFromData(data)) {
      writeToCache(reply->url(), data);
      m_images.insert(reply->url(), pix);
      
      // ugly method to force relayoutDocument()
      qDebug() << __FUNCTION__ << "force relayout";
      QTextDocument *doc = document();
      setDocument(doc);
    } else
      setHtml(QString(data));
  }
}

FixHtmlView::FixHtmlView(const Description &desc, QWidget *parent) : 
  HtmlView(desc, parent) {
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  //  setEnabled(false);
  m_width = 0;
}

bool FixHtmlView::eventFilter(QObject *obj, QEvent *event) {
  if((event->type() == QEvent::KeyPress) ||
     (event->type() == QEvent::Wheel))
    return true;

  return QObject::eventFilter(obj, event);
}

void FixHtmlView::adjust() {
  if(m_width != size().width()) {
    m_width = size().width();
    
    QMargins margins = contentsMargins();
    
    int width = size().width() - 
      margins.left() - margins.right() - document()->documentMargin()*2;
    document()->setPageSize(QSizeF(width,-1));
    
    int height = document()->size().height() + 
      margins.top() + margins.bottom();
    setMaximumHeight(height);
    setMinimumHeight(height);
    
  }
}

void FixHtmlView::showEvent(QShowEvent *) {
  adjust();
}

void FixHtmlView::resizeEvent(QResizeEvent *) {
  adjust();
}

HtmlLabel::HtmlLabel(const Description &desc, QWidget *parent) : 
  QLabel(parent) {
  
  setTextFormat(Qt::RichText);
  setText(desc.text());
  setWordWrap(true);
}
