/*
 *  This file is part of CacheMe
 *
 *  (c) 2011 by Till Harbaum <till@harbaum.org>
 */

#ifndef COORDINATE_TOOL_H
#define COORDINATE_TOOL_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QVBoxLayout>

#include <QGeoCoordinate>
#include "mapwidget.h"
#include "iconloader.h"
#include "pickermanager.h"
#include "cache.h"
#include "waypointdb.h"

QTM_USE_NAMESPACE

class CoordinateTool: public QObject {
  Q_OBJECT;

 public:
  CoordinateTool(const Cache &, WaypointDb *, MapWidget *, QWidget * = 0);
  ~CoordinateTool();

  QWidget *createWidget();
  void addWidget(QWidget *);
  QGeoCoordinate coordinate();

  static QString latitudeString(const QGeoCoordinate &);
  static QString longitudeString(const QGeoCoordinate &);
  static QString zeroCut(qreal, int, int);

 signals:
  void coordinateChanged(const QGeoCoordinate &);
  void typeChanged(const Waypoint &);
  void typeChanged();

 private slots:
  void waypointAccepted();
  void coordinatesAccepted();
  void selectWaypoint(const QString &);
  void newTriggered();
  void editTriggered();
  void deleteTriggered();
  void copyTriggered();

 private:
  QVBoxLayout *m_vboxLayout;
  void updatedCoordinate(QGeoCoordinate &);

  QDialog *createWaypointDialog();
  QDialog *createCoordinateDialog();

  QWidget *m_parent;
  QDialog *m_dialog, *m_wptDialog;

  Cache m_cache;
  IconLoader *m_iconLoader;
  QLabel *m_latitudeLabel, *m_longitudeLabel;

  PickerManager *m_latManager, *m_lonManager;

  QLineEdit *m_nameEdit;
  QTextEdit *m_descEdit;

  QAction *m_actionNew, *m_actionEdit, *m_actionDelete;

  // current coordinate
  QGeoCoordinate m_coo;

  // waypoint to work on 
  Waypoint m_wpt;
  WaypointDb *m_wptDb;

  QComboBox *m_cBox;

  bool m_modeNew;
  QList<Waypoint> m_customWpts;

  // link to map widget to be able to request map and gps positions
  MapWidget *m_mapWidget;

  QGeoCoordinate m_gpsCoordinate;
};

#endif // COORDINATE_TOOL_H
