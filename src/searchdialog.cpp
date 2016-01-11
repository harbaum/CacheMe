#include <QDebug>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSettings>

#include "cache.h"
#include "searchdialog.h"
#include "iconloader.h"

#include "checklistdialog.h"

void SearchDialog::selectTypes(void) {
  qDebug() << __FUNCTION__;

  static const Cache::Type typeList[] = {
    Cache::TypeTraditional, 
    Cache::TypeMulti, 
    Cache::TypeMystery, 
    Cache::TypeVirtual,
    Cache::TypeWebcam, 
    Cache::TypeEvent, 
    Cache::TypeMegaEvent, 
    Cache::TypeLetterbox, 
    Cache::TypeEarthcache, 
    Cache::TypeWherigo,
    Cache::TypeCito,
    Cache::TypeUnknown 
  };

  CheckListDialog dialog(tr("Cache types"), this);
  IconLoader iconLoader(24);

  for(int ctype = 0; typeList[ctype] != Cache::TypeUnknown; ctype++)
    dialog.addItem(CheckListDialog::Item(
	       iconLoader.load(Cache::typeIconFile(typeList[ctype])),
	       Cache::typeString(typeList[ctype]), 
	       m_types.contains(typeList[ctype])));
  if(dialog.exec() == QDialog::Accepted) {
    m_types.clear();

    for(int ctype = 0; typeList[ctype] != Cache::TypeUnknown; ctype++)
      if(dialog.isChecked(Cache::typeString(typeList[ctype])))
	m_types.append(typeList[ctype]);
  }
}

void SearchDialog::selectContainers(void) {
  qDebug() << __FUNCTION__;

  static const Container::Type containerList[] = {
    Container::ContainerMicro,
    Container::ContainerSmall, 
    Container::ContainerRegular, 
    Container::ContainerLarge, 
    Container::ContainerOther,   
    Container::ContainerNotChosen, 
    Container::ContainerVirtual,
    Container::ContainerUnknown 
  };
  
  CheckListDialog dialog(tr("Cache containers"), this);
  IconLoader iconLoader(16);

  for(int ctype = 0; containerList[ctype] != Container::ContainerUnknown; 
      ctype++) 
    dialog.addItem(CheckListDialog::Item(
	 iconLoader.load(Container::iconFile(containerList[ctype], false)),
	 Container::toString(containerList[ctype]), 
	 m_containers.contains(containerList[ctype])));

  if(dialog.exec() == QDialog::Accepted) {
    m_containers.clear();

    for(int ctype = 0; containerList[ctype] != Container::ContainerUnknown; ctype++)
      if(dialog.isChecked(Container::toString(containerList[ctype])))
	m_containers.append(containerList[ctype]);
  }
}

void SearchDialog::foundChanged(int state) {
  m_excludeFound = (state == Qt::Checked);
}

void SearchDialog::ownedChanged(int state) {
  m_excludeOwned = (state == Qt::Checked);
}

void SearchDialog::saveSettings(void) {
  qDebug() << __FUNCTION__;

  QSettings settings;
  settings.beginGroup("Search");
  
  settings.setValue("Text", m_text);
  
  QStringList typeList;
  foreach(Cache::Type type, m_types)
    typeList.append(QString::number(type));
  settings.setValue("Types", typeList.join(";"));

  QStringList containerList;
  foreach(Container::Type type, m_containers)
    containerList.append(QString::number(type));
  settings.setValue("Containers", containerList.join(";"));

  settings.setValue("ExcludeOwned", m_excludeOwned);
  settings.setValue("ExcludeFound", m_excludeFound);

  settings.endGroup();
}

void SearchDialog::textChanged(const QString &text) {
  m_text = text;
}

void SearchDialog::params(QMap<QString, QVariant> &params) {
  if(!m_text.isEmpty()) params.insert("Text", m_text);

  QStringList typeList;
  foreach(Cache::Type type, m_types)
    typeList.append(QString::number(type));
  params.insert("Types", typeList.join(";"));

  if(m_containerSupported) {
    QStringList containerList;
    foreach(Container::Type type, m_containers)
      containerList.append(QString::number(type));
    params.insert("Containers", containerList.join(";"));
  }

  params.insert("ExcludeOwned", m_excludeOwned);
  params.insert("ExcludeFound", m_excludeFound);
}

SearchDialog::SearchDialog(bool containerSupported, QWidget *parent) :
  QDialog(parent), m_containerSupported(containerSupported) {

  // load defaults from config
  QSettings settings;
  settings.beginGroup("Search");

  m_text = settings.value("Text", "").toString();

  m_excludeOwned = settings.value("ExcludeOwned", false).toBool();
  m_excludeFound = settings.value("ExcludeFound", false).toBool();

  QString typesStr = settings.value("Types", "").toString();
  if(typesStr.isEmpty()) 
    for(int ctype = Cache::TypeTraditional; ctype <= Cache::TypeCito; ctype++)
      m_types.append((Cache::Type)ctype);
  else {
    QStringList typeList = typesStr.split(";");
    foreach(QString type, typeList)
      m_types.append((Cache::Type)type.toInt());
  }

  if(m_containerSupported) {
    QString containerStr = settings.value("Containers", "").toString(); 
    if(containerStr.isEmpty())
      for(int ctype = Container::ContainerRegular; ctype <= Container::ContainerVirtual; ctype++)
	m_containers.append((Container::Type)ctype);
    else {
      QStringList containerList = containerStr.split(";");
      foreach(QString container, containerList)
	m_containers.append((Container::Type)container.toInt());
    }
  }

  settings.endGroup();

  setWindowTitle(tr("Search"));

  QVBoxLayout *mainLayout = new QVBoxLayout;

  mainLayout->addWidget(new QLabel(tr("Search for") + ":"));

  QLineEdit *lineEdit = new QLineEdit(m_text);
  connect(lineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(textChanged(const QString &)));  
  mainLayout->addWidget(lineEdit);

  QPushButton *types = new QPushButton(tr("Cache types"));
  connect(types, SIGNAL(clicked()), this, SLOT(selectTypes()));
  mainLayout->addWidget(types);

  QPushButton *containers = new QPushButton(tr("Cache containers"));
  containers->setEnabled(m_containerSupported);
  connect(containers, SIGNAL(clicked()), this, SLOT(selectContainers()));
  mainLayout->addWidget(containers);

  QCheckBox *owned = new QCheckBox(tr("Exclude owned"));
  owned->setChecked(m_excludeOwned);
  connect(owned, SIGNAL(stateChanged(int)), this, SLOT(ownedChanged(int)));
  mainLayout->addWidget(owned);

  QCheckBox *found = new QCheckBox(tr("Exclude found"));
  found->setChecked(m_excludeFound);
  connect(found, SIGNAL(stateChanged(int)), this, SLOT(foundChanged(int)));
  mainLayout->addWidget(found);

  QDialogButtonBox *buttonBox = 
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(this, SIGNAL(accepted()), this, SLOT(saveSettings()));

  mainLayout->addWidget(buttonBox);
  setLayout(mainLayout);
}
