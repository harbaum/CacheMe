#include <QAction>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>

#include "tool_roman.h"

#define TOOLNAME  "Roman"

#ifdef FLICKCHARM
#include "flickcharm.h"
#endif

bool ToolRoman::convert(char input[]){
  bool ok = true;
  int  i;

  for(i=0;input[i];i++) {
    if(input[i] == 'I')      angka[i] = 1;
    else if(input[i] == 'V') angka[i] = 5;
    else if(input[i] == 'X') angka[i] = 10;
    else if(input[i] == 'L') angka[i] = 50;
    else if(input[i] == 'C') angka[i] = 100;
    else if(input[i] == 'D') angka[i] = 500;
    else if(input[i] == 'M') angka[i] = 1000;
    else{
      qDebug() << "Unknown value, dropping it:" << input[i];
      ok = false;
    }
  }
  return ok;
}

void ToolRoman::roman() {
  roman(m_textEdit->text());
}

void ToolRoman::roman(QString str1) {
  qDebug() << __FUNCTION__;
  int n = 0,total=0,x=0,totals = 0;
  int  i=0;

  str1.replace(" ", "");
  str1 = str1.toUpper();
  QByteArray ba = str1.toLatin1();
  char *text = ba.data();

  for(i=0;i<MAX_CHARS;i++)
    angka[i] = 0;
  
  bool ok = convert(text);

  // sum up all values
  while(angka[n]){
    if(angka[n] >= angka[n+1]){
      total=total+angka[n];
    }
    else{
      if(angka[n]<angka[n+1]){
	x=angka[n+1]-angka[n];
	n++;
      }
    }
    totals=total+x;
    n++;
  }

  qDebug() << totals << ok;

  if(ok) {
    QString r = QString::number(totals) ;
    r.prepend(tr("Result") + ": ");
    lbl_result->setText(r);
  }
  else{
    lbl_result->setText(tr("Error! Please check the letters!"));
  }
}

ToolRoman::ToolRoman(QWidget *parent):
  Tool(TOOLNAME, parent), m_textEdit(NULL){
  setWindowTitle(tr("Roman"));

  QWidget *mainWidget = new QWidget;
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainWidget->setLayout(mainLayout);

  QSettings settings;
  settings.beginGroup("Tools/" TOOLNAME);
  QString text = settings.value("text").toString();


  QLabel *head = new QLabel(tr("Enter the roman numbers (I.e.: MXXVIV or xiv):"));
  mainLayout->addWidget(head);

  QHBoxLayout *exprLayout = new QHBoxLayout;

  m_textEdit = new QLineEdit(text, this);
  m_textEdit->setMaxLength(MAX_CHARS);
  connect(m_textEdit, SIGNAL(textChanged(const QString&)), this, SLOT(roman(const QString&)));

  exprLayout->addWidget(m_textEdit);

  mainLayout->addLayout(exprLayout);

  lbl_result = new QLabel();
  mainLayout->addWidget(lbl_result);


  QLabel *spacer = new QLabel("M=1000, D=500, C=100, L=50, X=10, V=5, I=1");
  spacer->setMinimumHeight(220);
  spacer->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter );
  mainLayout->addWidget(spacer);
  
  setCentralWidget(mainWidget);

  settings.endGroup();

  roman();
}

ToolRoman::~ToolRoman() {
  QSettings settings;
  settings.beginGroup("Tools/" TOOLNAME);

  if(m_textEdit) 
    settings.setValue("text", m_textEdit->text());

  settings.endGroup();
}

QAction *ToolRoman::registerMenu(QMenu *menu, QObject *target, const char *slot) {
  return Tool::registerMenu(QObject::tr("Roman numbers"), menu, target, slot);
}
