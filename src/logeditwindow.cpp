#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>

#include "logeditwindow.h"
#include "iconloader.h"
#include "swapbox.h"

#ifdef FLICKCHARM
#include "flickcharm.h"
#endif

void LogEditWindow::handleReply(const Params &params) {
  QMessageBox msgBox;
  QString type = params.value("Type").toString();

  if(type.compare("LogSubmitted") == 0) {
    // re-enable submit button
    m_submitButton->setText(tr("Submit"));
    m_submitButton->setEnabled(true);

    bool status = params.value("Status").toBool();

    if(status)
      msgBox.setText(tr("Log submitted!"));
    else
      msgBox.setText(tr("Error submitting log: ") + 
		     params.value("Message").toString());

    msgBox.exec();
  } else
    qDebug() << __FUNCTION__ << "Unknown type" << type;
}

void LogEditWindow::submitLog() {
  m_submitButton->setText(tr("Submitting log ..."));
  m_submitButton->setEnabled(false);

  QMap<QString, QVariant> params;
  params.insert("Type", "CreateLog"); 
  params.insert("TypeId", m_cBox->itemData(m_cBox->currentIndex()).toInt());
  params.insert("Date", m_dateEdit->date());
  params.insert("Favorite", m_favorite->isChecked());
  params.insert("Encrypt", m_encrypt->isChecked());
  params.insert("Publish", m_publish->isChecked());
  params.insert("Text", m_textEdit->toPlainText());
  params.insert("Cache", m_cache.name());
  m_cacheProvider->requestCustom(params);
}

LogEditWindow::LogEditWindow(const Cache &cache, 
			     CacheProvider *cacheProvider, QWidget *parent) : 
  CustomWindow("LogEditWindow", parent), m_cache(cache), 
  m_cacheProvider(cacheProvider) {

  setWindowTitle(tr("Create new log"));

  QWidget *vBox = new QWidget;
  QVBoxLayout *vBoxLayout = new QVBoxLayout;
  vBoxLayout->setContentsMargins(0,0,0,0);
  vBox->setLayout(vBoxLayout);

  SwapBox *swapBox = new SwapBox();

  m_dateEdit = new QDateEdit(QDate::currentDate(), this);
  swapBox->addWidget(m_dateEdit);	

  m_cBox = new QComboBox(this);

  static const Log::Type::Id normalList[] = {
    Log::Type::Found, Log::Type::NotFound, Log::Type::WriteNote, 
    Log::Type::NeedsMaintenance, Log::Type::NeedsArchived,
    Log::Type::Unknown
  };

  static const Log::Type::Id photoList[] = {
    Log::Type::Photo, Log::Type::NotFound, Log::Type::WriteNote, 
    Log::Type::NeedsMaintenance, Log::Type::NeedsArchived,
    Log::Type::Unknown
  };

  static const Log::Type::Id eventList[] = {
    Log::Type::WriteNote, Log::Type::NeedsArchived,
    Log::Type::WillAttend, Log::Type::Attended,
    Log::Type::Unknown
  };

  IconLoader iconLoader(24);

  // different cache types can have different logs
  const Log::Type::Id *list = normalList;
  if(cache.type() == Cache::TypeWebcam)
    list = photoList;
  else if((cache.type() == Cache::TypeEvent) ||
	  (cache.type() == Cache::TypeMegaEvent))
    list = eventList;

  for(int ctype = 0; list[ctype] != Log::Type::Unknown; ctype++) {
    QPixmap *pix = iconLoader.load(Log::Type::iconFile(list[ctype]));
    m_cBox->addItem(QIcon(*pix), (Log::Type::toString(list[ctype])),
		    list[ctype]);
  }

  swapBox->addWidget(m_cBox);

  vBoxLayout->addWidget(swapBox);

  m_textEdit = new QTextEdit(this);
#ifdef FLICKCHARM
  new FlickCharm(m_textEdit, this);
#endif

  vBoxLayout->addWidget(m_textEdit);

  swapBox = new SwapBox();

  m_favorite = new QCheckBox(tr("Mark as favorite"));
  swapBox->addWidget(m_favorite);

  m_encrypt = new QCheckBox(tr("Encrypt log"));
  swapBox->addWidget(m_encrypt);

  vBoxLayout->addWidget(swapBox);

  m_publish = new QCheckBox(tr("Publish log"));
  m_publish->setChecked(true); 
  vBoxLayout->addWidget(m_publish);

  m_submitButton = new QPushButton(tr("Submit"), this);
  connect(m_submitButton, SIGNAL(clicked()), this, SLOT(submitLog()));
  vBoxLayout->addWidget(m_submitButton);

  QObject::connect(cacheProvider, SIGNAL(reply(const Params &)), 
  		   this, SLOT(handleReply(const Params &)));

  setCentralWidget(vBox);
}
