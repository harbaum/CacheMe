#include <QAction>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>

#include "tool_formula.h"
#include "EXParser.h"

#include <QDoubleValidator>


#define TOOLNAME  "Formula"

#ifdef FLICKCHARM
#include "flickcharm.h"
#endif

void ToolFormula::clear() {
  lbl_result->setText("");
}

void ToolFormula::parse() {
  bool error;
  error = false;
  
  QString str1 = m_textEdit->text().toLower();

  int i;
  for(i=0;i<CHARS;i++) {
    QString m = m_lineEdit[i]->text();
    if(m.isEmpty()) m = "0";
    str1.replace('a'+i, m);
  }

  EXParser mathParser;

  qDebug() << __FUNCTION__ << "Exp: " << str1;
  std::string r =  mathParser.evaluate(str1.toLatin1().data());
  QString result = QString::fromStdString(r);
  qDebug() << __FUNCTION__ << "Result: " << result;

  //result.replace("_", " ");

  qDebug() << result << error;
  if(!error) {
    result.prepend(tr("Result") + ": ");
    lbl_result->setText(result);
  }
  else{
      lbl_result->setText(tr("Error! Please check the letters!"));
  }

}

ToolFormula::ToolFormula(QWidget *parent):
  Tool(TOOLNAME, parent), m_textEdit(NULL){
  setWindowTitle(tr("Formula"));

  QWidget *mainWidget = new QWidget;
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainWidget->setLayout(mainLayout);

  QSettings settings;
  settings.beginGroup("Tools/" TOOLNAME);
  QString text = settings.value("expr").toString();

  QLabel *head = new QLabel(tr("Enter the values:"));
  mainLayout->addWidget(head);

  QHBoxLayout *valuesLayout = new QHBoxLayout;

  QValidator *inputRange = new QDoubleValidator(0.000, 999.999, 3, this);

  int i;
  for(i=0;i<CHARS;i++) {
    QLabel *label = new QLabel(QString('a'+i) + "=");
    valuesLayout->addWidget(label);

    m_lineEdit[i] = new QLineEdit(settings.value(QString('a'+i)).toString(), this);
    m_lineEdit[i]->setValidator(inputRange);
    connect(m_lineEdit[i], SIGNAL(textChanged(const QString&)), this, SLOT(parse()));
    valuesLayout->addWidget(m_lineEdit[i]);
  }

  mainLayout->addLayout(valuesLayout);

  QLabel *head1 = new QLabel(tr("Enter the formula:"));
  mainLayout->addWidget(head1);

  QHBoxLayout *exprLayout = new QHBoxLayout;

  m_textEdit = new QLineEdit(text, this);
  connect(m_textEdit, SIGNAL(textChanged(const QString&)), this, SLOT(parse()));

  exprLayout->addWidget(m_textEdit);

  mainLayout->addLayout(exprLayout);

  lbl_result = new QLabel(" ");
  mainLayout->addWidget(lbl_result);

  QLabel *spacer = new QLabel(tr("Enter the values and the formula in the fields.") + " " +
                              tr("Separate the blocks with an _ (underline).") + "\n" +
                              tr("I.e. instead of 47") + QString::fromUtf8("°") + 
			      " [7-a][(c+d)/2].[d^2][f-e+3][0.25*c]\n" + 
			      tr("you can write") + 
			      " 47_7-a_(c+d)/2_d^2_f-e+3_0.25*c\n");

  mainLayout->addWidget(spacer);

  setCentralWidget(mainWidget);

  settings.endGroup();

  parse();
}

ToolFormula::~ToolFormula() {
  QSettings settings;
  settings.beginGroup("Tools/" TOOLNAME);

  int i;
  for(i=0;i<CHARS;i++) 
    if(m_lineEdit[i]) settings.setValue(QString('a'+i), m_lineEdit[i]->text());

  if(m_textEdit)settings.setValue("expr", m_textEdit->text());

  settings.endGroup();
}

QAction *ToolFormula::registerMenu(QMenu *menu, QObject *target, const char *slot) {
  return Tool::registerMenu(QObject::tr("Formula solver"), menu, target, slot);
}
