#ifndef TOOL_FORMULA_H
#define TOOL_FORMULA_H

#include <QTextEdit>
#include <QLineEdit>
#include <QSpinBox>

#include "tool.h"

#define CHARS 6

class ToolFormula: public Tool {
  Q_OBJECT;

 public:
  ToolFormula(QWidget * = 0);
  ~ToolFormula();

  static QAction *registerMenu(QMenu *, QObject *, const char *);
  
 private slots:
  void parse();
  void clear();
  
 private:
  QLineEdit *m_lineEdit[CHARS];
  QLineEdit *m_textEdit;
  QLabel *lbl_result;
};

#endif // TOOL_FORMULA_H
