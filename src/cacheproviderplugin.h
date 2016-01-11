#ifndef CACHEPROVIDERPLUGIN_H
#define CACHEPROVIDERPLUGIN_H

#include <QDialog>
#include <QGeoBoundingBox>
#include <QMap>
QTM_USE_NAMESPACE

#include "cache.h"
class CacheProvider;

typedef QMap<QString, QVariant> Params;

class CacheProviderPlugin {
 public:
  enum requestFlags {
    None        = 0x00,
    NoFound     = 0x01,  // don't return found caches
    NoOwned     = 0x02,  // don't return owned caches
    Manual      = 0x04   // request has been triggered by user
  };

  virtual const QVariant getInfo(const QString &) const = 0;

  virtual QObject *object() = 0;

  virtual void init(QWidget *, const QMap<QString, QVariant> &) = 0;
  virtual void registerSettings(QDialog *) = 0;

  virtual bool busy() = 0;

  // new interface: just "request"
  virtual void request(const Params &) = 0;
};

Q_DECLARE_INTERFACE(CacheProviderPlugin,
      "org.harbaum.cacheme.cacheproviderplugin/1.0");

#endif // CACHEPROVIDERPLUGIN_H
