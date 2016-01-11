#ifndef FILEFINDER_H
#define FILEFINDER_H

#include <QFile>

class FileFinder {
 public:
  static QString find(const QString &, const QString &);
  static QString dataPath();

 private:
  static QString assembleFilename(const QStringList &);
};

#endif
