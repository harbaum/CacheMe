#ifndef TOOL_ROMAN_H
#define TOOL_ROMAN_H

#include <QTextEdit>
#include <QLineEdit>
#include <QSpinBox>

#include "tool.h"

#define MAX_CHARS 50

class ToolRoman: public Tool {
  Q_OBJECT;

 public:
  ToolRoman(QWidget * = 0);
  ~ToolRoman();

  static QAction *registerMenu(QMenu *, QObject *, const char *);
  
 private slots:
  void roman();
  void roman(QString);
  
 private:
  bool convert(char *);

  QLineEdit *m_textEdit;
  QLabel *lbl_result;


  int  angka[MAX_CHARS];
};

#endif // TOOL_ROMAN_H
