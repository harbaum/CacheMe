#include "gclivedialog.h"

#include <QPixmap>
#include <QVBoxLayout>

#include "gclive_logo.h"

GcLiveDialog::GcLiveDialog(QWidget *parent): QDialog(parent) { 
  setWindowTitle(tr("Basic Member"));

  QVBoxLayout *mainLayout = new QVBoxLayout;

  QPixmap *pix = new QPixmap();
  pix->loadFromData(gcLiveLogo, GCLIVELOGO_LEN);

  QLabel *labelText = new QLabel(tr("Powered by Geocaching Live"));
  labelText->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(labelText);

  QLabel *labelIcon = new QLabel();
  labelIcon->setPixmap(*pix);
  labelIcon->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(labelIcon);

  m_content = new QLabel();
  m_content->setWordWrap(true);
  mainLayout->addWidget(m_content);

  m_buttonBox = new QDialogButtonBox();
  connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  mainLayout->addWidget(m_buttonBox);
  
  setLayout(mainLayout);
}

void GcLiveDialog::limits(const QDateTime &time, const int count) {
  QString msg(tr("As a basic member of Geocaching.com you can "
		 "download full details of 3 caches per 24 hour period.") + "\n\n");

  m_buttonBox->addButton(QDialogButtonBox::Cancel);

  if(count < 3) {
    msg += tr("After downloading this cache you will have ") + 
      ((2-count > 0)?QString::number(2-count):QString("no")) + 
      tr(" cache") + QString((2-count == 1)?"":"s") +
      tr(" remaining until ") + time.toString() +
      tr(" when the limit will be reset");
    m_buttonBox->addButton(QDialogButtonBox::Ok);
  } else
    msg += tr("You cannot download further caches until ") + 
      time.toString() + tr(" when the limit will be reset");

  m_content->setText(msg);
}
