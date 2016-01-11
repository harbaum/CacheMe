#include "filefinder.h"

#include <QDebug>
#include <QApplication>
#include <QFile>
#include <QStringList>
#include <QDesktopServices>

#include "config.h"

QString FileFinder::assembleFilename(const QStringList &parts) {
  Q_ASSERT(parts.size() > 1);

  QString fileName;

  QStringList::const_iterator i = parts.begin();
  if(*i == ".") {
    fileName = QApplication::applicationDirPath();
    i++;
  }

  for(;i!=parts.end();i++)
    fileName += "/" + *i;

  return fileName;
}

QString FileFinder::find(const QString &type, const QString &name) {
  // try internal resource first
  QString full = ":" + name;
  QFile file(full);

  // internal resource does not exist, try local file then
  if(!file.exists()) {

    // try local filename then
    full = assembleFilename(QStringList(".")<<"data"<<type<<name);
    file.setFileName(full);

    // local file does not exist, try system wide file
    if(!file.exists()) {
#ifdef DATADIR
      full = assembleFilename(QStringList(DATADIR) << type << name);
      file.setFileName(full);
      if(!file.exists()) {
        qWarning() << __FUNCTION__ << "Unable to find file for " << name;
        return NULL;
      }
#else
      qWarning() << __FUNCTION__ << "Unable to find file for " << name;
      return NULL;
#endif
    }
  }

  return full;
}

// return where permanent bulk data is to be stored
// used for waypoint db and cached tiles
QString FileFinder::dataPath() {
#ifdef Q_OS_SYMBIAN
  return 
    QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) 
    + "/" + APPNAME;
#else
  return 
    QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
}
