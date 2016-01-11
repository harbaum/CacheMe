#include <QDebug>

#include <QNmeaPositionInfoSource>

#include <QApplication>
#include <QFile>

#include "locationprovider.h"

#ifdef GPSDPOSITIONINFOSOURCE
#include "gpsdpositioninfosource.h"
#endif

void LocationProvider::positionHasBeenUpdated(const QGeoPositionInfo &pos) {
  emit positionUpdated(pos);
}

void LocationProvider::checkCompassReading() {
  emit positionUpdated(m_compass->reading());
}

LocationProvider::LocationProvider() : QObject(), 
		       m_location(NULL), m_compass(NULL) {
  qDebug() << __FUNCTION__;

  m_location = QGeoPositionInfoSource::createDefaultSource(this);
  if(!m_location) {
    qWarning() << __FUNCTION__ << "no default position source found";

#ifdef GPSDPOSITIONINFOSOURCE
    qDebug() << __FUNCTION__ << "using gpsd";
    m_location = new GpsdPositionInfoSource(this);
#else
    qDebug() << __FUNCTION__ << "running simulation from nmea log";

    // if everything fails: try to load nmealog.txt from parent dir
    QNmeaPositionInfoSource *nmeaLocation = 
      new QNmeaPositionInfoSource(QNmeaPositionInfoSource::SimulationMode, this);
    nmeaLocation->setDevice(new QFile(QApplication::applicationDirPath() + 
				      + "/../nmealog.txt", this));
    m_location = nmeaLocation;
#endif
  }

  if(m_location) {
    m_location->setUpdateInterval(1000);
  
    connect(m_location, SIGNAL(positionUpdated(QGeoPositionInfo)),
	    this, SLOT(positionHasBeenUpdated(QGeoPositionInfo)));  

    m_location->startUpdates();
  } else
    qWarning() << __FUNCTION__ << ": Position reporting finally failed!";

  m_compass = new QCompass(this);
  connect(m_compass, SIGNAL(readingChanged()), 
	  this, SLOT(checkCompassReading()));

  if(!m_compass->start()) {
    qDebug() << __FUNCTION__ << "failed to start compass";

    // and forget aboout it
    delete m_compass;
    m_compass = NULL;
  }
}

LocationProvider::~LocationProvider() {
  qDebug() << __FUNCTION__;

  if(m_location) 
    m_location->stopUpdates();

  if(m_compass) 
    delete m_compass;
}
