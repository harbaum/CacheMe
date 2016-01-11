/*
 *  This file is part of CacheMe
 *
 *  (c) 2011 by Till Harbaum <till@harbaum.org>
 */

#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QToolButton>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <math.h>
#include <QAbstractItemView>

#include "coordinatetool.h"
#include "pickermanager.h"
#include "filefinder.h"
#include "swapbox.h"
#include "infowidget.h"

QGeoCoordinate CoordinateTool::coordinate() {
  return m_coo;
}

void CoordinateTool::coordinatesAccepted() {
  qDebug() << __FUNCTION__;

  // fetch coordinate from dialog
  m_latManager->evaluate();
  m_lonManager->evaluate();
  m_coo = QGeoCoordinate(m_latManager->coordinate().latitude(),
			 m_lonManager->coordinate().longitude());

  // this updates the labels as well as the compass
  updatedCoordinate(m_coo);

  m_wpt.setCoordinate(m_coo);
  
  // add to database
  if(m_modeNew) {
    m_wptDb->append(m_wpt);
    // append to combobox
    // if this is the first custom entry, then add delimiter now
    if(m_wptDb->get(m_cache).size() == 1)
      m_cBox->insertSeparator(m_cBox->count());

    m_cBox->addItem(m_wpt.description());

    // and select the new entry
    m_cBox->setCurrentIndex(m_cBox->count()-1);

    // a newly created entry can be edited and deleted
    m_actionEdit->setEnabled(true);
    m_actionDelete->setEnabled(true);

    m_wpt.setType(Waypoint::Custom);
    emit typeChanged(m_wpt);
  } else {
    m_wptDb->update(m_wpt);
    // no need to update combobox as name cannot be changed
  }

  // reload custom waypoints list
  m_customWpts = m_wptDb->get(m_cache);
}

void CoordinateTool::waypointAccepted() {
  qDebug() << __FUNCTION__ << m_nameEdit->text();

  m_wpt.setDescription(m_nameEdit->text());
  m_wpt.setComment(m_descEdit->toPlainText());

  if(m_dialog) {
    delete m_dialog;
    m_dialog = NULL;
  }
    
  m_dialog = createCoordinateDialog();
  
  // update pickers and launch tool
  m_latManager->updateWidgets();
  m_lonManager->updateWidgets();
  m_dialog->exec();
}

/* build the waypoint name dialog */
QDialog *CoordinateTool::createWaypointDialog() {
  QString idStr;

  // in edit mode, wpt is already set
  idStr = m_wptDb->getUnusedId(m_cache);
  if(idStr.isEmpty()) return NULL;
  m_wpt.setName(idStr);

  QDialog *dialog = new QDialog(m_parent);

  dialog->setWindowTitle(tr("New waypoint"));

  QVBoxLayout *mainLayout = new QVBoxLayout;

  /* ------------- contents ------------- */
  QWidget *hboxWidget = new QWidget;
  QHBoxLayout *hboxLayout = new QHBoxLayout;
  hboxLayout->setContentsMargins(0,0,0,0);
  hboxLayout->setSpacing(0);
  hboxWidget->setLayout(hboxLayout);

  hboxLayout->addWidget(new QLabel(tr("Name:")));
  m_nameEdit = new QLineEdit(dialog);

  m_nameEdit->setText(m_wptDb->getUnusedName(m_cache));
  
  hboxLayout->addWidget(m_nameEdit);

  mainLayout->addWidget(hboxWidget);

  mainLayout->addWidget(new QLabel(tr("Description:")));
  m_descEdit = new QTextEdit(dialog);
  m_descEdit->setAcceptRichText(false);
  if(!m_modeNew) m_descEdit->setText(m_wpt.comment());
  mainLayout->addWidget(m_descEdit);

  /* add button box for "ok" and "cancel" */
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
				     | QDialogButtonBox::Cancel);

  connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

  mainLayout->addWidget(buttonBox);
  dialog->setLayout(mainLayout);  

  connect(dialog, SIGNAL(accepted()), this, SLOT(waypointAccepted()));

  return dialog;
}

