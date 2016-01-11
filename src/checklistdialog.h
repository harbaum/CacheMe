#ifndef CHECKLISTDIALOG_H
#define CHECKLISTDIALOG_H

#include <QDialog>
#include <QString>
#include <QCheckBox>
#include <QListWidget>

class CheckListDialog : public QDialog {
  Q_OBJECT;
  
 public:
  
  class Item {
  public:
    Item(QPixmap *, const QString &, bool);
    QString text() const;
    bool checked() const;
    QPixmap *pixmap() const;
    void setCheckBox(QCheckBox*);
    QCheckBox *checkBox() const;

  private:
    QString m_text;
    QPixmap *m_pixmap;
    bool m_checked;
    QCheckBox *m_checkBox;
  };
  
  CheckListDialog(const QString &, QWidget * = 0);
  void addItem(const Item &);
  bool isChecked(const QString &) const;

 private:
  QListWidget *m_listWidget;
  QList<Item> m_items;
};

#endif
