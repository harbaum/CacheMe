#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDebug>

#include "mainwindow.h"
#include "locationprovider.h"
#include "config.h"

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  QString locale = QLocale::system().name();
  QTranslator translator;

#ifdef LIBDIR
  // make sure the geoservice plugins in /usr/lib/cacheme/plugins
  // can be found
  app.addLibraryPath(QString(LIBDIR) + "/plugins");
#endif  

#ifdef DATADIR
  // try to load translation, first from local, then from DATADIR
  if(!translator.load(QString(APPNAME) + "_" + locale))
    translator.load(QString(APPNAME) + "_" + locale, 
		    QString(DATADIR) + "/lang");
#else
  translator.load(QString(APPNAME) + "_" + locale);
#endif
  
  app.installTranslator(&translator);
  
  // these values are important to store app settings
  app.setOrganizationName("Till Harbaum");
  app.setOrganizationDomain("harbaum.org");
  app.setApplicationName(APPNAME);

  LocationProvider locationProvider;

  qDebug() << __FUNCTION__ << "creating main window!";
  MainWindow *mainWindow = new MainWindow(&locationProvider);
  mainWindow->show();

  return app.exec();
}