/* build the coordinate edit dialog */
QDialog *CoordinateTool::createCoordinateDialog() {
  QDialog *dialog = new QDialog(m_parent);

  if(m_modeNew) dialog->setWindowTitle(tr("New Coordinate"));
  else          dialog->setWindowTitle(tr("Edit Coordinate"));
  
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addStretch(1);

#ifdef Q_OS_SYMBIAN
  /* symbian looks ugly without top and bottom margins, so keep them */
  int left, right, top, bottom;
  mainLayout->getContentsMargins(&left, &top, &right, &bottom);
  mainLayout->setContentsMargins(0,top,0,bottom);
#else
  mainLayout->setContentsMargins(0,0,0,0);
#endif

  // under symbian all widgets fit into one line
#ifdef SWAP_COORDINATES
  SwapBox *swapBox = new SwapBox();
#endif

  /* first half of latitude */
  QWidget *hboxWidget = new QWidget;
  QHBoxLayout *hboxLayout = new QHBoxLayout;
  hboxLayout->setContentsMargins(0,0,0,0);
  hboxLayout->setSpacing(0);
  hboxWidget->setLayout(hboxLayout);

  m_latManager = new PickerManager(m_coo, PickerManager::Latitude);

  hboxLayout->addStretch(1);
  m_latManager->addWidgets(true, hboxLayout);

#ifdef SWAP_COORDINATES
  hboxLayout->addStretch(1);

  swapBox->addWidget(hboxWidget);

  /* second half of latitude */
  hboxWidget = new QWidget;
  hboxLayout = new QHBoxLayout;
  hboxLayout->setContentsMargins(0,0,0,0);
  hboxLayout->setSpacing(0);
  hboxWidget->setLayout(hboxLayout);
#endif

  hboxLayout->addStretch(1);
  m_latManager->addWidgets(false, hboxLayout);
  hboxLayout->addStretch(1);

#ifdef SWAP_COORDINATES
  swapBox->addWidget(hboxWidget);
  mainLayout->addWidget(swapBox);
  swapBox = new SwapBox();
#else
  mainLayout->addWidget(hboxWidget);
  mainLayout->addStretch(1);
#endif

  /* first half of longitude */
  hboxWidget = new QWidget;
  hboxLayout = new QHBoxLayout;
  hboxLayout->setContentsMargins(0,0,0,0);
  hboxLayout->setSpacing(0);
  hboxWidget->setLayout(hboxLayout);

  m_lonManager = new PickerManager(m_coo, PickerManager::Longitude);


  hboxLayout->addStretch(1);
  m_lonManager->addWidgets(true, hboxLayout);

#ifdef SWAP_COORDINATES
  hboxLayout->addStretch(1);
  swapBox->addWidget(hboxWidget);

  /* second half of longitude */
  hboxWidget = new QWidget;
  hboxLayout = new QHBoxLayout;
  hboxLayout->setContentsMargins(0,0,0,0);
  hboxLayout->setSpacing(0);
  hboxWidget->setLayout(hboxLayout);
#endif

  hboxLayout->addStretch(1);
  m_lonManager->addWidgets(false, hboxLayout);
  hboxLayout->addStretch(1);

#ifdef SWAP_COORDINATES
  swapBox->addWidget(hboxWidget);
  mainLayout->addWidget(swapBox);
#else
  mainLayout->addWidget(hboxWidget);
#endif

  mainLayout->addStretch(1);

  /* add button box for "ok" and "cancel" */
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
				     | QDialogButtonBox::Cancel);

  connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

  mainLayout->addWidget(buttonBox);
  dialog->setLayout(mainLayout);  

  connect(dialog, SIGNAL(accepted()), this, SLOT(coordinatesAccepted()));

  return dialog;
}

// on symbian the coordinates fit into one row
#ifndef Q_OS_SYMBIAN
#define SWAP_COORDINATES
#endif

CoordinateTool::CoordinateTool(const Cache &cache, WaypointDb *wptDb,
			       MapWidget *mapWidget, QWidget *parent) :
  m_parent(parent), m_dialog(NULL), m_wptDialog(NULL),
  m_cache(cache), m_iconLoader(NULL), m_wptDb(wptDb), m_mapWidget(mapWidget) {

  qDebug() << __FUNCTION__ << cache.waypoints().size();

  // start with caches main coordinate
  m_coo = m_cache.coordinate();

  m_latitudeLabel = NULL;
  m_longitudeLabel = NULL;

  m_iconLoader = new IconLoader(64);
}

CoordinateTool::~CoordinateTool() {
  qDebug() << __FUNCTION__;

  if(m_wptDialog) {
    delete m_wptDialog;
  }

  if(m_dialog) {
    delete m_dialog;
    delete m_latManager;
    delete m_lonManager;
  }

  if(m_iconLoader)
    delete m_iconLoader;

  // don't destroy the widget as it is owned by its parent 
  qDebug() << __FUNCTION__ << "done";
}

