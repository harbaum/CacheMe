#ifndef TOOL_ROT13_H
#define TOOL_ROT13_H

#include <QTextEdit>
#include <QSpinBox>

#include "tool.h"

class ToolRot13: public Tool {
  Q_OBJECT;

 public:
  ToolRot13(QWidget * = 0);
  ~ToolRot13();

  static QAction *registerMenu(QMenu *, QObject *, const char *);
  
 private slots:
  void shiftDown();
  void shiftUp();
  void rot13();
  
 private:
  void shift(int);
  int val();

  QTextEdit *m_textEdit;
  QSpinBox *m_spinBox;
};

#endif // TOOL_ROT13_H
