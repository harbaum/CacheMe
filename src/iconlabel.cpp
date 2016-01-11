#include <QDebug>

#include <QLabel>
#include <QFont>
#include <QHBoxLayout>

#include "iconlabel.h"
#include "iconloader.h"

void IconLabel::resizeEvent(QResizeEvent *) {
  if(m_elided) {
    QFontMetrics fm = QFontMetrics(m_label->font());
    int maxWidth = m_label->width();
    QString elidedString = fm.elidedText(m_text, Qt::ElideRight, maxWidth);
    m_label->setText(elidedString);
  }
}

int IconLabel::prepare(const QString &text, qreal scale, bool elided) {
  m_text = text;
  m_elided = elided;
  
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setContentsMargins(0,0,0,0);
  setLayout(layout);

  m_label = new QLabel(text, this);
  QFont font = m_label->font();

  font.setPointSize(font.pointSize()*scale);
  m_label->setFont(font);
  if(!elided) 
    m_label->setWordWrap(true);
  else {
    QSizePolicy sp1(QSizePolicy::Ignored, QSizePolicy::Fixed);
    m_label->setSizePolicy(sp1);
  }

  QFontMetrics(m_label->font()).height();

  layout->addWidget(m_label, 1);
  return QFontMetrics(font).height();
}

IconLabel::IconLabel(IconLoader *loader, const QStringList &icon,
     const QString &text, qreal scale, QWidget *parent) : QWidget(parent) {
  int iconSize = prepare(text, scale, false);

  QHBoxLayout *hbox = static_cast<QHBoxLayout *>(layout());
  hbox->insertWidget(0, loader->newWidget(icon, iconSize));		      
}

IconLabel::IconLabel(IconLoader *loader, const QString &icon,
     const QString &text, qreal scale, QWidget *parent) : QWidget(parent) {

  int iconSize = prepare(text, scale, false);

  QHBoxLayout *hbox = static_cast<QHBoxLayout *>(layout());
  hbox->insertWidget(0, loader->newWidget(icon, iconSize));		      
}

IconLabel::IconLabel(IconLoader *loader, const Cache &cache,
     const QString &text, qreal scale, bool elide, QWidget *parent) : QWidget(parent) {

  int iconSize = prepare(text, scale, elide);

  QHBoxLayout *hbox = static_cast<QHBoxLayout *>(layout());
  hbox->insertWidget(0, loader->newWidget(cache, iconSize));		      
}