// returns the number num as a string with precision p and c digit mantissa
QString CoordinateTool::zeroCut(qreal num, int p, int c) {
  return QString("000").append(QString::number(num, 'f', p)).right(c+p);
}

QString CoordinateTool::latitudeString(const QGeoCoordinate &coo) {
  QString str;

  if(coo.isValid()) {
    qreal fractional, latitude = coo.latitude();
    double integral;

    if(latitude < 0) { latitude = fabs(latitude); str = tr("S");
    } else                                        str = tr("N");

    fractional = modf(latitude, &integral);
    str += " " + zeroCut(integral, 0, 2) + "\260 " +
      zeroCut(60*fractional, 3, 3) + "'";
  }
  return str;
}

QString CoordinateTool::longitudeString(const QGeoCoordinate &coo) {
  QString str;

  if(coo.isValid()) {
    qreal fractional, longitude = coo.longitude();
    double integral;

    if(longitude < 0) { longitude = fabs(longitude); str = tr("W");
    } else                                           str = tr("E");

    fractional = modf(longitude, &integral);
    str += " " + zeroCut(integral, 0, 3) + "\260 " +
      zeroCut(60*fractional, 3, 3) + "'";
  }
  return str;
}

void CoordinateTool::updatedCoordinate(QGeoCoordinate &coo) {
  m_latitudeLabel->setText("<b>" + latitudeString(coo) + "</b>");
  m_longitudeLabel->setText("<b>" + longitudeString(coo) + "</b>");
  emit coordinateChanged(coo);
}

void CoordinateTool::selectWaypoint(const QString &name) {
  qDebug() << __FUNCTION__ << name;

  m_actionEdit->setEnabled(false);
  m_actionDelete->setEnabled(false);

  m_coo = QGeoCoordinate();

  if(name == m_cache.description()) {
    m_coo = m_cache.coordinate();
    emit typeChanged();
  } else if(name == tr("Map Center")) {
    m_coo = m_mapWidget->center();
  } else if(name == tr("GPS Position")) {
    m_coo = m_gpsCoordinate;
  }

  if(!m_coo.isValid()) {
    foreach(Waypoint wpt, m_cache.waypoints()) {
      if(name == wpt.description()) {
	m_coo = wpt.coordinate();
	emit typeChanged(wpt);
      }
    }
  }

  // find in custom waypoints
  if(!m_coo.isValid()) {
    foreach(Waypoint wpt, m_customWpts) {
      if(name == wpt.description()) {
	m_coo = wpt.coordinate();

	m_wpt = wpt;  // save this to edit it

	emit typeChanged(wpt);

	// custom entries and be edited and deleted
	m_actionEdit->setEnabled(true);
	m_actionDelete->setEnabled(true);
      }
    }
  }

  if(m_coo.isValid())
    updatedCoordinate(m_coo);
  else
    qDebug() << __FUNCTION__ << "could not find" << name;
}

