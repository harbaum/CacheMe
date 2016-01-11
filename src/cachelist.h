#ifndef CACHELIST_H
#define CACHELIST_H

#include "cache.h"

class CacheList : QList<Cache> {
 public:
  CacheList();
  CacheList(const QString &);
  ~CacheList();
  void append(Cache);
  int size() const;
  Cache at(int);
  Cache last();
  void removeFirst();
  void setName(const QString &);
  QString name();
  QList<Cache>::const_iterator begin() const;
  QList<Cache>::const_iterator end() const;
  QList<Cache>::const_iterator;
  QList<Cache>::iterator begin();
  QList<Cache>::iterator end();
  void clear();
  void setDate(const QDate &);
  QDate date() const;
  void sortByDistance(const QGeoCoordinate &);
  void sortByDescription();
  void clip(const QGeoBoundingBox &);
  void removeByName(const QString &);
  bool save(QXmlStreamWriter *, const QString &);
  bool load(QXmlStreamReader &);

 private:
  void sortByDistance(qreal *, int, int);
  void sortByDescription(int, int);
  QString m_name;
  QDate m_date;
};

#endif // CACHELIST_H
