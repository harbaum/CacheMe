#ifndef GPX_H
#define GPX_H

#include <QObject>
#include <QThread>

#include "cachelist.h"
#include "cacheproviderplugin.h"
#include "gpxfileparser.h"

class Gpx : public QObject, public CacheProviderPlugin {
 Q_OBJECT
 Q_INTERFACES(CacheProviderPlugin)
    
public:
  Gpx();
  ~Gpx();

  const QVariant getInfo(const QString &) const;
  QObject *object();

  void init(QWidget *, const QMap<QString, QVariant> &);

  bool busy();
  void request(const Params &);
  void registerSettings(QDialog *);

 private slots:
  void applyChanges();
  void fileParserFailed(const QString &);
  void fileParserSucceeded(const CacheList &);

 signals:
  void reply(const Params &);
  void replyCache(const Params &, const Cache &);
  void replyCacheList(const Params &, const CacheList &);
  void reload();
  void notifyBusy(bool);
  void done();
  void next();

  void fileParse(const QString &);

 private:
  void error(const QString &);
  void requestOverview(const QGeoBoundingBox &, const int);
  void requestInfo(const QString &);
  void requestDetail(const QString &);

  CacheList m_cacheList;
  GpxFileParser *m_fileParser;
  QThread *m_thread;

  QString m_name;
  bool m_initialized;
  QString m_fileName;
};

#endif // GPX_H
