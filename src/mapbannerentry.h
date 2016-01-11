#ifndef MAP_BANNER_ENTRY_H
#define MAP_BANNER_ENTRY_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <QList>
#include <QPainter>
#include <QRect>

class MapWidget;

class MapBannerEntry : public QObject {
  Q_OBJECT;
 public:
  enum State { Waiting, Appearing, Visible, Disappearing, Done };
  MapBannerEntry(MapWidget *, const QString &);
  ~MapBannerEntry();
  void paint(QPainter *, bool);
  bool isDisappearing() const;

 private slots:
  void timeout();

 signals:
  void done(MapBannerEntry *);

 private:
  QRect area();

  QTimer *m_timer;
  QString m_message;
  State m_state;
  int m_counter;
  MapWidget *m_mapWidget;
  bool m_important;

  static const int STEPS = 10;
  static const int APPEAR = 500;   // (dis-)appears in these msec
  static const int STAY = 3000;    // stays these msec
};

class MapBannerEntryList : public QList<MapBannerEntry*> {
 public:
  MapBannerEntryList();
  ~MapBannerEntryList();
  QList<MapBannerEntry*>::const_iterator begin() const;
  QList<MapBannerEntry*>::const_iterator end() const;
};

#endif // MAP_BANNER_ENTRY_H
