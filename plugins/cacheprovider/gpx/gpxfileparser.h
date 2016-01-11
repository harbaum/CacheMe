/*
*/  

#ifndef GPXFILEPARSER_H
#define GPXFILEPARSER_H

#include <QObject>
#include "gpxparser.h"

class GpxFileParser : public QObject, GpxParser {
  Q_OBJECT;

 public:
  GpxFileParser();

 public slots:
  void parse(const QString &);
  void setName(const QString &);

 signals:
  void succeeded(const CacheList &);
  void failed(const QString &);

 private:
  QString parseGpxFile(QIODevice &);
  QString parseGpxWptFile(QIODevice &);

  CacheList m_cacheList;
  QString m_name;
};


#endif // GPXFILEPARSER_H
