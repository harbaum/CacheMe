#ifndef GCLIVEDIALOG_H
#define GCLIVEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QDateTime>
#include <QDialogButtonBox>

class GcLiveDialog : public QDialog {
  Q_OBJECT
 public:
  GcLiveDialog(QWidget *parent = 0);
  void limits(const QDateTime &, const int);

 private:
  QLabel *m_content;
  QDialogButtonBox *m_buttonBox;
};

#endif // GCLIVEDIALOG_H
