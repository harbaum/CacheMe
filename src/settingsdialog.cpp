#include <QDebug>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QDialogButtonBox>
#include <QGeoServiceProvider>
#include <QLabel>
#include <QSettings>
#include <QTabWidget>

#include "settingsdialog.h"

#ifndef Q_WS_MAEMO_5
#define USE_TABS    // use tabs except on maemo5 (use scroll area instead)
#endif

void SettingsDialog::applyChanges() {
  qDebug() << __FUNCTION__;

  QSettings settings;
  settings.beginGroup("Map");

  if(m_typeCombo) {
    int type = m_typeCombo->itemData(m_typeCombo->currentIndex()).toInt();
    settings.setValue("Type", type);	  
    m_mapWidget->setMapType((QGraphicsGeoMap::MapType)type);
  }

  if(m_serviceCombo) 
    settings.setValue("Service", m_serviceCombo->currentText());

  if(m_hiRez) {
    bool hiRez = m_hiRez->isChecked();
    settings.setValue("HiRez", hiRez);
    m_mapWidget->setHiRez(hiRez);
  }

  if(m_disableScreensaver) {
    bool disableScreensaver = m_disableScreensaver->isChecked();
    settings.setValue("DisableScreenSaver", disableScreensaver);
    m_mapWidget->setScreenSaverDisabled(disableScreensaver);
  }

  if(m_hideOwn) {
    bool hideOwn = m_hideOwn->isChecked();
    settings.setValue("HideOwn", hideOwn);
    m_mapWidget->setHideOwn(hideOwn);
  }

  settings.endGroup();

  settings.beginGroup("Account");
  if(m_name) settings.setValue("Name", m_name->text());
#ifdef BUILTINBROWSER
  settings.setValue("BuiltinBrowser", m_builtinBrowser->isChecked());
#endif
  settings.endGroup();

}

QWidget *SettingsDialog::createMapTab() {
  /* create vbox for map tab */

#ifdef USE_TABS
  QWidget *mapTab = new QWidget(this);
#else
  QGroupBox *mapTab = new QGroupBox(tr("Map"), this);
#endif

  QGridLayout *mapTabLayout = new QGridLayout;
  mapTabLayout->setSpacing(0);
  mapTab->setLayout(mapTabLayout);

  const QString mapTypeStr[] = {
    "No Map", 
    tr("Street Map"), tr("Satellite Map Day"), 
    tr("Satellite Map Night"), tr("Terrain Map"),
    tr("Hybrid Map"), tr("Transit Map"), 
    tr("Gray Street Map"), tr("Mobile Street Map"), 
    tr("Mobile Terrain Map"), tr("Mobile Hybrid Map"), 
    tr("Mobile Transit Map"), tr("Mobile Gray Street Map") };

  /* -------------- map type combo box --------------- */
  mapTabLayout->addWidget(new QLabel(tr("Type") + ":", this), 0, 0);

  m_typeCombo = new QComboBox(this);
  QList<QGraphicsGeoMap::MapType> types = 
    this->m_mapWidget->supportedMapTypes(); 

  // add entry for all map types
  for(int i = 0; i < types.size(); i++) {
    if(types[i] <= 12 /* QGraphicsGeoMap::MobileGrayStreetMap */ ) {
      m_typeCombo->addItem(mapTypeStr[types[i]], types[i]);
    
      if(types[i] == this->m_mapWidget->mapType())
	m_typeCombo->setCurrentIndex(i);
    }
  }

  mapTabLayout->addWidget(m_typeCombo, 0, 1);

  /* -------------- geo service combo box --------------- */
  mapTabLayout->addWidget(new QLabel(tr("Service") + ":", this), 1, 0);

  m_serviceCombo = new QComboBox(this);
  QStringList list = QGeoServiceProvider::availableServiceProviders();
  for(int i=0;i<list.size();i++) {
    m_serviceCombo->addItem(list[i]);

    if(list[i] == this->m_mapWidget->managerName())
      m_serviceCombo->setCurrentIndex(i);
  }
  mapTabLayout->addWidget(m_serviceCombo, 1, 1);
 

  QLabel *label = new QLabel("<small>" + tr("A change of the map service "
		    "takes effect after restart.") + "</small>", this);
  label->setWordWrap(true);
  mapTabLayout->addWidget(label, 2, 0, 1, 2);

  /* -------------- hirez switch ------------------ */
  m_hiRez = new QCheckBox( tr("Bigger icons"), this);
  m_hiRez->setChecked(this->m_mapWidget->hiRez());
  mapTabLayout->addWidget(m_hiRez, 3, 0, 1, 2);

  /* -------------- hide owned/found switch ------------------ */
  m_hideOwn = new QCheckBox( tr("Hide owned/found"), this);
  m_hideOwn->setChecked(this->m_mapWidget->hideOwn());
  mapTabLayout->addWidget(m_hideOwn, 4, 0, 1, 2);

  /* -------------- disable screen saver switch ------------------ */
  m_disableScreensaver = new QCheckBox( tr("Disable Screensaver"), this);
  m_disableScreensaver->setChecked(this->m_mapWidget->screenSaverDisabled());
  mapTabLayout->addWidget(m_disableScreensaver, 5, 0, 1, 2);

  return mapTab;
}

