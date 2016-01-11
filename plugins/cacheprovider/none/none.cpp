#include <QDebug>
#include <QtPlugin>

#include "none.h"
#include "cache.h"

#define PLUGIN_NAME "None"

const QVariant None::getInfo(const QString &name) const {
  if(name.compare("Name") == 0)
    return PLUGIN_NAME;

  if(name.compare("CanBeDefault") == 0)
    return true;

  if(name.compare("License") == 0)
    return QObject::tr("Copyright 2011 by Till Harbaum");

  return QVariant();
}

None::None() { }

None::~None() { }

QObject *None::object() {
  return this;
}

void None::init(QWidget *, const QMap<QString, QVariant> &) {   
  qDebug() << PLUGIN_NAME << __FUNCTION__;
  emit reload();
}

bool None::busy() {
  return false;
}

void None::request(const Params &params) {
  QString type = params.value("Type").toString();

  Q_ASSERT(m_currentRequest == None);

  if(type.compare("Overview") == 0) {
    Params params;
    params.insert("Type", "Overview");
    CacheList cacheList(PLUGIN_NAME);
    
    emit done();
    emit replyCacheList(params, cacheList);
    emit next();
    return;
  }
  
  if((type.compare("Info") == 0) || (type.compare("Detail") == 0)) {
    emit done();
    emit next();
    return;
  }

  qDebug() << __FUNCTION__ << "unexpected request" << type;
  emit done();
  emit next();
}

void None::registerSettings(QDialog *) { }

Q_EXPORT_PLUGIN2(none, None);