/* todo: allow differnt layout styles */
QWidget *CoordinateTool::createWidget() {
  // create a groupbox for both coordinates

#ifdef Q_OS_SYMBIAN
  QWidget *groupBox = new QWidget();
#else
  QGroupBox *groupBox = new QGroupBox();
#endif
  QHBoxLayout *hboxLayout = new QHBoxLayout;

  QWidget *vboxWidget = new QWidget;
  m_vboxLayout = new QVBoxLayout;
  m_vboxLayout->setContentsMargins(0,0,0,0);
  m_vboxLayout->setSpacing(0);
  vboxWidget->setLayout(m_vboxLayout);


  QWidget *toolBoxWidget = new QWidget;
  QHBoxLayout *toolBoxLayout = new QHBoxLayout;
  toolBoxLayout->setContentsMargins(0,0,0,0);
  toolBoxLayout->setSpacing(0);
  toolBoxWidget->setLayout(toolBoxLayout);

  // --------- button to add/edit/delete coordinates ------------
  QToolButton *button = new QToolButton(); 
  QString filename = FileFinder::find("icons", "button_edit.svg");
  if(!filename.isEmpty())
    button->setIcon ( QIcon(filename) );

  QMenu *toolMenu = new QMenu(button);
  m_actionNew = toolMenu->addAction(tr("New"));
  connect(m_actionNew, SIGNAL(triggered()), this, SLOT(newTriggered()));

  m_actionEdit = toolMenu->addAction(tr("Edit"));
  m_actionEdit->setEnabled(false);
  connect(m_actionEdit, SIGNAL(triggered()), this, SLOT(editTriggered()));

  m_actionDelete = toolMenu->addAction(tr("Delete"));
  m_actionDelete->setEnabled(false);
  connect(m_actionDelete, SIGNAL(triggered()), this, SLOT(deleteTriggered()));

  QAction *actionCopy = toolMenu->addAction(tr("Copy to clipboard"));
  connect(actionCopy, SIGNAL(triggered()), this, SLOT(copyTriggered()));

  button->setMenu(toolMenu);
  button->setPopupMode(QToolButton::InstantPopup);

  toolBoxLayout->addWidget(button);
  
  // -------- the waypoint combobox -------------
  m_cBox = new QComboBox();
  connect(m_cBox, SIGNAL(activated(const QString &)), 
	  this, SLOT(selectWaypoint(const QString &)));

  m_cBox->view()->setTextElideMode (Qt::ElideRight);
  QSizePolicy sp1(QSizePolicy::Ignored, QSizePolicy::Fixed, 
		  QSizePolicy::ComboBox);
  sp1.setHorizontalStretch(2);
  m_cBox->setSizePolicy(sp1);

  m_cBox->addItem(m_cache.description());

  foreach(Waypoint wpt, m_cache.waypoints())
    if(wpt.coordinate().isValid()) 
      m_cBox->addItem(wpt.description());

  // add two special entries for current map and gps pos
  m_cBox->insertSeparator(m_cBox->count());
  m_cBox->addItem(tr("Map Center"));

  // check if gps coordinate is valid
  m_gpsCoordinate = m_mapWidget->gpsCoordinate();
  if(m_gpsCoordinate.isValid())
    m_cBox->addItem(tr("GPS Position"));

  m_customWpts = m_wptDb->get(m_cache);
  if(m_customWpts.size() > 0) {
    m_cBox->insertSeparator(m_cBox->count());

    foreach(Waypoint wpt, m_customWpts)
      m_cBox->addItem(wpt.description());
  }

  toolBoxLayout->addWidget(m_cBox);

  m_vboxLayout->addWidget(toolBoxWidget);

  InfoWidget *latInfo = new InfoWidget(tr("Latitude"), 
				       latitudeString(m_coo));
  m_vboxLayout->addWidget(latInfo);
  m_latitudeLabel = latInfo->getLabel();

  InfoWidget *lonInfo = new InfoWidget(tr("Longitude"), 
				       longitudeString(m_coo));
  m_vboxLayout->addWidget(lonInfo);
  m_longitudeLabel = lonInfo->getLabel();

  hboxLayout->addWidget(vboxWidget, 1);

  groupBox->setLayout(hboxLayout);

  return groupBox;
}

void CoordinateTool::addWidget(QWidget *widget) {
  m_vboxLayout->addWidget(widget);
}

// triggered when the user wants to create a new waypoint
void CoordinateTool::newTriggered() {
  qDebug() << __FUNCTION__;

  m_modeNew = true;

  // destroy any existing dialog
  if(m_wptDialog) {
    delete m_wptDialog;
    m_wptDialog = NULL;
  }

  // create new dialog
  m_wptDialog = createWaypointDialog();

  // and run it
  if(m_wptDialog) m_wptDialog->exec();
}

void CoordinateTool::editTriggered() {
  qDebug() << __FUNCTION__;

  m_modeNew = false;

  if(m_dialog) {
    delete m_dialog;
    m_dialog = NULL;
  }
    
  m_dialog = createCoordinateDialog();
  
  // update pickers and launch tool
  m_latManager->updateWidgets();
  m_lonManager->updateWidgets();
  m_dialog->exec();
}

void CoordinateTool::copyTriggered() {
  qDebug() << __FUNCTION__ << m_coo;

  // copy a textual represenation of the coordinate to the clipboard
  QApplication::clipboard()->setText(
     latitudeString(m_coo) + " " + longitudeString(m_coo));
}

void CoordinateTool::deleteTriggered() {
  qDebug() << __FUNCTION__;

  // get name of selected entry
  int cur = m_cBox->currentIndex();
  if(cur > 0) {
    // delete it from database ...
    m_wptDb->removeByName(m_cBox->currentText());

    // ... and delete it from combobox
    m_cBox->removeItem(cur);

    // reload custom waypoints list
    m_customWpts = m_wptDb->get(m_cache);

    // check if we've removed the last custom entry
    // and remove delimiter in that case as well

    if(m_wptDb->get(m_cache).size() == 0) 
      m_cBox->removeItem(m_cBox->count()-1);

    // select waypoint updates the coordinate, the compass
    // and the edit/new button
    selectWaypoint(m_cBox->currentText());
  }
}