QWidget *SettingsDialog::createAccountTab() {
  /* create vbox for account tab */

#ifdef USE_TABS
  QWidget *accountTab = new QWidget(this);
#else
  QGroupBox *accountTab = new QGroupBox(tr("Account"), this);
#endif

  QVBoxLayout *accountTabLayout = new QVBoxLayout;
  accountTabLayout->setSpacing(0);
  accountTab->setLayout(accountTabLayout);

  /* ------------------- user configuration --------------------- */

  QSettings settings;
  settings.beginGroup("Account");

  QLabel *accountTitle = new QLabel(tr("Geocaching.com account"));
  accountTabLayout->addWidget(accountTitle);

  accountTabLayout->addStretch(1);

  /* -------------- user name entry --------------- */

  accountTabLayout->addWidget(new QLabel(tr("Username") + ":"));
  m_name = new QLineEdit(settings.value("Name", "").toString());
  accountTabLayout->addWidget(m_name);

#ifdef BUILTINBROWSER
  m_builtinBrowser = new QCheckBox( tr("Use built-in browser"), this);
  m_builtinBrowser->setChecked(settings.value("BuiltinBrowser", false).toBool());
  accountTabLayout->addWidget(m_builtinBrowser);
#endif

  accountTabLayout->addStretch(1);

  settings.endGroup();

  return accountTab;
}

SettingsDialog::SettingsDialog(MapWidget *mapWidget, 
	       CacheProvider *cacheProvider, QWidget *parent) :
  QDialog(parent), m_mapWidget(mapWidget), m_typeCombo(NULL),
  m_serviceCombo(NULL), m_name(NULL),
  m_hiRez(NULL), m_hideOwn(NULL) {

  setWindowTitle(tr("Settings"));

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(0,0,0,0);
  mainLayout->setSpacing(0);

#ifdef USE_TABS
  /* topics are sorted into tabs */
  QTabWidget *tab = new QTabWidget(this);
  tab->addTab(createMapTab(), tr("Map"));
  tab->addTab(createAccountTab(), tr("Account"));
  mainLayout->addWidget(tab);
#else
  QScrollArea *scrollArea = new QScrollArea(this);
  scrollArea->setWidgetResizable(true);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  QWidget *scrollWidget = new QWidget;
  QVBoxLayout *scrollLayout = new QVBoxLayout;
  scrollWidget->setLayout(scrollLayout);
  scrollLayout->addWidget(createMapTab());
  scrollLayout->addWidget(createAccountTab());
  scrollArea->setWidget(scrollWidget);
  mainLayout->addWidget(scrollArea);
#endif

  /* ------------------------ plugins ------------------------ */
  cacheProvider->registerSettings(this);

  QDialogButtonBox *buttonBox = 
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(this, SIGNAL(accepted()), this, SLOT(applyChanges()));

  mainLayout->addWidget(buttonBox);
  setLayout(mainLayout);
}

#ifdef BUILTINBROWSER
bool SettingsDialog::useBuiltInBrowser() {
  QSettings settings;
  settings.beginGroup("Account");
  bool val = settings.value("BuiltinBrowser", false).toBool();
  settings.endGroup();
  
  return val;
}
#endif
