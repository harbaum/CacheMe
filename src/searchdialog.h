#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QString>
#include <QMap>
#include <QVariant>
#include "cache.h"

class SearchDialog : public QDialog {
 Q_OBJECT

 public:
  SearchDialog(bool, QWidget * = 0);
  void params(QMap<QString, QVariant> &);

 private slots:
  void selectTypes();
  void selectContainers();
  void textChanged(const QString &);
  void saveSettings();
  void foundChanged(int);
  void ownedChanged(int);

 private:
  QList<Cache::Type> m_types;
  QList<Container::Type> m_containers;
  QString m_text;
  bool m_excludeOwned, m_excludeFound;
  bool m_containerSupported;
};

#endif
