// a hbox with two strings, one left aligned and one right aligned
#include <QLabel>

class InfoWidget : public QWidget {
public:
  InfoWidget(const QString &leftStr = "",
	     const QString &rightStr = "",
	     QWidget *parent = 0);

  QLabel *getLabel();
  void setInfo(const QString &);

private:
  QLabel *m_left, *m_right;
};

