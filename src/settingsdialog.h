#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>

#include "cacheprovider.h"
#include "mapwidget.h"

class SettingsDialog : public QDialog {
 Q_OBJECT

 public:
  SettingsDialog(MapWidget *, CacheProvider *, QWidget * = 0);
#ifdef BUILTINBROWSER
  static bool useBuiltInBrowser();
#endif

 private slots:
  void applyChanges();

 private:
  QWidget *createMapTab();
  QWidget *createAccountTab();

  MapWidget *m_mapWidget;
  QComboBox *m_typeCombo;
  QComboBox *m_serviceCombo;
  QLineEdit *m_name;
  QCheckBox *m_hiRez;
  QCheckBox *m_disableScreensaver;
  QCheckBox *m_hideOwn;
#ifdef BUILTINBROWSER
  QCheckBox *m_builtinBrowser;
#endif
};

#endif
