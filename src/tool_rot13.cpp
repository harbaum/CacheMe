#include <QAction>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QPushButton>

#include "tool_rot13.h"

#define TOOLNAME  "Rot13Cesar"

#ifdef FLICKCHARM
#include "flickcharm.h"
#endif

int ToolRot13::val() {
  if(m_spinBox)
    return m_spinBox->value();

  return 0;
}

void ToolRot13::shift(int offset) {
  if(m_textEdit) {
    QString r = m_textEdit->toPlainText();
    int i = r.length();
    while( i-- ) {
      if( r[i] >= QChar('A') && r[i] <= QChar('Z') ) {
	r[i] = (char)((int)QChar(r[i]).toLatin1() + offset);

	// adjust for under/overflows
	if( r[i] < QChar('A') ) r[i] = (char)((int)QChar(r[i]).toLatin1() + 26);
	if( r[i] > QChar('Z') ) r[i] = (char)((int)QChar(r[i]).toLatin1() - 26);
      }
      
      else if( r[i] >= QChar('a') && r[i] <= QChar('z') ) {
	r[i] = (char)((int)QChar(r[i]).toLatin1() + offset);
	
	// adjust for under/overflows
	if( r[i] < QChar('a') ) r[i] = (char)((int)QChar(r[i]).toLatin1() + 26);
	if( r[i] > QChar('z') ) r[i] = (char)((int)QChar(r[i]).toLatin1() - 26);
      }
    }
    
    m_textEdit->setText(r);
  }
}

void ToolRot13::shiftDown() {
  shift(-val());
}

void ToolRot13::shiftUp() {
  shift(val());
}

void ToolRot13::rot13() {
  shift(13);
}

ToolRot13::ToolRot13(QWidget *parent): 
  Tool(TOOLNAME, parent), m_textEdit(NULL), m_spinBox(NULL) {
  setWindowTitle(tr("Rot13/Cesar"));

  QWidget *mainWidget = new QWidget;
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainWidget->setLayout(mainLayout);

  QSettings settings;
  settings.beginGroup("Tools/" TOOLNAME);
  QString text = settings.value("text").toString();

  m_textEdit = new QTextEdit(text, this);
#ifdef FLICKCHARM
  new FlickCharm(m_textEdit, this);
#endif

  mainLayout->addWidget(m_textEdit);

  // create button box below
  QWidget *controlWidget = new QWidget;
  QHBoxLayout *controlLayout = new QHBoxLayout;
  controlWidget->setLayout(controlLayout);

  QPushButton *downButton = new QPushButton("-");
  connect(downButton, SIGNAL(clicked()), this, SLOT(shiftDown()));
  controlLayout->addWidget(downButton);

  m_spinBox = new QSpinBox();
  m_spinBox->setRange(1, 25);
  m_spinBox->setValue(settings.value("step").toInt());
  controlLayout->addWidget(m_spinBox);

  QPushButton *upButton = new QPushButton("+");
  connect(upButton, SIGNAL(clicked()), this, SLOT(shiftUp()));
  controlLayout->addWidget(upButton);

  QPushButton *rot13Button = new QPushButton(tr("Rot13"));
  connect(rot13Button, SIGNAL(clicked()), this, SLOT(rot13()));
  controlLayout->addWidget(rot13Button);

  mainLayout->addWidget(controlWidget);
  
  setCentralWidget(mainWidget);

  settings.endGroup();
}

ToolRot13::~ToolRot13() {
  QSettings settings;
  settings.beginGroup("Tools/" TOOLNAME);

  if(m_textEdit) 
    settings.setValue("text", m_textEdit->toPlainText());

  if(m_spinBox)
    settings.setValue("step", val());

  settings.endGroup();
}

QAction *ToolRot13::registerMenu(QMenu *menu, QObject *target, const char *slot) {
  return Tool::registerMenu(QObject::tr("&Rot13/Cesar"), menu, target, slot);
}
