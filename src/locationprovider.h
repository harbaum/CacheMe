#ifndef LOCATIONPROVIDER_H
#define LOCATIONPROVIDER_H

#include <QGeoPositionInfoSource>

#include <QCompass>

QTM_USE_NAMESPACE

class LocationProvider : public QObject {
  Q_OBJECT;

 public:
  LocationProvider();
  ~LocationProvider();

 private slots:
  void positionHasBeenUpdated(const QGeoPositionInfo &);
  void checkCompassReading();

 signals:
  void positionUpdated(const QGeoPositionInfo &);
  void positionUpdated(const QCompassReading *);

 private:
  QGeoPositionInfoSource *m_location;
  QCompass *m_compass;
};

#endif // LOCATIONPROVIDER_H
