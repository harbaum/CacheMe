#include <QAction>
#include <QSettings>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>

#include <math.h>

#include "tool_projection.h"
#include "swapbox.h"

#define TOOLNAME  "Projection"

/* great circle radius in meters */
#define GCRAD 6371000.0f

void ToolProjection::updateTarget() {
  QGeoCoordinate coo = m_coordinateTool->coordinate();
  qreal dist = m_distanceWidget->value();
  qreal dir = m_directionWidget->value();
  QGeoCoordinate target;

  // from: http://www.movable-type.co.uk/scripts/latlong.html
  target.setLatitude(asin(sin(coo.latitude()/180*M_PI) * cos(dist/GCRAD) + 
			  cos(coo.latitude()/180*M_PI) * sin(dist/GCRAD) * 
			  cos(dir/180*M_PI) )/M_PI*180);

  target.setLongitude(coo.longitude() + 
		      atan2(sin(dir/180*M_PI)*sin(dist/GCRAD)*
			    cos(coo.latitude()/180*M_PI), 
			    cos(dist/GCRAD)-sin(coo.latitude()/180*M_PI)*
			    sin(target.latitude()/180*M_PI))/M_PI*180);

  // normalise to -180...+180
  target.setLongitude(fmodf(target.longitude()+180,360) - 180);

  m_latitude->setInfo(CoordinateTool::latitudeString(target));
  m_longitude->setInfo(CoordinateTool::longitudeString(target));

  m_wpt.setCoordinate(target);
}

void ToolProjection::saveWaypoint() {
  m_wptDb->update(m_wpt);
}

void ToolProjection::distanceChanged(qreal) {
  updateTarget();
}

void ToolProjection::directionChanged(qreal) {
  updateTarget();
}

void ToolProjection::coordinateChanged(const QGeoCoordinate &) {
  updateTarget();
}

ToolProjection::ToolProjection(MapWidget *mapWidget, WaypointDb *wptDb, 
			       QWidget *parent): 
  Tool(TOOLNAME, parent), m_wptDb(wptDb), m_coordinateTool(NULL) {
  setWindowTitle(tr("Projection"));

  /* fill waypoint entry */
  m_wpt.setType(Waypoint::Custom);
  m_wpt.setDescription(tr("Projection"));
  m_wpt.setName(mapWidget->selectedCache().name() + "-PRJ");

  SwapBox *swapBox = new SwapBox();

  QWidget *mainWidget = new QWidget;
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(0,0,0,0);
  mainLayout->setSpacing(0);
  mainWidget->setLayout(mainLayout);

  QSettings settings;
  settings.beginGroup("Tools/" TOOLNAME);

  /* ------------- left/top -------------- */

  m_coordinateTool = new CoordinateTool(mapWidget->selectedCache(), 
					wptDb, mapWidget, this);
  connect(m_coordinateTool, SIGNAL(coordinateChanged(const QGeoCoordinate &)),
	  this, SLOT(coordinateChanged(const QGeoCoordinate &)));
  mainLayout->addWidget(m_coordinateTool->createWidget());

  mainLayout->addStretch(1);

  mainLayout->addWidget(new QLabel(tr("Distance") + ":"));
  m_distanceWidget = new DistanceWidget(this);
  m_distanceWidget->setValue(settings.value("distance").toFloat());
  m_distanceWidget->setUnit((DistanceWidget::Unit)settings.value("unit", 
			 (int)m_distanceWidget->unit()).toInt());
  connect(m_distanceWidget, SIGNAL(changed(qreal)), 
	  this, SLOT(distanceChanged(qreal)));
  mainLayout->addWidget(m_distanceWidget);

  mainLayout->addStretch(1);

  mainLayout->addWidget(new QLabel(tr("Direction") + ":"));
  m_directionWidget = new DirectionWidget(this);
  m_directionWidget->setValue(settings.value("direction").toFloat());
  connect(m_directionWidget, SIGNAL(changed(qreal)), 
	  this, SLOT(directionChanged(qreal)));
  mainLayout->addWidget(m_directionWidget);

  swapBox->addWidget(mainWidget);

  /* ------------- right/bottom -------------- */

  mainWidget = new QWidget;
  mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(0,0,0,0);
  mainLayout->setSpacing(0);
  mainWidget->setLayout(mainLayout);

#ifdef Q_OS_SYMBIAN
  QWidget *groupBox = new QWidget();
#else
  QGroupBox *groupBox = new QGroupBox();
#endif
  QVBoxLayout *vboxLayout = new QVBoxLayout;
  vboxLayout->setContentsMargins(0,0,0,0);
  vboxLayout->setSpacing(0);
  groupBox->setLayout(vboxLayout);

  vboxLayout->addStretch(1);

  vboxLayout->addWidget(new QLabel(tr("Result") + ":"));
  m_latitude = new InfoWidget(tr("Latitude"), "---");
  vboxLayout->addWidget(m_latitude);
  m_longitude = new InfoWidget(tr("Longitude"), "---");
  vboxLayout->addWidget(m_longitude);

  vboxLayout->addStretch(1);

  QPushButton *wptButton = new QPushButton(tr("Create Waypoint"));
  connect(wptButton, SIGNAL(clicked()), this, SLOT(saveWaypoint()));
  vboxLayout->addWidget(wptButton);

  vboxLayout->addStretch(1);

  updateTarget();

  mainLayout->addWidget(groupBox);

  swapBox->addWidget(mainWidget);

  setCentralWidget(swapBox);
  settings.endGroup();
}

ToolProjection::~ToolProjection() {
  QSettings settings;
  settings.beginGroup("Tools/" TOOLNAME);

  // save form contents ...
  settings.setValue("distance", m_distanceWidget->value());
  settings.setValue("unit", (int)m_distanceWidget->unit());
  settings.setValue("direction", m_directionWidget->value());

  settings.endGroup();
}

QAction *ToolProjection::registerMenu(QMenu *menu, QObject *target, const char *slot) {
  return Tool::registerMenu(QObject::tr("&Projection"), menu, target, slot);
}
