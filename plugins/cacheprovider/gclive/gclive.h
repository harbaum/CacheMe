#ifndef GCLIVE_H
#define GCLIVE_H

#include <QObject>
#include <QNetworkReply>
#include <QStringList>

#include "cachelist.h"
#include "cacheproviderplugin.h"

class KQOAuthManager;
class KQOAuthRequest;

class GcLive : public QObject, public CacheProviderPlugin {
 Q_OBJECT
 Q_INTERFACES(CacheProviderPlugin)
    
public:
  GcLive();
  ~GcLive();

  const QVariant getInfo(const QString &) const;
  QObject *object();

  void init(QWidget *, const QMap<QString, QVariant>&);

  bool busy();

  void request(const Params &);

  void registerSettings(QDialog *);

private slots:
  void replyFinished(QNetworkReply*);
  void onTemporaryTokenReceived(QString temporaryToken, QString temporaryTokenSecret);
  void onAuthorizationReceived(QString token, QString verifier);
  void onAccessTokenReceived(QString token, QString tokenSecret);
  void onSslErrors( QNetworkReply *, QList<QSslError> &);
  void onOpenUrl(const QUrl &);

 signals:
  void reply(const Params &);
  void replyCache(const Params &, const Cache &);
  void replyCacheList(const Params &, const CacheList &);

  void notifyBusy(bool);
  void reload();
  void done();
  void next();

 private:
  enum CurrentRequest { None, Overview, Info, Detail, Search, SubmitLog };

  void requestOverview(const QGeoBoundingBox &, const int);
  void requestInfoDetail(const QString &, bool);
  void requestSearch(const QGeoCoordinate &, const QString &, const QStringList &, const QStringList &, bool, bool);
  void requestLog(const QString &, const QString &, const QDate &, Log::Type::Id, bool, bool, bool);

  void error(const QString &);
  void requestGet(const QString &);
  void requestPost(const QString &, const QString &);
  void authorize();
  void requestUserInfo();
  bool decodeJson(const QString &);

  bool parseStatus(const QVariantMap &);
  bool parseCacheLimits(const QVariantMap &);
  bool parseProfile(const QVariantMap &);
  bool parseGeocaches(const QVariantList &);
  bool parseGeocache(const QVariantMap &);
  bool parseImages(const QVariantList &, Cache &);
  bool parseImage(const QVariantMap &, Cache &);
  bool parsePerson(const QVariantMap &, const QString &, QString &);
  bool parseAttributes(const QVariantMap &, Cache &);
  bool parseAttribute(const QVariantMap &, Cache &);
  bool parseWaypoints(const QVariantMap &, Cache &);
  bool parseWaypoint(const QVariantMap &, Cache &);
  bool parseWptType(const QMap<QString, QVariant> &, Waypoint::Type &);
  bool parseLogType(const QMap<QString, QVariant> &, Log::Type::Id &);
  bool parseLogs(const QVariantMap &, Cache &);
  bool parseLog(const QVariantMap &, Cache &);
  bool parseType(const QMap<QString, QVariant> &, Cache::Type &);
  bool parseContainer(const QMap<QString, QVariant> &, Container &);
  bool parseBool(const QMap<QString, QVariant> &, const QString &, bool &); 
  bool parseFloat(const QMap<QString, QVariant> &, const QString &, qreal &);
  bool parseString(const QMap<QString, QVariant> &, const QString &, QString &);
  bool parseInt(const QMap<QString, QVariant> &, const QString &, int &);
  bool parseDate(const QMap<QString, QVariant> &, const QString &, QDate &);
  bool parseDescription(const QMap<QString, QVariant> &, const QString &, Description &);

  // handling of the cache file
  bool save();
  bool load();
  bool loadGclive(QXmlStreamReader &);
  bool loadBasicLimits(QXmlStreamReader &);

  KQOAuthRequest *m_oauthRequest;
  KQOAuthManager *m_oauthManager;
  QNetworkAccessManager *m_manager;

  QString m_consumerKey, m_consumerSecretKey;
  QString m_dataDir;

  int m_uniqueId;
  QString m_token;

  QString m_userName;
  bool m_isBasicMember;
  bool m_initialized;
  bool m_overviewSent;

  CacheList m_cacheDetails;
  CacheList m_cacheList;
  CacheList m_searchResult;
  CurrentRequest m_currentRequest;

  QDateTime m_basicDetailLock;
  int m_basicDetailCount;
  bool m_status;
  QString m_statusMessage;
};

#endif // GCLIVE_H
