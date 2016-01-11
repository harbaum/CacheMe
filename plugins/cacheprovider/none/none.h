#ifndef NONE_H
#define NONE_H

#include <QObject>

#include "cachelist.h"
#include "cacheproviderplugin.h"

class None : public QObject, public CacheProviderPlugin {
 Q_OBJECT
 Q_INTERFACES(CacheProviderPlugin)
    
public:
  None();
  ~None();

  const QVariant getInfo(const QString &) const;
  QObject *object();

  void init(QWidget *, const QMap<QString, QVariant> &);

  bool busy();
  void request(const Params &);
  void registerSettings(QDialog *);

 signals:
  void replyCacheList(const Params &, const CacheList &);
  void reload();
  void done();
  void next();
};

#endif // NONE_H
