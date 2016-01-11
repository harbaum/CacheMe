#ifndef OC_H
#define OC_H

#include <QObject>
#include <QList>
#include <QNetworkReply>
#include <QStringList>

#include "cachelist.h"
#include "cacheproviderplugin.h"

class Oc : public QObject, public CacheProviderPlugin {
 Q_OBJECT
 Q_INTERFACES(CacheProviderPlugin)
    
public:
  Oc();
  ~Oc();

  const QVariant getInfo(const QString &) const;
  QObject *object();

  void init(QWidget *, const QMap<QString, QVariant>&);

  bool busy();

  void request(const Params &);

  void registerSettings(QDialog *);

private slots:
  void replyFinished(QNetworkReply*);

 signals:
  void reply(const Params &);
  void replyCache(const Params &, const Cache &);
  void replyCacheList(const Params &, const CacheList &);

  void notifyBusy(bool);
  void reload();
  void done();
  void next();

 private:
  enum CurrentRequest { None, Overview, Info, Detail };

  void requestOverview(const QGeoBoundingBox &);
  void requestInfoDetail(const QString &, bool);

  bool parseGeocaches(const QVariantList &);
  bool parseGeocache(const QVariantMap &);
  bool parseLocation(const QVariantMap &, QGeoCoordinate &);
  bool parseFloat(const QVariantMap &, const QString &, qreal &);
  bool parseString(const QVariantMap &, const QString &, QString &);
  bool parseDescription(const QVariantMap &, const QString &, Description &, bool);
  bool parseType(const QVariantMap &, Cache::Type &);
  bool parsePerson(const QVariantMap &, const QString &, QString &);
  bool parseSize(const QVariantMap &, Container::Type &);
  bool parseDate(const QVariantMap &, const QString &, QDate &);
  bool parseLogs(const QVariantMap &, Cache &);
  bool parseLog(const QVariantMap &, Cache &);
  bool parseLogType(const QMap<QString, QVariant> &, Log::Type::Id &);
  bool parseImages(const QVariantList &, Cache &);
  bool parseImage(const QVariantMap &, Cache &);

  void error(const QString &);
  void requestGet(const QString &, const QMap<QString, QString> &);
  bool decodeJson(const QString &);

  QNetworkAccessManager *m_manager;

  Cache m_cache;
  CacheList m_cacheList;
  CurrentRequest m_currentRequest;
};

#endif // OC_H
