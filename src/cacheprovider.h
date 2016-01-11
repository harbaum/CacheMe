#ifndef CACHEPROVIDER_H
#define CACHEPROVIDER_H

#include "cachelist.h"

#include <QQueue>
#include <QDir>
#include <QVBoxLayout>
#include <QMenuBar>

#include "cacheproviderplugin.h"

class CacheProvider  : public QObject {
  Q_OBJECT;

 public:
  CacheProvider(QWidget *parent = 0);
  ~CacheProvider();
  void requestOverview(const QGeoBoundingBox &, int, int = CacheProviderPlugin::None);
  void requestInfo(const QString &);
  void requestDetail(const QString &);
  void requestCustom(const QMap<QString, QVariant> &);
  void start(QWidget * = 0);
  QStringList names();
  QStringList licenses();
  void registerSettings(QDialog *);

  // functions redirected at current plugin
  QString name();
  bool busy();
  void processRequestOverview(const QGeoBoundingBox &, const int, const int);
  void processRequestInfo(const QString &);
  void processRequestDetail(const QString &);
  void processRequestCustom(const QMap<QString, QVariant> &);

  void createMenu(QMenuBar *);
  bool loggingIsSupported();
  QString getInfoString(const QString &);
  bool getInfoBool(const QString &);

 public slots:
  void next();
  void done();

 private slots:
  void emitReply(const Params &);
  void emitReplyCache(const Params &, const Cache &);
  void emitReplyCacheList(const Params &, const CacheList &);

  void emitReload();
  void emitNotifyBusy(bool);
  void providerSelected(QAction *);

 signals:
  void reply(const Params &);
  void replyCache(const Params &, const Cache &);
  void replyCacheList(const Params &, const CacheList &);

  void reload();
  void notifyBusy(bool);
  void waypointsChanged();
  void manualUpdateRequired(bool);
  void searchSupported(bool);
  void authorizationSupported(bool);

 public:

  // a request entry is being kept in the request queue
  class RequestEntry {    
    
  public: 
    enum RequestType { Overview, Info, Detail, Custom };
    RequestEntry(const RequestType &, const QMap<QString, QVariant> &);
    RequestEntry(const RequestType &, const QString &);
    RequestEntry(const RequestType &, const QGeoBoundingBox &, const int, const int);
    int flags();
    int zoom();
    RequestType type();
    QMap<QString, QVariant> params();
    QGeoBoundingBox bbox();
    QString cache();
    void set(const QGeoBoundingBox &, const int, const int);
    void set(const QString &);

  private:
    RequestType m_type;
    QMap<QString, QVariant> m_params;
    QGeoBoundingBox m_bbox;
    QString m_cache;
    int m_flags, m_zoom;
  };

  class RequestQueue : public QQueue<RequestEntry *> {
  private:
    CacheProvider *m_cacheProvider;

  public:
    RequestQueue(CacheProvider *);
    bool add(const RequestEntry::RequestType &, const QMap<QString, QVariant> &);
    bool add(const RequestEntry::RequestType &, const QString &);
    bool add(const RequestEntry::RequestType &, const QGeoBoundingBox &, const int, const int);
    RequestEntry::RequestType type();
    void done();
    void next();
    void restart();
  };
  
  RequestEntry::RequestType type();

 private:
  void loadPluginsInDir(QDir &);
  const QMap<QString, QVariant> getParams();
  bool getInfoBool(const CacheProviderPlugin *, const QString &);
  QString getInfoString(const CacheProviderPlugin *, const QString &);

  // queue of pending requests
  RequestQueue *m_pending;

  // list of available cache provider plugins
  QList <CacheProviderPlugin *> m_cacheProviderPluginList;
  CacheProviderPlugin *m_currentPlugin;
  QWidget *m_parent;
};

#endif // CACHEPROVIDER_H
