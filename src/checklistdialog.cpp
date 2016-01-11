#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QLabel>

#include "checklistdialog.h"
#include "flickcharm.h"

CheckListDialog::Item::Item(QPixmap *pixmap, 
			    const QString &text, bool checked):
  m_text(text), m_pixmap(pixmap), m_checked(checked), m_checkBox(NULL) {}

QString CheckListDialog::Item::text() const { return m_text; }
QPixmap *CheckListDialog::Item::pixmap() const { return m_pixmap; }
bool CheckListDialog::Item::checked() const { return m_checked; }
void CheckListDialog::Item::setCheckBox(QCheckBox *checkBox) { m_checkBox = checkBox; }
QCheckBox *CheckListDialog::Item::checkBox() const { return m_checkBox; }

CheckListDialog::CheckListDialog(const QString &title, QWidget *parent) :
  QDialog(parent) {

  setWindowTitle(title);

  QVBoxLayout *mainLayout = new QVBoxLayout;

  // one big listwidget
  m_listWidget = new QListWidget();
  m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);

#ifdef FLICKCHARM
  m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  new FlickCharm(m_listWidget, this);
#endif

  mainLayout->addWidget(m_listWidget);  

  QDialogButtonBox *buttonBox = 
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  mainLayout->addWidget(buttonBox);
  setLayout(mainLayout);
}

bool CheckListDialog::isChecked(const QString &text) const {
  for(int i=0;i < m_items.size();i++)
    if(m_items[i].text().compare(text) == 0)
      return m_items[i].checkBox()->isChecked();

  return false;
}

void CheckListDialog::addItem(const Item &item) {
  Item citem(item);

  // build a custom widget
  QWidget *w = new QWidget();
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setContentsMargins(5,0,5,0);
  w->setLayout(layout);

  QCheckBox *cbox = new QCheckBox(item.text());
  cbox->setCheckState(item.checked()?Qt::Checked:Qt::Unchecked);
  citem.setCheckBox(cbox);
  layout->addWidget(cbox, 1);
  QLabel *label = new QLabel(this);
  label->setPixmap(*item.pixmap());
  layout->addWidget(label, 0);
  
  QListWidgetItem *witem = 
    new QListWidgetItem(m_listWidget, QListWidgetItem::UserType);
  m_listWidget->addItem(witem);
  witem->setSizeHint(w->sizeHint());
  m_listWidget->setItemWidget(witem, w);

  m_items.append(citem);
}
